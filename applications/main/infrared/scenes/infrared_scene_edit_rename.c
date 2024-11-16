#include <string.h>
#include <toolbox/path.h>
#include "../infrared_signal.h"
#include "../infrared_remote.h"
#include "../infrared_app_i.h"
#include "../infrared_metadata.h"

static int32_t infrared_scene_edit_rename_task_callback(void* context) {
    InfraredApp* infrared = context;
    InfraredAppState* app_state = &infrared->app_state;
    const InfraredEditTarget edit_target = app_state->edit_target;

    InfraredErrorCode error = InfraredErrorCodeNone;
    if(edit_target == InfraredEditTargetButton) {
        furi_assert(app_state->current_button_index != InfraredButtonIndexNone);
        error = infrared_remote_rename_signal(
            infrared->remote, app_state->current_button_index, infrared->text_store[0]);
    } else if(edit_target == InfraredEditTargetRemote) {
        error = infrared_rename_current_remote(infrared, infrared->text_store[0]);
    } else {
        furi_crash();
    }

    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);

    return error;
}

void infrared_scene_edit_rename_on_enter(void* context) {
    InfraredApp* infrared = context;
    TextInput* text_input = infrared->text_input;
    FuriString* temp_str = furi_string_alloc();
    size_t enter_name_length = 0;

    switch(infrared->app_state.edit_target) {
    case InfraredEditTargetButton:
        text_input_set_header_text(text_input, "Name the button");
        const int32_t current_button_index = infrared->app_state.current_button_index;
        furi_check(current_button_index != InfraredButtonIndexNone);
        enter_name_length = INFRARED_MAX_BUTTON_NAME_LENGTH;
        strlcpy(
            infrared->text_store[0],
            infrared_remote_get_signal_name(infrared->remote, current_button_index),
            enter_name_length);
        break;
    case InfraredEditTargetRemote:
        text_input_set_header_text(text_input, "Name the remote");
        enter_name_length = INFRARED_MAX_REMOTE_NAME_LENGTH;
        strlcpy(
            infrared->text_store[0],
            infrared_remote_get_name(infrared->remote),
            enter_name_length);
        break;
    case InfraredEditTargetMetadataBrand:
        text_input_set_header_text(text_input, "Enter Brand Name");
        furi_string_set(
            temp_str, infrared_metadata_get_brand(infrared_remote_get_metadata(infrared->remote)));
        strlcpy(infrared->text_store[0], furi_string_get_cstr(temp_str), INFRARED_TEXT_STORE_SIZE);
        break;
    case InfraredEditTargetMetadataModel:
        text_input_set_header_text(text_input, "Enter Device Model");
        furi_string_set(
            temp_str, infrared_metadata_get_model(infrared_remote_get_metadata(infrared->remote)));
        strlcpy(infrared->text_store[0], furi_string_get_cstr(temp_str), INFRARED_TEXT_STORE_SIZE);
        break;
    case InfraredEditTargetMetadataContributor:
        text_input_set_header_text(text_input, "Enter Contributor Name");
        furi_string_set(
            temp_str,
            infrared_metadata_get_contributor(infrared_remote_get_metadata(infrared->remote)));
        strlcpy(infrared->text_store[0], furi_string_get_cstr(temp_str), INFRARED_TEXT_STORE_SIZE);
        break;
    case InfraredEditTargetMetadataRemoteModel:
        text_input_set_header_text(text_input, "Enter Remote Model");
        furi_string_set(
            temp_str,
            infrared_metadata_get_remote_model(infrared_remote_get_metadata(infrared->remote)));
        strlcpy(infrared->text_store[0], furi_string_get_cstr(temp_str), INFRARED_TEXT_STORE_SIZE);
        break;
    case InfraredEditTargetSignal:
        text_input_set_header_text(text_input, "Name the Button");
        furi_check(infrared->app_state.current_button_index != InfraredButtonIndexNone);
        enter_name_length = INFRARED_MAX_BUTTON_NAME_LENGTH;
        strlcpy(
            infrared->text_store[0],
            infrared_remote_get_signal_name(
                infrared->remote, infrared->app_state.current_button_index),
            enter_name_length);
        break;
    default:
        text_input_set_header_text(text_input, "Edit Name");
        break;
    }

    // Add file validation for remote renaming
    if(infrared->app_state.edit_target == InfraredEditTargetRemote) {
        FuriString* folder_path = furi_string_alloc();
        if(furi_string_end_with(infrared->file_path, INFRARED_APP_EXTENSION)) {
            path_extract_dirname(furi_string_get_cstr(infrared->file_path), folder_path);
        }
        ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
            furi_string_get_cstr(folder_path),
            INFRARED_APP_EXTENSION,
            infrared_remote_get_name(infrared->remote));
        text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);
        furi_string_free(folder_path);
    }

    text_input_set_result_callback(
        text_input,
        infrared_text_input_callback,
        infrared,
        infrared->text_store[0],
        enter_name_length ? enter_name_length : INFRARED_TEXT_STORE_SIZE,
        false);

    furi_string_free(temp_str);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewTextInput);
}

bool infrared_scene_edit_rename_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTextEditDone) {
            if(infrared->app_state.edit_target == InfraredEditTargetButton ||
               infrared->app_state.edit_target == InfraredEditTargetRemote) {
                // Use original task-based rename for buttons and remotes
                infrared_blocking_task_start(infrared, infrared_scene_edit_rename_task_callback);
            } else {
                // Handle metadata updates directly
                InfraredRemote* remote = infrared->remote;
                InfraredMetadata* metadata = infrared_remote_get_metadata(remote);

                switch(infrared->app_state.edit_target) {
                case InfraredEditTargetMetadataBrand:
                    infrared_metadata_set_brand(metadata, infrared->text_store[0]);
                    break;
                case InfraredEditTargetMetadataModel:
                    infrared_metadata_set_model(metadata, infrared->text_store[0]);
                    break;
                case InfraredEditTargetMetadataContributor:
                    infrared_metadata_set_contributor(metadata, infrared->text_store[0]);
                    break;
                case InfraredEditTargetMetadataRemoteModel:
                    infrared_metadata_set_remote_model(metadata, infrared->text_store[0]);
                    break;
                case InfraredEditTargetSignal:
                    infrared_remote_rename_signal(
                        infrared->remote,
                        infrared->app_state.current_button_index,
                        infrared->text_store[0]);
                    break;
                case InfraredEditTargetNone:
                case InfraredEditTargetRemote:
                case InfraredEditTargetButton:
                case InfraredEditTargetMetadataDeviceType:
                default:
                    break;
                }
                infrared_remote_save(remote);
                scene_manager_next_scene(infrared->scene_manager, InfraredSceneEditRenameDone);
            }
            consumed = true;
        } else if(event.event == InfraredCustomEventTypeTaskFinished) {
            const InfraredErrorCode task_error = infrared_blocking_task_finalize(infrared);

            if(!INFRARED_ERROR_PRESENT(task_error)) {
                scene_manager_next_scene(infrared->scene_manager, InfraredSceneEditRenameDone);
            } else {
                bool long_signal = INFRARED_ERROR_CHECK(
                    task_error, InfraredErrorCodeSignalRawUnableToReadTooLongData);

                const char* format = "Failed to rename\n%s";
                const char* target = infrared->app_state.edit_target == InfraredEditTargetButton ?
                                         "button" :
                                         "file";
                if(long_signal) {
                    format = "Failed to rename\n\"%s\" is too long.\nTry to edit file from pc";
                    target = infrared_remote_get_signal_name(
                        infrared->remote, INFRARED_ERROR_GET_INDEX(task_error));
                }

                infrared_show_error_message(infrared, format, target);

                const uint32_t possible_scenes[] = {InfraredSceneRemoteList, InfraredSceneRemote};
                scene_manager_search_and_switch_to_previous_scene_one_of(
                    infrared->scene_manager, possible_scenes, COUNT_OF(possible_scenes));
            }

            infrared->app_state.current_button_index = InfraredButtonIndexNone;
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_edit_rename_on_exit(void* context) {
    InfraredApp* infrared = context;
    TextInput* text_input = infrared->text_input;

    ValidatorIsFile* validator_context = text_input_get_validator_callback_context(text_input);
    if(validator_context) {
        validator_is_file_free(validator_context);
    }

    text_input_reset(text_input);
}

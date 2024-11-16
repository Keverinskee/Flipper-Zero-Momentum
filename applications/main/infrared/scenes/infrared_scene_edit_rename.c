#include <string.h>
#include <toolbox/path.h>
#include "../infrared_signal.h"
#include "../infrared_remote.h"
#include "../infrared_app_i.h"
#include "../infrared_metadata.h"

void infrared_scene_edit_rename_on_enter(void* context) {
    InfraredApp* infrared = context;
    TextInput* text_input = infrared->text_input;
    FuriString* temp_str = furi_string_alloc();
    InfraredMetadata* metadata = infrared_remote_get_metadata(infrared->remote);

    // Set appropriate header text based on edit target
    switch(infrared->app_state.edit_target) {
    case InfraredEditTargetMetadataBrand:
        text_input_set_header_text(text_input, "Enter Brand Name");
        furi_string_set(temp_str, infrared_metadata_get_brand(metadata));
        break;
    case InfraredEditTargetMetadataModel:
        text_input_set_header_text(text_input, "Enter Device Model");
        furi_string_set(temp_str, infrared_metadata_get_model(metadata));
        break;
    case InfraredEditTargetMetadataContributor:
        text_input_set_header_text(text_input, "Enter Contributor Name");
        furi_string_set(temp_str, infrared_metadata_get_contributor(metadata));
        break;
    case InfraredEditTargetMetadataRemoteModel:
        text_input_set_header_text(text_input, "Enter Remote Model");
        furi_string_set(temp_str, infrared_metadata_get_remote_model(metadata));
        break;
    case InfraredEditTargetSignal:
        text_input_set_header_text(text_input, "Name the Button");
        furi_string_set(
            temp_str,
            infrared_remote_get_signal_name(
                infrared->remote, infrared->app_state.current_button_index));
        break;
    default:
        text_input_set_header_text(text_input, "Edit Name");
        break;
    }

    // Copy string to text store
    strncpy(infrared->text_store[0], furi_string_get_cstr(temp_str), INFRARED_TEXT_STORE_SIZE);

    text_input_set_result_callback(
        text_input,
        infrared_text_input_callback,
        infrared,
        infrared->text_store[0],
        INFRARED_MAX_BUTTON_NAME_LENGTH,
        true);

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewTextInput);
}

bool infrared_scene_edit_rename_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;
    InfraredMetadata* metadata = infrared_remote_get_metadata(infrared->remote);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTextEditDone) {
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
            default:
                break;
            }
            scene_manager_previous_scene(infrared->scene_manager);
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

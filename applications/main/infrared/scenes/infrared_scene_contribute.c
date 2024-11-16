#include "../infrared_app_i.h"

static void infrared_scene_contribute_popup_callback(void* context) {
    InfraredApp* infrared = context;
    if(infrared->app_state.is_contributing_remote) {
        view_dispatcher_send_custom_event(infrared->view_dispatcher, InfraredCustomEventTypePopupClosed);
    }
}

void infrared_scene_contribute_on_enter(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;

    popup_reset(popup);
    
    infrared->app_state.is_contributing_remote = true;

    popup_set_header(popup, "Contribute IR", 64, 3, AlignCenter, AlignTop);
    popup_set_text(popup, "This will prepare your\nremote for sharing by\nadding required metadata", 64, 25, AlignCenter, AlignTop);
    
    popup_set_callback(popup, infrared_scene_contribute_popup_callback);
    popup_set_context(popup, infrared);
    popup_set_timeout(popup, 2000);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_contribute_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypePopupClosed) {
            if(infrared->app_state.is_contributing_remote) {
                infrared->app_state.edit_target = InfraredEditTargetMetadataBrand;
                infrared->app_state.edit_mode = InfraredEditModeRename;
                scene_manager_next_scene(infrared->scene_manager, InfraredSceneEditRename);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        popup_disable_timeout(infrared->popup);
        popup_set_callback(infrared->popup, NULL);
        infrared->app_state.is_contributing_remote = false;
        popup_reset(infrared->popup);
        scene_manager_previous_scene(infrared->scene_manager);
        consumed = true;
    }
    
    return consumed;
}

void infrared_scene_contribute_on_exit(void* context) {
    InfraredApp* infrared = context;
    popup_set_callback(infrared->popup, NULL);
    popup_reset(infrared->popup);
    
    if(!infrared->app_state.is_contributing_remote) {
        infrared->app_state.edit_target = InfraredEditTargetNone;
        infrared->app_state.edit_mode = InfraredEditModeNone;
    }
}

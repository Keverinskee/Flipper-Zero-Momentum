#include "../infrared_app_i.h"

typedef struct {
    const char* name;
    const char* value;
} InfraredDeviceType;

static const InfraredDeviceType device_types[] = {
    {"TV", "TV"},
    {"DVD Player", "DVD_Players"},
    {"Blu-Ray Player", "Blu-Ray"},
    {"Set-top Box", "Cable_Boxes"},
    {"Streaming Device", "Streaming_Devices"},
    {"Audio System", "Audio_and_Video_Receivers"},
    {"Speakers", "Speakers"},
    {"Sound Bar", "SoundBars"},
    {"CCTV", "CCTV"},
    {"Gaming Console", "Consoles"},
    {"Projector", "Projectors"},
    {"Universal Remote", "Universal_TV_Remotes"},
    {"Air Purifier", "Air_Purifiers"},
    {"Air Conditioner", "ACs"},
    {"Fan", "Fans"},
    {"Heater", "Heaters"},
    {"Humidifier", "Humidifiers"},
    {"Vacuum Cleaner", "Vacuum_Cleaners"},
    {"Car Multimedia", "Car_Multimedia"},
    {"Fireplace", "Fireplaces"},
    {"Digital Sign", "Digital_Signs"},
    {"Toy", "Toys"},
    {"Video Conferencing", "Videoconferencing"},
    {"Monitor", "Monitors"},
    {"Picture Frame", "Picture_Frames"},
    {"Clock", "Clocks"},
    {"Computer", "Computers"},
    {"KVM Switch", "KVM"},
    {"DVB-T", "DVB-T"},
    {"Laserdisc", "Laserdisc"},
    {"CD Player", "CD_Players"},
    {"MiniDisc Player", "MiniDisc"},
    {"Miscellaneous", "Miscellaneous"},
    {"Touchscreen Display", "Touchscreen_Displays"},
    {"Dust Collector", "Dust_Collectors"},
    {"Window Cleaner", "Window_cleaners"},
    {"LED Lighting", "LED_Lighting"},
    {"Converter", "Converters"},
    {"Whiteboard", "Whiteboards"},
    {"Other", "Other"},
};

static void
    infrared_scene_edit_select_device_type_submenu_callback(void* context, uint32_t index) {
    InfraredApp* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, index);
}

void infrared_scene_edit_select_device_type_on_enter(void* context) {
    InfraredApp* infrared = context;
    Submenu* submenu = infrared->submenu;
    submenu_reset(submenu);

    for(size_t i = 0; i < COUNT_OF(device_types); i++) {
        submenu_add_item(
            submenu,
            device_types[i].name,
            i,
            infrared_scene_edit_select_device_type_submenu_callback,
            context);
    }

    submenu_set_header(submenu, "Select Device Type");
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewSubmenu);
}

bool infrared_scene_edit_select_device_type_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        InfraredRemote* remote = infrared->remote;
        InfraredMetadata* metadata = infrared_remote_get_metadata(remote);

        // Set the selected device type
        infrared_metadata_set_device_type(metadata, device_types[event.event].value);
        infrared_remote_save(remote);

        // Show the saved popup
        scene_manager_next_scene(infrared->scene_manager, InfraredSceneEditRenameDone);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(infrared->scene_manager);
        consumed = true;
    }

    return consumed;
}

void infrared_scene_edit_select_device_type_on_exit(void* context) {
    InfraredApp* infrared = context;
    submenu_reset(infrared->submenu);
}

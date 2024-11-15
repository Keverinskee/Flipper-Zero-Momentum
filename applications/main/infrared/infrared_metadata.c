#include "infrared_remote.h"
#include "infrared_metadata.h"
#include <toolbox/path.h>
#include <furi_hal_resources.h>

// Metadata keys
#define INFRARED_METADATA_BRAND_KEY       "brand"
#define INFRARED_METADATA_DEVICE_TYPE_KEY "device_type"
#define INFRARED_METADATA_MODEL_KEY       "model"

struct InfraredMetadata {
    FuriString* brand;
    FuriString* device_type;
    FuriString* model;
};

InfraredMetadata* infrared_metadata_alloc() {
    InfraredMetadata* metadata = malloc(sizeof(InfraredMetadata));
    metadata->brand = furi_string_alloc();
    metadata->device_type = furi_string_alloc();
    metadata->model = furi_string_alloc();
    return metadata;
}

void infrared_metadata_free(InfraredMetadata* metadata) {
    furi_assert(metadata);
    furi_string_free(metadata->brand);
    furi_string_free(metadata->device_type);
    furi_string_free(metadata->model);
    free(metadata);
}

void infrared_metadata_reset(InfraredMetadata* metadata) {
    furi_assert(metadata);
    furi_string_reset(metadata->brand);
    furi_string_reset(metadata->device_type);
    furi_string_reset(metadata->model);
}

// Modify infrared_metadata_save to be simpler since it's now used within batch operations
InfraredErrorCode infrared_metadata_save(InfraredMetadata* metadata, FlipperFormat* ff) {
    InfraredErrorCode error = InfraredErrorCodeNone;

    // Write blank line
    if(!flipper_format_write_comment_cstr(ff, "")) {
        return InfraredErrorCodeFileOperationFailed;
    }

    // Write brand if exists
    if(furi_string_size(metadata->brand) > 0) {
        if(!flipper_format_write_string_cstr(
               ff, "# Brand", furi_string_get_cstr(metadata->brand))) {
            return InfraredErrorCodeFileOperationFailed;
        }
    }

    // Write device type if exists
    if(furi_string_size(metadata->device_type) > 0) {
        if(!flipper_format_write_string_cstr(
               ff, "# Device Type", furi_string_get_cstr(metadata->device_type))) {
            return InfraredErrorCodeFileOperationFailed;
        }
    }

    // Write model if exists
    if(furi_string_size(metadata->model) > 0) {
        if(!flipper_format_write_string_cstr(
               ff, "# Model", furi_string_get_cstr(metadata->model))) {
            return InfraredErrorCodeFileOperationFailed;
        }
    }

    return error;
}

InfraredErrorCode infrared_metadata_read(InfraredMetadata* metadata, FlipperFormat* ff) {
    infrared_metadata_reset(metadata);
    InfraredErrorCode error = InfraredErrorCodeNone;

    // Read each metadata field - using verified functions only
    flipper_format_read_string(ff, "#Brand", metadata->brand);
    flipper_format_read_string(ff, "#Device Type", metadata->device_type);
    flipper_format_read_string(ff, "#Model", metadata->model);

    // Process the ": " prefix if present for each field
    if(furi_string_size(metadata->brand) > 2 &&
       strncmp(furi_string_get_cstr(metadata->brand), ": ", 2) == 0) {
        furi_string_right(metadata->brand, 2);
    }

    if(furi_string_size(metadata->device_type) > 2 &&
       strncmp(furi_string_get_cstr(metadata->device_type), ": ", 2) == 0) {
        furi_string_right(metadata->device_type, 2);
    }

    if(furi_string_size(metadata->model) > 2 &&
       strncmp(furi_string_get_cstr(metadata->model), ": ", 2) == 0) {
        furi_string_right(metadata->model, 2);
    }

    return error;
}
const char* infrared_metadata_get_brand(const InfraredMetadata* metadata) {
    return furi_string_get_cstr(metadata->brand);
}

const char* infrared_metadata_get_device_type(const InfraredMetadata* metadata) {
    return furi_string_get_cstr(metadata->device_type);
}

const char* infrared_metadata_get_model(const InfraredMetadata* metadata) {
    return furi_string_get_cstr(metadata->model);
}

void infrared_metadata_set_brand(InfraredMetadata* metadata, const char* brand) {
    furi_string_set(metadata->brand, brand);
}

void infrared_metadata_set_device_type(InfraredMetadata* metadata, const char* device_type) {
    furi_string_set(metadata->device_type, device_type);
}

void infrared_metadata_set_model(InfraredMetadata* metadata, const char* model) {
    furi_string_set(metadata->model, model);
}

#include "infrared_remote.h"
#include "infrared_metadata.h"
#include <toolbox/path.h>
#include <furi_hal_resources.h>
#include <toolbox/stream/stream.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#define TAG                               "InfraredMetadata"
// Metadata keys
#define INFRARED_METADATA_BRAND_KEY       "Brand"
#define INFRARED_METADATA_DEVICE_TYPE_KEY "Device Type"
#define INFRARED_METADATA_MODEL_KEY       "Model"

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

InfraredErrorCode infrared_metadata_save(InfraredMetadata* metadata, FlipperFormat* ff) {
    InfraredErrorCode error = InfraredErrorCodeNone;

    FURI_LOG_D(
        TAG,
        "Saving metadata - Brand: '%s', Type: '%s', Model: '%s'",
        furi_string_get_cstr(metadata->brand),
        furi_string_get_cstr(metadata->device_type),
        furi_string_get_cstr(metadata->model));

    // Write blank line
    if(!flipper_format_write_comment_cstr(ff, "")) {
        FURI_LOG_E(TAG, "Failed to write blank line");
        return InfraredErrorCodeFileOperationFailed;
    }

    // Write brand if exists
    if(furi_string_size(metadata->brand) > 0) {
        char comment[256];
        snprintf(comment, sizeof(comment), "Brand: %s", furi_string_get_cstr(metadata->brand));
        bool success = flipper_format_write_comment_cstr(ff, comment);
        FURI_LOG_D(TAG, "Writing brand comment: '%s', result: %d", comment, success);
        if(!success) return InfraredErrorCodeFileOperationFailed;
    }

    // Write device type if exists
    if(furi_string_size(metadata->device_type) > 0) {
        char comment[256];
        snprintf(comment, sizeof(comment), "Device Type: %s", furi_string_get_cstr(metadata->device_type));
        bool success = flipper_format_write_comment_cstr(ff, comment);
        FURI_LOG_D(TAG, "Writing device type comment: '%s', result: %d", comment, success);
        if(!success) return InfraredErrorCodeFileOperationFailed;
    }

    // Write model if exists
    if(furi_string_size(metadata->model) > 0) {
        char comment[256];
        snprintf(comment, sizeof(comment), "Model: %s", furi_string_get_cstr(metadata->model));
        bool success = flipper_format_write_comment_cstr(ff, comment);
        FURI_LOG_D(TAG, "Writing model comment: '%s', result: %d", comment, success);
        if(!success) return InfraredErrorCodeFileOperationFailed;
    }

    return error;
}

InfraredErrorCode infrared_metadata_read(InfraredMetadata* metadata, FlipperFormat* ff) {
    infrared_metadata_reset(metadata);
    InfraredErrorCode error = InfraredErrorCodeNone;
    FuriString* line = furi_string_alloc();
    
    FURI_LOG_D(TAG, "Starting metadata read");
    
    Stream* stream = flipper_format_get_raw_stream(ff);
    if(!stream) {
        FURI_LOG_E(TAG, "Failed to get stream");
        furi_string_free(line);
        return InfraredErrorCodeFileOperationFailed;
    }

    // Store current position
    size_t pos = stream_tell(stream);
    
    do {
        // Rewind and skip header
        if(!stream_rewind(stream)) {
            FURI_LOG_E(TAG, "Failed to rewind stream");
            error = InfraredErrorCodeFileOperationFailed;
            break;
        }

        // Skip header (first two lines)
        for(int i = 0; i < 2; i++) {
            if(!stream_read_line(stream, line)) {
                FURI_LOG_E(TAG, "Failed to skip header line %d", i + 1);
                error = InfraredErrorCodeFileOperationFailed;
                break;
            }
        }
        if(error != InfraredErrorCodeNone) break;

        FURI_LOG_D(TAG, "Reading metadata comments");

        // Read lines until we find a non-comment
        while(!stream_eof(stream)) {
            if(!stream_read_line(stream, line)) break;
            const char* line_str = furi_string_get_cstr(line);
            
            // Skip non-comment or empty comment lines
            if(strncmp(line_str, "# ", 2) != 0 || strlen(line_str) <= 2) continue;

            // Parse "# Key: Value" format
            const char* content = line_str + 2;
            char* sep = strstr(content, ": ");
            if(!sep) continue;

            size_t key_len = sep - content;
            char* value = sep + 2;

            if(strncmp(content, "Brand", key_len) == 0) {
                furi_string_set(metadata->brand, value);
                FURI_LOG_D(TAG, "Found brand: '%s'", value);
            } else if(strncmp(content, "Device Type", key_len) == 0) {
                furi_string_set(metadata->device_type, value);
                FURI_LOG_D(TAG, "Found device type: '%s'", value);
            } else if(strncmp(content, "Model", key_len) == 0) {
                furi_string_set(metadata->model, value);
                FURI_LOG_D(TAG, "Found model: '%s'", value);
            }
        }

    } while(false);

    // Always try to restore position
    if(!stream_seek(stream, pos, StreamOffsetFromStart)) {
        FURI_LOG_E(TAG, "Failed to restore stream position");
        error = InfraredErrorCodeFileOperationFailed;
    }

    furi_string_free(line);
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

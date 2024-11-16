#include <core/check.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/string_stream.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>
#include "flipper_format.h"
#include "flipper_format_i.h"
#include "flipper_format_stream.h"
#include "flipper_format_stream_i.h"

/********************************** Private **********************************/
struct FlipperFormat {
    Stream* stream;
    bool strict_mode;
};

static const char* const flipper_format_filetype_key = "Filetype";
static const char* const flipper_format_version_key = "Version";
static const char* const TAG = "FlipperFormat";

Stream* flipper_format_get_raw_stream(FlipperFormat* flipper_format) {
    return flipper_format->stream;
}

/********************************** Public **********************************/

FlipperFormat* flipper_format_string_alloc(void) {
    FlipperFormat* flipper_format = malloc(sizeof(FlipperFormat));
    flipper_format->stream = string_stream_alloc();
    flipper_format->strict_mode = false;
    return flipper_format;
}

FlipperFormat* flipper_format_file_alloc(Storage* storage) {
    FlipperFormat* flipper_format = malloc(sizeof(FlipperFormat));
    flipper_format->stream = file_stream_alloc(storage);
    flipper_format->strict_mode = false;
    return flipper_format;
}

FlipperFormat* flipper_format_buffered_file_alloc(Storage* storage) {
    FlipperFormat* flipper_format = malloc(sizeof(FlipperFormat));
    flipper_format->stream = buffered_file_stream_alloc(storage);
    flipper_format->strict_mode = false;
    return flipper_format;
}

bool flipper_format_file_open_existing(FlipperFormat* flipper_format, const char* path) {
    furi_check(flipper_format);
    return file_stream_open(flipper_format->stream, path, FSAM_READ_WRITE, FSOM_OPEN_EXISTING);
}

bool flipper_format_buffered_file_open_existing(FlipperFormat* flipper_format, const char* path) {
    furi_check(flipper_format);
    return buffered_file_stream_open(
        flipper_format->stream, path, FSAM_READ_WRITE, FSOM_OPEN_EXISTING);
}

bool flipper_format_file_open_append(FlipperFormat* flipper_format, const char* path) {
    furi_check(flipper_format);

    bool result =
        file_stream_open(flipper_format->stream, path, FSAM_READ_WRITE, FSOM_OPEN_APPEND);

    // Add EOL if it is not there
    if(stream_size(flipper_format->stream) >= 1) {
        do {
            char last_char;
            result = false;

            if(!stream_seek(flipper_format->stream, -1, StreamOffsetFromEnd)) break;

            uint16_t bytes_were_read =
                stream_read(flipper_format->stream, (uint8_t*)&last_char, 1);
            if(bytes_were_read != 1) break;

            if(last_char != flipper_format_eoln) {
                if(!flipper_format_stream_write_eol(flipper_format->stream)) break;
            }

            result = true;
        } while(false);
    } else {
        stream_seek(flipper_format->stream, 0, StreamOffsetFromEnd);
    }

    return result;
}

bool flipper_format_file_open_always(FlipperFormat* flipper_format, const char* path) {
    furi_check(flipper_format);
    return file_stream_open(flipper_format->stream, path, FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
}

bool flipper_format_buffered_file_open_always(FlipperFormat* flipper_format, const char* path) {
    furi_check(flipper_format);
    return buffered_file_stream_open(
        flipper_format->stream, path, FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
}

bool flipper_format_file_open_new(FlipperFormat* flipper_format, const char* path) {
    furi_check(flipper_format);
    return file_stream_open(flipper_format->stream, path, FSAM_READ_WRITE, FSOM_CREATE_NEW);
}

bool flipper_format_file_close(FlipperFormat* flipper_format) {
    furi_check(flipper_format);
    return file_stream_close(flipper_format->stream);
}

bool flipper_format_buffered_file_close(FlipperFormat* flipper_format) {
    furi_check(flipper_format);
    return buffered_file_stream_close(flipper_format->stream);
}

void flipper_format_free(FlipperFormat* flipper_format) {
    furi_check(flipper_format);
    stream_free(flipper_format->stream);
    free(flipper_format);
}

void flipper_format_set_strict_mode(FlipperFormat* flipper_format, bool strict_mode) {
    flipper_format->strict_mode = strict_mode;
}

bool flipper_format_rewind(FlipperFormat* flipper_format) {
    furi_check(flipper_format);
    return stream_rewind(flipper_format->stream);
}

bool flipper_format_seek_to_end(FlipperFormat* flipper_format) {
    furi_check(flipper_format);
    return stream_seek(flipper_format->stream, 0, StreamOffsetFromEnd);
}

bool flipper_format_key_exist(FlipperFormat* flipper_format, const char* key) {
    size_t pos = stream_tell(flipper_format->stream);
    stream_seek(flipper_format->stream, 0, StreamOffsetFromStart);
    bool result = flipper_format_stream_seek_to_key(flipper_format->stream, key, false);
    stream_seek(flipper_format->stream, pos, StreamOffsetFromStart);

    return result;
}

bool flipper_format_read_header(
    FlipperFormat* flipper_format,
    FuriString* filetype,
    uint32_t* version) {
    furi_check(flipper_format);
    return flipper_format_read_string(flipper_format, flipper_format_filetype_key, filetype) &&
           flipper_format_read_uint32(flipper_format, flipper_format_version_key, version, 1);
}

bool flipper_format_write_header(
    FlipperFormat* flipper_format,
    FuriString* filetype,
    const uint32_t version) {
    furi_check(flipper_format);
    return flipper_format_write_header_cstr(
        flipper_format, furi_string_get_cstr(filetype), version);
}

bool flipper_format_write_header_cstr(
    FlipperFormat* flipper_format,
    const char* filetype,
    const uint32_t version) {
    furi_check(flipper_format);
    return flipper_format_write_string_cstr(
               flipper_format, flipper_format_filetype_key, filetype) &&
           flipper_format_write_uint32(flipper_format, flipper_format_version_key, &version, 1);
}

bool flipper_format_get_value_count(
    FlipperFormat* flipper_format,
    const char* key,
    uint32_t* count) {
    furi_check(flipper_format);
    return flipper_format_stream_get_value_count(
        flipper_format->stream, key, count, flipper_format->strict_mode);
}

bool flipper_format_read_string(FlipperFormat* flipper_format, const char* key, FuriString* data) {
    furi_check(flipper_format);
    return flipper_format_stream_read_value_line(
        flipper_format->stream, key, FlipperStreamValueStr, data, 1, flipper_format->strict_mode);
}

bool flipper_format_write_string(FlipperFormat* flipper_format, const char* key, FuriString* data) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueStr,
        .data = furi_string_get_cstr(data),
        .data_size = 1,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_write_string_cstr(
    FlipperFormat* flipper_format,
    const char* key,
    const char* data) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueStr,
        .data = data,
        .data_size = 1,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_read_hex_uint64(
    FlipperFormat* flipper_format,
    const char* key,
    uint64_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    return flipper_format_stream_read_value_line(
        flipper_format->stream,
        key,
        FlipperStreamValueHexUint64,
        data,
        data_size,
        flipper_format->strict_mode);
}

bool flipper_format_write_hex_uint64(
    FlipperFormat* flipper_format,
    const char* key,
    const uint64_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueHexUint64,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_read_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    uint32_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    return flipper_format_stream_read_value_line(
        flipper_format->stream,
        key,
        FlipperStreamValueUint32,
        data,
        data_size,
        flipper_format->strict_mode);
}

bool flipper_format_write_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueUint32,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_read_int32(
    FlipperFormat* flipper_format,
    const char* key,
    int32_t* data,
    const uint16_t data_size) {
    return flipper_format_stream_read_value_line(
        flipper_format->stream,
        key,
        FlipperStreamValueInt32,
        data,
        data_size,
        flipper_format->strict_mode);
}

bool flipper_format_write_int32(
    FlipperFormat* flipper_format,
    const char* key,
    const int32_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueInt32,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_read_bool(
    FlipperFormat* flipper_format,
    const char* key,
    bool* data,
    const uint16_t data_size) {
    return flipper_format_stream_read_value_line(
        flipper_format->stream,
        key,
        FlipperStreamValueBool,
        data,
        data_size,
        flipper_format->strict_mode);
}

bool flipper_format_write_bool(
    FlipperFormat* flipper_format,
    const char* key,
    const bool* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueBool,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_read_float(
    FlipperFormat* flipper_format,
    const char* key,
    float* data,
    const uint16_t data_size) {
    return flipper_format_stream_read_value_line(
        flipper_format->stream,
        key,
        FlipperStreamValueFloat,
        data,
        data_size,
        flipper_format->strict_mode);
}

bool flipper_format_write_float(
    FlipperFormat* flipper_format,
    const char* key,
    const float* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueFloat,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_read_hex(
    FlipperFormat* flipper_format,
    const char* key,
    uint8_t* data,
    const uint16_t data_size) {
    return flipper_format_stream_read_value_line(
        flipper_format->stream,
        key,
        FlipperStreamValueHex,
        data,
        data_size,
        flipper_format->strict_mode);
}

bool flipper_format_write_hex(
    FlipperFormat* flipper_format,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueHex,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_write_value_line(flipper_format->stream, &write_data);
    return result;
}

bool flipper_format_write_comment(FlipperFormat* flipper_format, FuriString* data) {
    furi_check(flipper_format);
    return flipper_format_write_comment_cstr(flipper_format, furi_string_get_cstr(data));
}

bool flipper_format_write_comment_cstr(FlipperFormat* flipper_format, const char* data) {
    furi_check(flipper_format);
    return flipper_format_stream_write_comment_cstr(flipper_format->stream, data);
}

static bool flipper_format_stream_read_comment_line(Stream* stream, const char* key, FuriString* data) {
    const size_t pos = stream_tell(stream);
    if(!stream_rewind(stream)) return false;

    furi_string_reset(data);
    FuriString* line = furi_string_alloc();
    bool found = false;

    while(!found && !furi_string_empty(line)) {
        // Read a complete line
        furi_string_reset(line);
        char c;
        
        while(stream_read(stream, (uint8_t*)&c, 1) == 1) {
            if(c == flipper_format_eolr) continue;
            if(c == flipper_format_eoln) break;
            furi_string_push_back(line, c);
        }

        // Check for metadata comment format: "# Key: Value"
        if(furi_string_start_with_str(line, "# ")) {
            size_t key_end = furi_string_search_str(line, ": ");
            if(key_end != FURI_STRING_FAILURE) {
                // Extract and compare key (skip "# " prefix)
                FuriString* found_key = furi_string_alloc();
                furi_string_set_n(found_key, line, 2, key_end - 2);

                if(furi_string_equal_str(found_key, key)) {
                    // Extract value (skip ": ")
                    furi_string_set_n(data, line, key_end + 2, furi_string_size(line) - (key_end + 2));
                    found = true;
                }
                furi_string_free(found_key);
            }
        }
    }

    furi_string_free(line);
    stream_seek(stream, pos, StreamOffsetFromStart);
    return found;
}

bool flipper_format_read_comment(FlipperFormat* flipper_format, const char* key, FuriString* data) {
    furi_check(flipper_format);
    furi_check(key);
    furi_check(data);
    return flipper_format_stream_read_comment_line(flipper_format->stream, key, data);
}

bool flipper_format_read_comment_cstr(FlipperFormat* flipper_format, const char* key, char* data) {
    FuriString* string = furi_string_alloc();
    bool result = flipper_format_read_comment(flipper_format, key, string);
    if(result) {
        strncpy(data, furi_string_get_cstr(string), strlen(furi_string_get_cstr(string)) + 1);
    }
    furi_string_free(string);
    return result;
}

bool flipper_format_delete_key(FlipperFormat* flipper_format, const char* key) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueIgnore,
        .data = NULL,
        .data_size = 0,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_string(FlipperFormat* flipper_format, const char* key, FuriString* data) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueStr,
        .data = furi_string_get_cstr(data),
        .data_size = 1,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_string_cstr(
    FlipperFormat* flipper_format,
    const char* key,
    const char* data) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueStr,
        .data = data,
        .data_size = 1,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size) {
    furi_check(flipper_format);
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueUint32,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_int32(
    FlipperFormat* flipper_format,
    const char* key,
    const int32_t* data,
    const uint16_t data_size) {
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueInt32,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_bool(
    FlipperFormat* flipper_format,
    const char* key,
    const bool* data,
    const uint16_t data_size) {
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueBool,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_float(
    FlipperFormat* flipper_format,
    const char* key,
    const float* data,
    const uint16_t data_size) {
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueFloat,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_update_hex(
    FlipperFormat* flipper_format,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size) {
    FlipperStreamWriteData write_data = {
        .key = key,
        .type = FlipperStreamValueHex,
        .data = data,
        .data_size = data_size,
    };
    bool result = flipper_format_stream_delete_key_and_write(
        flipper_format->stream, &write_data, flipper_format->strict_mode);
    return result;
}

bool flipper_format_insert_or_update_string(
    FlipperFormat* flipper_format,
    const char* key,
    FuriString* data) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_string(flipper_format, key, data);
    } else {
        result = flipper_format_update_string(flipper_format, key, data);
    }

    return result;
}

bool flipper_format_insert_or_update_string_cstr(
    FlipperFormat* flipper_format,
    const char* key,
    const char* data) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_string_cstr(flipper_format, key, data);
    } else {
        result = flipper_format_update_string_cstr(flipper_format, key, data);
    }

    return result;
}

bool flipper_format_insert_or_update_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_uint32(flipper_format, key, data, data_size);
    } else {
        result = flipper_format_update_uint32(flipper_format, key, data, data_size);
    }

    return result;
}

bool flipper_format_insert_or_update_int32(
    FlipperFormat* flipper_format,
    const char* key,
    const int32_t* data,
    const uint16_t data_size) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_int32(flipper_format, key, data, data_size);
    } else {
        result = flipper_format_update_int32(flipper_format, key, data, data_size);
    }

    return result;
}

bool flipper_format_insert_or_update_bool(
    FlipperFormat* flipper_format,
    const char* key,
    const bool* data,
    const uint16_t data_size) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_bool(flipper_format, key, data, data_size);
    } else {
        result = flipper_format_update_bool(flipper_format, key, data, data_size);
    }

    return result;
}

bool flipper_format_insert_or_update_float(
    FlipperFormat* flipper_format,
    const char* key,
    const float* data,
    const uint16_t data_size) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_float(flipper_format, key, data, data_size);
    } else {
        result = flipper_format_update_float(flipper_format, key, data, data_size);
    }

    return result;
}

bool flipper_format_insert_or_update_hex(
    FlipperFormat* flipper_format,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size) {
    bool result = false;

    if(!flipper_format_key_exist(flipper_format, key)) {
        flipper_format_seek_to_end(flipper_format);
        result = flipper_format_write_hex(flipper_format, key, data, data_size);
    } else {
        result = flipper_format_update_hex(flipper_format, key, data, data_size);
    }

    return result;
}

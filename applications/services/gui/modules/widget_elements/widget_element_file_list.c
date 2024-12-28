#include "assets_icons.h"
#include "path.h"
#include "widget_element_i.h"
#include <gui/elements.h>
#include <core/common_defines.h>
#include "archive/archive_i.h"
#include "assets_icons.h"

typedef struct {
    FuriString* name;
    const Icon* icon;
} FileListItem;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t lines;
    FileListItem* files;
    size_t count;
    size_t offset;
} FileListModel;

static void widget_element_file_list_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    FileListModel* model = element->model;
    size_t items_visible = MIN(model->lines, model->count);

    canvas_set_font(canvas, FontSecondary);
    for(size_t i = 0; i < items_visible; i++) {
        size_t idx = model->offset + i;
        if(idx < model->count) {
            canvas_draw_icon(
                canvas, model->x + 2, model->y + (i * FRAME_HEIGHT) - 9, model->files[idx].icon);
            canvas_draw_str(
                canvas,
                model->x + 15,
                model->y + (i * FRAME_HEIGHT),
                furi_string_get_cstr(model->files[idx].name));
        }
    }

    if(model->count > model->lines) {
        elements_scrollbar_pos(
            canvas,
            128,
            model->y - 9,
            model->lines * FRAME_HEIGHT,
            model->offset,
            model->count - 2);
    }
}

static bool widget_element_file_list_input(InputEvent* event, WidgetElement* element) {
    furi_assert(element);
    FileListModel* model = element->model;
    bool consumed = false;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        if(event->key == InputKeyUp) {
            model->offset = (model->offset > 0) ? model->offset - 1 : model->count - 3;
            consumed = true;
        } else if(event->key == InputKeyDown) {
            model->offset = ((model->offset + 3) < model->count) ? model->offset + 1 : 0;
            consumed = true;
        }
    }

    return consumed;
}

static void widget_element_file_list_free(WidgetElement* element) {
    furi_assert(element);
    FileListModel* model = element->model;
    for(size_t i = 0; i < model->count; i++) {
        furi_string_free(model->files[i].name);
    }
    free(model->files);
    free(model);
    free(element);
}

WidgetElement* widget_element_file_list_create(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t lines,
    FuriString** files,
    size_t count) {
    // Allocate and init model
    FileListModel* model = malloc(sizeof(FileListModel));
    model->x = x;
    model->y = y;
    model->lines = lines;
    model->count = count;
    model->offset = 0;
    model->files = malloc(sizeof(FileListItem) * count);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    for(size_t i = 0; i < count; i++) {
        model->files[i].name = furi_string_alloc();
        path_extract_filename(files[i], model->files[i].name, false);
        if(storage_dir_exists(storage, furi_string_get_cstr(files[i]))) {
            model->files[i].icon = &I_dir_10px;
        } else {
            const char* ext = strrchr(furi_string_get_cstr(model->files[i].name), '.');
            model->files[i].icon = (ext && strcasecmp(ext, ".js") == 0) ? &I_js_script_10px :
                                                                          &I_unknown_10px;
        }
    }
    furi_record_close(RECORD_STORAGE);

    // Allocate and init Element
    WidgetElement* element = malloc(sizeof(WidgetElement));
    element->draw = widget_element_file_list_draw;
    element->input = widget_element_file_list_input;
    element->free = widget_element_file_list_free;
    element->parent = widget;
    element->model = model;
    return element;
}

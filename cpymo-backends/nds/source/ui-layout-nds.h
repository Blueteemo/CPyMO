#ifndef CPYMO_NDS_UI_LAYOUT_H
#define CPYMO_NDS_UI_LAYOUT_H

#include "cpymo_backend_image.h"
#include "cpymo_backend_text.h"

float nds_ui_scale(void);
void nds_ui_native_rect(int x, int y, int width, int height, float rect[4]);
bool nds_ui_menu_stretch_band(cpymo_backend_image image, int *first, int *end);
error_t nds_ui_text_create(cpymo_backend_text *out, float *width,
    cpymo_str text, float size, unsigned nominal);
void nds_ui_text_ink(cpymo_backend_text text, int bounds[4]);
float nds_ui_centered_baseline(cpymo_backend_text text, float y, float height);
void nds_ui_text_at(cpymo_backend_text text, float x, float y, cpymo_color color);
void nds_ui_library_notice_background(void);
cpymo_color nds_ui_library_text_color(void);
struct cpymo_engine;
bool nds_ui_draw_dialogue_cursor(const struct cpymo_engine *, float slot_x, float slot_y,
    float previous_baseline, enum cpymo_backend_image_draw_type);

#endif

#include "cpymo_prelude.h"
#include "cpymo_msgbox_ui.h"
#include "cpymo_engine.h"
#include "cpymo_localization.h"
#include "cpymo_key_hold.h"
#include <stdlib.h>
#include <string.h>
#ifdef __NDS__
#include "../cpymo-backends/nds/source/ui-layout-nds.h"
#include "../cpymo-backends/nds/source/ui-language-nds.h"
#endif

#ifdef __3DS__
extern bool fill_screen_enabled;
const extern bool fill_screen;
const extern bool enhanced_3ds_display_mode;
#endif

typedef struct {
	cpymo_backend_text message;
	float message_width;

	cpymo_backend_text confirm_btn;
	float confirm_btn_width;
	cpymo_backend_text cancel_btn;
	float cancel_btn_width;
#ifdef __NDS__
	cpymo_backend_text detail;
	const char *cancel_label;
	float font_size;
	bool legacy;
	bool ignore_ok_release;
	bool touch_armed;
#endif

	int selection;

	error_t(*okcancel_callback)(cpymo_engine *e, void *data, bool);
	void *okcancel_callback_data;
	cpymo_key_hold mouse_button;
} cpymo_msgbox_ui;


static error_t cpymo_msgbox_ui_okcancel_finish(cpymo_engine *e, bool is_ok)
{
	cpymo_msgbox_ui *ui = (cpymo_msgbox_ui *)cpymo_ui_data(e);
	error_t(*func)(cpymo_engine *, void *, bool) = ui->okcancel_callback;
	void *data = ui->okcancel_callback_data;
	ui->okcancel_callback = NULL;
	ui->okcancel_callback_data = NULL;
	cpymo_ui_exit(e);
	return func ? func(e, data, is_ok) : CPYMO_ERR_SUCC;
}

static void cpymo_msgbox_ui_get_btn_rect(
	float *out_xywh,
	float txt_width,
	float font_size,
	float screen_w,
	float y)
{
	float btn_width = txt_width * 3.0f;
	float btn_height = font_size * 1.5f;

	out_xywh[0] = screen_w / 2 - btn_width / 2;
	out_xywh[1] = y;
	out_xywh[2] = btn_width;
	out_xywh[3] = btn_height;
}

static int cpymo_msgbox_ui_get_mouse_selection(cpymo_engine *e)
{
	cpymo_msgbox_ui *ui = (cpymo_msgbox_ui *)cpymo_ui_data(e);
	float fontsize = cpymo_gameconfig_font_size(&e->gameconfig);
#ifdef __NDS__
	fontsize = ui->font_size;
#endif

	float screen_w = (float)e->gameconfig.imagesize_w, screen_h = (float)e->gameconfig.imagesize_h;

	const cpymo_input *input = &e->input;
	if (!input->mouse_position_useable)
		input = &e->prev_input;

	float btn_width =
		ui->confirm_btn_width > ui->cancel_btn_width ?
		ui->confirm_btn_width : ui->cancel_btn_width;

	if (input->mouse_position_useable) {
		float xywh[4];
		cpymo_msgbox_ui_get_btn_rect(
			xywh,
			btn_width,
			fontsize,
			screen_w,
			ui->cancel_btn ? screen_h * 0.5f : screen_h * 0.618f);

		float x = input->mouse_x;
		float y = input->mouse_y;
		if (x >= xywh[0] && x <= xywh[0] + xywh[2] && y >= xywh[1] && y <= xywh[1] + xywh[3])
			return 0;

		if (ui->cancel_btn) {
			cpymo_msgbox_ui_get_btn_rect(
				xywh,
				btn_width,
				fontsize,
				screen_w,
				screen_h * 0.5f + fontsize * 1.75f + (8.0f / (fontsize / 32)));

			if (x >= xywh[0] && x <= xywh[0] + xywh[2] && y >= xywh[1] && y <= xywh[1] + xywh[3])
				return 1;
		}
	}

	return -1;
}

static error_t cpymo_msgbox_ui_update(cpymo_engine *e, void *ui_data, float dt)
{
	cpymo_msgbox_ui *ui = (cpymo_msgbox_ui *)ui_data;

	enum cpymo_key_hold_result mbs = cpymo_key_hold_update(
		e, &ui->mouse_button, dt, e->input.mouse_button);
	
	if (CPYMO_INPUT_JUST_RELEASED(e, cancel) || mbs == cpymo_key_hold_result_holding) {
		cpymo_msgbox_ui_okcancel_finish(e, false);
		return CPYMO_ERR_SUCC;
	}

#ifdef ENABLE_TEXT_EXTRACT
	const cpymo_localization *l = cpymo_localization_get(e);
#ifdef __NDS__
	const char *cancel_label = ui->cancel_label;
#else
	const char *cancel_label = l->msgbox_cancel;
#endif
#endif

	if ((CPYMO_INPUT_JUST_PRESSED(e, up) || CPYMO_INPUT_JUST_PRESSED(e, down))) {
		if (ui->selection == -1) {
			if (ui->cancel_btn) ui->selection = 1;
			else ui->selection = 0;
		}
		else if (ui->selection == 0 && ui->cancel_btn) ui->selection = 1;
		else if (ui->selection == 1 && ui->cancel_btn) ui->selection = 0;

		cpymo_engine_request_redraw(e);

		cpymo_backend_text_extract(ui->selection == 0 ? l->msgbox_ok : cancel_label);
	}
	else if (CPYMO_INPUT_JUST_PRESSED(e, ok)) {
		if (ui->cancel_btn == NULL) {
			ui->selection = 0;
		}

		cpymo_engine_request_redraw(e);
	}
	else if (CPYMO_INPUT_JUST_PRESSED(e, mouse_button)) {
		ui->selection = cpymo_msgbox_ui_get_mouse_selection(e);
#ifdef __NDS__
		ui->touch_armed = ui->selection >= 0;
#endif
		cpymo_engine_request_redraw(e);
	}
	else if (CPYMO_INPUT_JUST_RELEASED(e, ok)) {
#ifdef __NDS__
		if (ui->ignore_ok_release) {
			ui->ignore_ok_release = false;
			return CPYMO_ERR_SUCC;
		}
#endif
		if (ui->selection < 0) return CPYMO_ERR_SUCC;
		cpymo_engine_request_redraw(e);
		return cpymo_msgbox_ui_okcancel_finish(e, !ui->selection);
	}
	else if (CPYMO_INPUT_JUST_RELEASED(e, mouse_button)) {
		cpymo_engine_request_redraw(e);
		int mouse_sel = cpymo_msgbox_ui_get_mouse_selection(e);
#ifdef __NDS__
		if (ui->legacy && (!ui->touch_armed || mouse_sel < 0)) {
			ui->touch_armed = false;
			ui->selection = 1;
			return CPYMO_ERR_SUCC;
		}
#endif
		return cpymo_msgbox_ui_okcancel_finish(e, !mouse_sel);
	}

	if (cpymo_input_mouse_moved(e)) {
		int mouse_sel = cpymo_msgbox_ui_get_mouse_selection(e);
		if (mouse_sel != ui->selection) {
			ui->selection = mouse_sel;
			cpymo_engine_request_redraw(e);

			if (ui->selection >= 0)
				cpymo_backend_text_extract(
					ui->selection == 0 ? l->msgbox_ok : cancel_label);
		}
	}

	return CPYMO_ERR_SUCC;
}

static void cpymo_msgbox_ui_draw_btn(
	cpymo_backend_text btn_txt,
	float btn_width,
	float txt_width, 
	float font_size, 
	float screen_w,
	float screen_h,
	float y, 
	bool sel,
	bool key_down)
{
	float xywh[4];
	cpymo_msgbox_ui_get_btn_rect(xywh, btn_width, font_size, screen_w, y);

	cpymo_color gray;
	gray.r = 128;
	gray.g = 128;
	gray.b = 128;

	cpymo_backend_image_fill_rects(
		xywh,
		1,
		sel ? (key_down ? gray : cpymo_color_white) : cpymo_color_black,
		sel ? 1.0f : 0.5f,
		cpymo_backend_image_draw_type_ui_element);

	cpymo_backend_text_draw(
		btn_txt,
		screen_w / 2 - txt_width / 2,
#ifdef __NDS__
		nds_ui_centered_baseline(btn_txt, y, xywh[3]),
#else
		y + xywh[3] / 2 + font_size / 2 - font_size * 0.15f,
#endif
		sel ? cpymo_color_black : cpymo_color_white,
		1.0f,
		cpymo_backend_image_draw_type_ui_element);
}

static void cpymo_msgbox_ui_draw(const cpymo_engine *e, const void *ui_data)
{
	cpymo_bg_draw(e);
	cpymo_scroll_draw(&e->scroll);

	#ifdef __3DS__
	if (fill_screen && !enhanced_3ds_display_mode)
		fill_screen_enabled = false;
	#endif

	float screen_w = (float)e->gameconfig.imagesize_w, screen_h = (float)e->gameconfig.imagesize_h;
	float xywh[] = { -100, -100, screen_w + 200, screen_h + 200 };
	cpymo_backend_image_fill_rects(xywh, 1, cpymo_color_black, 0.5f, cpymo_backend_image_draw_type_bg);

	const cpymo_msgbox_ui *ui = (const cpymo_msgbox_ui *)ui_data;
	float fontsize = cpymo_gameconfig_font_size(&e->gameconfig);
#ifdef __NDS__
	fontsize = ui->font_size;
#endif

	float x = (screen_w - ui->message_width) / 2;
	float y = screen_h * 0.3f + fontsize / 2;
#ifdef __NDS__
	if (ui->legacy) {
		nds_ui_library_notice_background();
		const cpymo_backend_text lines[] = {ui->message, ui->detail};
		const float scale = nds_ui_scale();
		for (int i = 0; i < 2; ++i) {
			int ink[4];
			nds_ui_text_ink(lines[i], ink);
			float left = (screen_w - (ink[0] + ink[2]) / scale) / 2;
			cpymo_backend_text_draw(lines[i], left, y + i * (fontsize + 3 / scale),
				nds_ui_library_text_color(), 1, cpymo_backend_image_draw_type_ui_element);
		}
	} else
#endif
		cpymo_backend_text_draw(ui->message, x, y, cpymo_color_white, 1.0f, cpymo_backend_image_draw_type_ui_element);

	bool key_down = e->input.mouse_button || e->input.ok;

	float btn_width = ui->confirm_btn_width > ui->cancel_btn_width ? ui->confirm_btn_width : ui->cancel_btn_width;

	cpymo_msgbox_ui_draw_btn(
		ui->confirm_btn, 
		btn_width,
		ui->confirm_btn_width, 
		fontsize, 
		screen_w, 
		screen_h,
		ui->cancel_btn ? screen_h * 0.5f : screen_h * 0.618f,
		ui->selection == 0,
		key_down);

	if (ui->cancel_btn) {
		cpymo_msgbox_ui_draw_btn(
			ui->cancel_btn,
			btn_width,
			ui->cancel_btn_width,
			fontsize,
			screen_w,
			screen_h,
			screen_h * 0.5f + fontsize * 1.75f + (8.0f / (fontsize / 32)),
			ui->selection == 1,
			key_down);
	}

	#ifdef __3DS__
	fill_screen_enabled = true;
	#endif
}

static void cpymo_msgbox_ui_delete(struct cpymo_engine *e, void *ui_data)
{
	cpymo_msgbox_ui *ui = (cpymo_msgbox_ui *)ui_data;

	if (ui->okcancel_callback) {
#ifdef __NDS__
		if (!ui->legacy)
#endif
		ui->okcancel_callback(e, ui->okcancel_callback_data, false);
		ui->okcancel_callback = NULL;
		ui->okcancel_callback_data = NULL;
	}
	
	if (ui->message) cpymo_backend_text_free(ui->message);
#ifdef __NDS__
	if (ui->detail) cpymo_backend_text_free(ui->detail);
#endif
	if (ui->confirm_btn) cpymo_backend_text_free(ui->confirm_btn);
	if (ui->cancel_btn) cpymo_backend_text_free(ui->cancel_btn);
}

static error_t cpymo_msgbox_ui_default_confirm(
	struct cpymo_engine *e, void *d, bool b)
{
	return CPYMO_ERR_SUCC;
}

static error_t cpymo_msgbox_ui_enter_impl(
	cpymo_engine *e, 
	cpymo_str message, 
	error_t(*okcancel_callback)(cpymo_engine *e, void *data, bool is_confirm), 
	void *okcancel_callback_data, const char *detail, const char *back_label)
{
	cpymo_msgbox_ui *ui = NULL;
	error_t err = cpymo_ui_enter(
		(void **)&ui,
		e,
		sizeof(cpymo_msgbox_ui),
		&cpymo_msgbox_ui_update,
		&cpymo_msgbox_ui_draw,
		&cpymo_msgbox_ui_delete);
	CPYMO_THROW(err);

	ui->message = NULL;
	ui->confirm_btn = NULL;
	ui->cancel_btn = NULL;
	ui->cancel_btn_width = 0;
	ui->confirm_btn_width = 0;
	const cpymo_localization *l = cpymo_localization_get(e);

	ui->okcancel_callback = okcancel_callback;
	ui->okcancel_callback_data = okcancel_callback_data;
	ui->selection = okcancel_callback ? -1 : 0;
	cpymo_key_hold_init(&ui->mouse_button, e->input.mouse_button);

	float fontsize = cpymo_gameconfig_font_size(&e->gameconfig);

#ifdef __NDS__
	ui->detail = NULL;
	ui->legacy = detail != NULL;
	ui->cancel_label = back_label ? back_label : l->msgbox_cancel;
	ui->ignore_ok_release = ui->legacy && e->input.ok;
	ui->touch_armed = false;
	ui->font_size = fontsize;
	unsigned forced_nominal = 0;
	if (ui->legacy) {
		ui->selection = 1;
		float detail_width = cpymo_backend_text_width(cpymo_str_pure(detail), fontsize);
		float title_width = cpymo_backend_text_width(message, fontsize);
		if (detail_width * nds_ui_scale() > 240 || title_width * nds_ui_scale() > 240) {
			forced_nominal = 12;
			fontsize = 12 / nds_ui_scale();
		}
		ui->font_size = fontsize;
	}
	err = nds_ui_text_create(&ui->message, &ui->message_width, message, fontsize, forced_nominal);
	if (!err && detail) {
		float detail_width;
		err = nds_ui_text_create(&ui->detail, &detail_width, cpymo_str_pure(detail), fontsize, forced_nominal);
	}
#else
	(void)detail;
	err = cpymo_backend_text_create(&ui->message, &ui->message_width, message, fontsize);
#endif
	if (err != CPYMO_ERR_SUCC) {
		cpymo_ui_exit(e);
		return err;
	}

#ifdef __NDS__
	err = nds_ui_text_create(&ui->confirm_btn, &ui->confirm_btn_width,
		cpymo_str_pure(l->msgbox_ok), fontsize, forced_nominal);
#else
	err = cpymo_backend_text_create(
		&ui->confirm_btn, 
		&ui->confirm_btn_width, 
		cpymo_str_pure(l->msgbox_ok), 
		fontsize);
#endif
	if (err != CPYMO_ERR_SUCC) {
		cpymo_ui_exit(e);
		return err;
	}

	if (okcancel_callback) {
#ifdef __NDS__
		err = nds_ui_text_create(&ui->cancel_btn, &ui->cancel_btn_width,
			cpymo_str_pure(ui->cancel_label), fontsize, forced_nominal);
#else
		err = cpymo_backend_text_create(
			&ui->cancel_btn,
			&ui->cancel_btn_width,
			cpymo_str_pure(l->msgbox_cancel),
			fontsize);
#endif
		if (err != CPYMO_ERR_SUCC) {
			cpymo_ui_exit(e);
			return err;
		}
	}
	else {
		ui->okcancel_callback = &cpymo_msgbox_ui_default_confirm;
	}

	cpymo_engine_extract_text(e, message);
	if (detail) cpymo_engine_extract_text_cstr(e, detail);
	cpymo_engine_extract_text_cstr(e, back_label ? back_label : l->msgbox_cancel);
	cpymo_engine_extract_text_submit(e);

	return err;
}

error_t cpymo_msgbox_ui_enter(cpymo_engine *e, cpymo_str message,
	error_t (*choose)(cpymo_engine *, void *, bool), void *data)
{
	return cpymo_msgbox_ui_enter_impl(e, message, choose, data, NULL, NULL);
}

#ifdef __NDS__
error_t cpymo_msgbox_ui_enter_legacy_mo(cpymo_engine *e,
	error_t (*choose)(cpymo_engine *, void *, bool))
{
	static const char *const labels[4][3] = {
		{"此游戏使用旧版 MO 脚本", "请用 PyMO Legacy Converter 转换", "返回"},
		{"This game uses legacy MO scripts", "Convert with PyMO Legacy Converter", "Back"},
		{"此遊戲使用舊版 MO 腳本", "請用 PyMO Legacy Converter 轉換", "返回"},
		{"旧式の MO スクリプトです", "PyMO Legacy Converter で変換", "戻る"},
	};
	const char *const *lines = labels[nds_language_effective()];
	return cpymo_msgbox_ui_enter_impl(e, cpymo_str_pure(lines[0]), choose, NULL, lines[1], lines[2]);
}
#endif


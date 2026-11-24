#include "cpymo_prelude.h"
#include "cpymo_rmenu.h"
#include "cpymo_engine.h"
#include "cpymo_config_ui.h"
#include "cpymo_msgbox_ui.h"
#include "cpymo_save_ui.h"
#include "cpymo_save.h"
#include "cpymo_localization.h"
#include "ui-language-nds.h"
#include "ui-layout-nds.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifdef __3DS__
extern bool fill_screen_enabled;
const extern bool fill_screen;
const extern bool enhanced_3ds_display_mode;
#endif

#ifdef ENABLE_TEXT_EXTRACT_ANDROID_ACCESSIBILITY
#include "../cpymo-backends/android/app/src/main/cpp/include/cpymo_android.h"
#endif

typedef struct {
	cpymo_backend_image bg;
	int bg_w, bg_h;

	cpymo_select_img menu;
	cpymo_wait menu_waiter;

	cpymo_key_hold touch;
	float panel[4];
	int bg_band_start, bg_band_end;

	bool alive;
} cpymo_rmenu;

static error_t cpymo_rmenu_update(cpymo_engine *e, void *ui_data, float dt)
{
	cpymo_rmenu *r = (cpymo_rmenu *)ui_data;

	enum cpymo_key_hold_result result = cpymo_key_hold_update(
		e, &r->touch, dt, e->input.mouse_button);
	if (result == cpymo_key_hold_result_just_hold) {
		cpymo_ui_exit(e);
		return CPYMO_ERR_SUCC;
	}

	if (r->alive) {
		error_t err = cpymo_wait_update(&r->menu_waiter, e, dt);
		CPYMO_THROW(err);

		if (r->alive) {
			err = cpymo_select_img_update(e, &r->menu, dt);
			CPYMO_THROW(err);
		}
	}

	return CPYMO_ERR_SUCC;
}

static bool cpymo_rmenu_draw_framed_skin(const cpymo_rmenu *r)
{
	if (r->bg_band_end <= r->bg_band_start)
		return false;

	const float *panel = r->panel;
	const float scale = nds_ui_scale();
	const float art_scale = panel[2] * scale / r->bg_w;
	const int panel_height = (int)(panel[3] * scale + .5f);
	const int art_height = (int)(r->bg_h * art_scale + .5f);
	if (art_height >= panel_height)
		return false;

	const int top = (int)(r->bg_band_start * art_scale + .5f);
	const int bottom = (int)((r->bg_h - r->bg_band_end) * art_scale + .5f);
	const int middle = panel_height - top - bottom;
	if (top <= 0 || bottom <= 0 || middle <= 0)
		return false;

	/* Keep the title and corners at their width scale. Only the repeated
	   middle rows gain height, continuing the straight sides of the frame. */
	cpymo_backend_image_draw(panel[0], panel[1], panel[2], top / scale,
		r->bg, 0, 0, r->bg_w, r->bg_band_start, 1.0f,
		cpymo_backend_image_draw_type_ui_element_bg);
	cpymo_backend_image_draw(panel[0], panel[1] + top / scale, panel[2], middle / scale,
		r->bg, 0, r->bg_band_start, r->bg_w, r->bg_band_end - r->bg_band_start, 1.0f,
		cpymo_backend_image_draw_type_ui_element_bg);
	cpymo_backend_image_draw(panel[0], panel[1] + (top + middle) / scale, panel[2], bottom / scale,
		r->bg, 0, r->bg_band_end, r->bg_w, r->bg_h - r->bg_band_end, 1.0f,
		cpymo_backend_image_draw_type_ui_element_bg);
	return true;
}

static void cpymo_rmenu_draw(const cpymo_engine *e, const void *ui_data)
{
	const cpymo_rmenu *r = (const cpymo_rmenu *)ui_data;

	cpymo_bg_draw(e);
	cpymo_scroll_draw(&e->scroll);

	#ifdef __3DS__
	if (fill_screen && !enhanced_3ds_display_mode)
		fill_screen_enabled = false;
	#endif

	const float *panel = r->panel;
	if (r->bg && r->bg_w > 0 && r->bg_h > 0) {
		if (!cpymo_rmenu_draw_framed_skin(r)) {
			/* Illustrated skins retain the proportional center crop. */
			int source_w = r->bg_w, source_h = r->bg_h;
			if (source_w * panel[3] > source_h * panel[2])
				source_w = (int)(source_h * panel[2] / panel[3] + .5f);
			else
				source_h = (int)(source_w * panel[3] / panel[2] + .5f);
			if (source_w < 1) source_w = 1;
			if (source_h < 1) source_h = 1;
			cpymo_backend_image_draw(panel[0], panel[1], panel[2], panel[3], r->bg,
				(r->bg_w - source_w) / 2, (r->bg_h - source_h) / 2,
				source_w, source_h, 1.0f, cpymo_backend_image_draw_type_ui_element_bg);
		}
	}
	else {
		cpymo_backend_image_fill_rects(
			panel, 1, cpymo_color_black, 0.75f,
			cpymo_backend_image_draw_type_ui_element_bg);
	}

	/* NDS: retain the game's menu art, improve white bitmap text contrast. */
	cpymo_backend_image_fill_rects(panel, 1, cpymo_color_black, 0.60f,
		cpymo_backend_image_draw_type_ui_element_bg);

	cpymo_select_img_nds_menu_draw(&r->menu);

	#ifdef __3DS__
	fill_screen_enabled = true;
	#endif
}

static void cpymo_rmenu_delete(cpymo_engine *e, void *ui_data)
{
	cpymo_rmenu *r = (cpymo_rmenu *)ui_data;
	r->alive = false;

	if (r->bg) cpymo_backend_image_free(r->bg);

	cpymo_select_img_free(&r->menu);
}

extern void nds_request_title_guide_reset(void);
error_t cpymo_rmenu_restart_game(cpymo_engine *e, void *data, bool confirm)
{
	if (!confirm) return CPYMO_ERR_SUCC;
	char *gamedir =
		cpymo_str_copy_malloc(cpymo_str_pure(e->assetloader.gamedir));
	if (gamedir == NULL) return CPYMO_ERR_OUT_OF_MEM;

	cpymo_engine_free(e);
	error_t err = cpymo_engine_init(e, gamedir);
	free(gamedir);
	if (!err) nds_request_title_guide_reset();
	return err;
}

extern void nds_request_exit_to_library(void);

/* The message box exits its own UI before calling this callback.
 * The owning main loop tears down the engine AFTER cpymo_engine_update. */
static error_t cpymo_rmenu_exit_to_library(
    cpymo_engine *e, void *data, bool confirmed)
{
    (void)e; (void)data;
    if (confirmed) nds_request_exit_to_library();
    return CPYMO_ERR_SUCC;
}

static error_t cpymo_rmenu_ok(cpymo_engine *e, int sel, uint64_t hash, bool _)
{
	switch (sel) {
	case 0: cpymo_ui_exit(e); e->say.auto_mode = true; e->say.auto_mode_timer = -1; break;
	case 1: cpymo_ui_exit(e); cpymo_save_ui_enter(e, false); break;
	case 2: cpymo_ui_exit(e); cpymo_save_ui_enter(e, true); break;
	case 3: cpymo_ui_exit(e); e->skipping = true; break;
	case 4: cpymo_say_hidewindow_until_click(e); cpymo_ui_exit(e); break;
	case 5: cpymo_ui_exit(e); cpymo_backlog_ui_enter(e); break;
	case 6: cpymo_ui_exit(e); cpymo_config_ui_enter(e); break;
	case 7:
		cpymo_ui_exit(e);
		cpymo_msgbox_ui_enter(
			e,
			cpymo_str_pure(
				cpymo_localization_get(e)->rmenu_restat_game_confirm_message),
			&cpymo_rmenu_restart_game,
			NULL);
		break;

	case 8:
        cpymo_ui_exit(e);
        cpymo_msgbox_ui_enter(e,
            cpymo_str_pure(nds_ui("确定退出到游戏库？\n未保存的进度将丢失。")),
            &cpymo_rmenu_exit_to_library, NULL);
        break;
    case 9: cpymo_ui_exit(e); break;
	default:
		assert(false);
	}

	return CPYMO_ERR_SUCC;
}

error_t cpymo_rmenu_enter(cpymo_engine *e)
{
	cpymo_rmenu *rmenu = NULL;
	error_t err = cpymo_ui_enter(
		(void **)&rmenu,
		e,
		sizeof(cpymo_rmenu),
		&cpymo_rmenu_update,
		&cpymo_rmenu_draw,
		&cpymo_rmenu_delete);
	CPYMO_THROW(err);

#ifdef ENABLE_TEXT_EXTRACT_ANDROID_ACCESSIBILITY
	cpymo_android_play_sound(SOUND_MENU);
#endif

	rmenu->alive = true;

	cpymo_key_hold_init(&rmenu->touch, e->input.mouse_button);

	cpymo_select_img_init(&rmenu->menu);
	cpymo_wait_reset(&rmenu->menu_waiter);

	rmenu->bg = NULL;
	err = cpymo_assetloader_load_system_image(
		&rmenu->bg,
		&rmenu->bg_w,
		&rmenu->bg_h,
		cpymo_str_pure("menu"),
		&e->assetloader,
		true);

	rmenu->bg_band_start = rmenu->bg_band_end = 0;
	if (err != CPYMO_ERR_SUCC) {
		rmenu->bg = NULL;
	} else if (!nds_ui_menu_stretch_band(rmenu->bg, &rmenu->bg_band_start, &rmenu->bg_band_end)) {
		rmenu->bg_band_end = 0;
	}

	err = cpymo_select_img_configuare_begin(
		&rmenu->menu,
		10,
		cpymo_str_pure(""),
		&e->assetloader,
		&e->gameconfig);
	if (err != CPYMO_ERR_SUCC) {
		cpymo_ui_exit(e);
		return err;
	}

	const float font_size = cpymo_gameconfig_font_size(&e->gameconfig);

	#define RMENU_ITEM(_, TEXT, ENABLED) \
		err = cpymo_select_img_configuare_select_text( \
			&rmenu->menu, \
			&e->assetloader, \
			&e->gameconfig, \
			&e->flags, \
			cpymo_str_pure(TEXT), \
			ENABLED, \
			cpymo_select_img_selection_nohint, \
			0xFFFFFFFFFFFFFFFF, \
			font_size); \
		if (err != CPYMO_ERR_SUCC) { \
			cpymo_ui_exit(e); \
			return err; \
		}

	cpymo_select_img_configuare_set_ok_callback(&rmenu->menu, &cpymo_rmenu_ok);

	const cpymo_localization *l = cpymo_localization_get(e);

	RMENU_ITEM(0, l->rmenu_auto, e->select_img.selections == NULL);
	RMENU_ITEM(1, l->rmenu_save, true);
	RMENU_ITEM(2, l->rmenu_load, true);
	RMENU_ITEM(3, l->rmenu_skip, e->select_img.selections == NULL);
	RMENU_ITEM(4, l->rmenu_hide_window, e->select_img.selections == NULL);
	RMENU_ITEM(5, l->rmenu_backlog, true);
	RMENU_ITEM(6, l->rmenu_config, true)
	RMENU_ITEM(7, l->rmenu_restart, true);
	RMENU_ITEM(8, nds_ui("退出到游戏库"), true);
	RMENU_ITEM(9, l->rmenu_back_to_game, true);

	const bool large = font_size * nds_ui_scale() >= 15;
	const int panel_top = large ? 1 : 8;
	const int content_top = panel_top + (large ? 18 : 25);
	const int content_bottom = 192 - panel_top - (large ? 3 : 5);
	nds_ui_native_rect(8, panel_top, 240, 192 - 2 * panel_top, rmenu->panel);
	cpymo_select_img_nds_menu_layout(&rmenu->menu, 8, content_top, 240,
		content_bottom - content_top, large ? 2 : 3);
	cpymo_select_img_configuare_end(&rmenu->menu, &rmenu->menu_waiter, e, 0);

	rmenu->menu.draw_type = cpymo_backend_image_draw_type_ui_element;

	return CPYMO_ERR_SUCC;
}

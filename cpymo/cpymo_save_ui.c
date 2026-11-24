#include "cpymo_prelude.h"
#include "cpymo_save_ui.h"
#include "cpymo_engine.h"
#include "cpymo_list_ui.h"
#include "cpymo_save.h"
#include "cpymo_msgbox_ui.h"
#include "cpymo_localization.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#ifdef __NDS__
#include "../cpymo-backends/nds/source/ui-layout-nds.h"
#endif

#ifndef CPYMO_MAX_SAVES
#define CPYMO_MAX_SAVES 31
#endif

typedef struct {
	cpymo_backend_text text;
#ifdef __NDS__
	cpymo_backend_text body[2], date;
	bool large;
#endif
#ifdef ENABLE_TEXT_EXTRACT
	char *orginal_text;
#endif
	bool is_empty_save;
} cpymo_save_ui_item;

typedef struct {
	cpymo_save_ui_item items[CPYMO_MAX_SAVES];
	bool is_load_ui;
} cpymo_save_ui;

#ifdef DISABLE_AUTOSAVE
const static size_t first_slot = 0;
#else
const static size_t first_slot = 1;
#endif

static void cpymo_save_ui_draw_node(const cpymo_engine *e, const void *node_to_draw, float y)
{
	const cpymo_save_ui *ui = (const cpymo_save_ui *)cpymo_list_ui_data_const(e);
	const cpymo_save_ui_item *item = &ui->items[cpymo_list_ui_encode_uint_node_dec(node_to_draw)];

#ifdef __NDS__
	const float scale = nds_ui_scale();
	const float inset = 8 / scale;
	const float row_scale = e->gameconfig.imagesize_h * scale / 192;
	const int small[] = {5, 20, 34, 51};
	const int large[] = {1, 18, 35, 53};
	const int *top = item->large ? large : small;
	const cpymo_color secondary = {214, 222, 239};
	nds_ui_text_at(item->text, inset, y + top[0] * row_scale / scale,
		item->is_empty_save ? secondary : cpymo_color_white);
	for (unsigned i = 0; i < 2; ++i)
		if (item->body[i])
			nds_ui_text_at(item->body[i], inset, y + top[i+1] * row_scale / scale, cpymo_color_white);
	if (item->date)
		nds_ui_text_at(item->date, inset, y + top[3] * row_scale / scale, secondary);
#else
	cpymo_backend_text_draw(
		item->text,
		0,
		y + cpymo_gameconfig_font_size(&e->gameconfig),
		cpymo_color_white,
		1.0f,
		cpymo_backend_image_draw_type_ui_element);
#endif
}

static error_t cpymo_save_confirm(cpymo_engine *e, void *data, bool confirm)
{
	if (!confirm) return CPYMO_ERR_SUCC;
	uintptr_t save_id = cpymo_list_ui_encode_uint_node_dec(data);
	error_t err = cpymo_save_write(e, (unsigned short)save_id);

	const cpymo_localization *l = cpymo_localization_get(e);

	char *msg = NULL;
	error_t errstr;
	if (err == CPYMO_ERR_SUCC) 
		errstr = l->save_already_save_to(&msg, (int)save_id);
	else 
		errstr = l->save_failed(&msg, err);
	
	if (errstr != CPYMO_ERR_SUCC) return errstr;

	cpymo_ui_exit(e);

	err = cpymo_msgbox_ui_enter(
		e,
		cpymo_str_pure(msg),
		NULL,
		NULL);

	free(msg);

	return err;
}

static error_t cpymo_save_ui_ok(cpymo_engine *e, void *selected)
{
	cpymo_save_ui *ui = (cpymo_save_ui *)cpymo_list_ui_data(e);

	uintptr_t save_id = cpymo_list_ui_encode_uint_node_dec(selected);

	if (ui->is_load_ui) {
		if (!ui->items[save_id].is_empty_save) {
			cpymo_list_ui_ignore_key_hold_exit_onetime(e);
			return cpymo_save_ui_load_savedata_yesnobox(e, (unsigned short)save_id);
		}
		else return CPYMO_ERR_SUCC;
	}
	else {
		char *msg = NULL;
		const cpymo_localization *l = cpymo_localization_get(e);
		error_t err = l->save_are_you_sure_save_to(&msg, (int)save_id);
		CPYMO_THROW(err);
		err = cpymo_msgbox_ui_enter(
			e,
			cpymo_str_pure(msg),
			&cpymo_save_confirm,
			selected);
		free(msg);
		return err;
	}
}

static void cpymo_save_ui_deleter(cpymo_engine *e, void *ui_data)
{
	cpymo_save_ui *ui = (cpymo_save_ui *)cpymo_list_ui_data(e);

	for (size_t i = 0; i < CPYMO_MAX_SAVES; ++i) {
		if (ui->items[i].text) 
			cpymo_backend_text_free(ui->items[i].text);
#ifdef __NDS__
		for (unsigned row = 0; row < 2; ++row)
			if (ui->items[i].body[row]) cpymo_backend_text_free(ui->items[i].body[row]);
		if (ui->items[i].date) cpymo_backend_text_free(ui->items[i].date);
#endif
#ifdef ENABLE_TEXT_EXTRACT
		if (ui->items[i].orginal_text) free(ui->items[i].orginal_text);
#endif
	}
}

static void* cpymo_save_ui_get_next(const cpymo_engine *e, const void *ui_data, const void *cur)
{
	uintptr_t i = cpymo_list_ui_encode_uint_node_dec(cur);
	if (i >= CPYMO_MAX_SAVES - 1) 
		return NULL;
	else return cpymo_list_ui_encode_uint_node_enc(i + 1);
}

static void* cpymo_save_ui_get_prev(const cpymo_engine *e, const void *ui_data, const void *cur)
{
	const cpymo_save_ui *ui = (const cpymo_save_ui *)cpymo_list_ui_data_const(e);

	uintptr_t i = cpymo_list_ui_encode_uint_node_dec(cur);
	if (i == (ui->is_load_ui ? 0 : first_slot)) return NULL;
	else return cpymo_list_ui_encode_uint_node_enc(i - 1);
}

#ifdef ENABLE_TEXT_EXTRACT
static error_t cpymo_save_ui_visual_impaired_selection_changed(cpymo_engine *e, void *cur)
{
	if (cur) {
		uintptr_t i = cpymo_list_ui_encode_uint_node_dec(cur);
		const cpymo_save_ui *ui = (const cpymo_save_ui *)cpymo_list_ui_data(e);
		cpymo_backend_text_extract(ui->items[i].orginal_text);
	}
	return CPYMO_ERR_SUCC;
}
#endif

#ifdef __NDS__
static cpymo_str nds_save_take_line(cpymo_str *rest, float fontsize, float width)
{
	cpymo_str line = {rest->begin, 0};
	float used = 0;
	while (rest->len) {
		if (rest->begin[0] == '\n' || rest->begin[0] == '\r') {
			char newline = rest->begin[0];
			++rest->begin;
			--rest->len;
			if (newline == '\r' && rest->len && rest->begin[0] == '\n') {
				++rest->begin;
				--rest->len;
			}
			break;
		}
		cpymo_str tail = *rest;
		cpymo_str ch = cpymo_str_utf8_try_head(&tail);
		float advance = cpymo_backend_text_width(ch, fontsize);
		if (!ch.len || used + advance > width) break;
		used += advance;
		line.len += ch.len;
		*rest = tail;
	}
	return line;
}

static error_t nds_save_make_line(cpymo_backend_text *out, cpymo_str *rest,
	float fontsize, float width, bool final)
{
	cpymo_str original = *rest;
	cpymo_str line = nds_save_take_line(rest, fontsize, width);
	float advance;
	if (!final || !rest->len)
		return cpymo_backend_text_create(out, &advance, line, fontsize);
	const char *ending = "…";
	const size_t ending_len = strlen(ending);
	float ending_width = cpymo_backend_text_width(cpymo_str_pure(ending), fontsize);
	line = nds_save_take_line(&original, fontsize, width - ending_width);
	char *display = malloc(line.len + ending_len + 1);
	if (!display) return CPYMO_ERR_OUT_OF_MEM;
	memcpy(display, line.begin, line.len);
	memcpy(display + line.len, ending, ending_len + 1);
	error_t err = cpymo_backend_text_create(out, &advance, cpymo_str_pure(display), fontsize);
	free(display);
	return err;
}

static error_t nds_save_make_item(cpymo_engine *e, cpymo_save_ui_item *item, unsigned slot)
{
	cpymo_save_title title = {0};
	char date[24] = {0};
	FILE *save = cpymo_save_open_read(e, slot);
	item->is_empty_save = !save;
	if (save) {
		item->is_empty_save = cpymo_save_load_title(&title, save) != CPYMO_ERR_SUCC ||
			!title.title || !title.say_name || !title.say_text;
		fclose(save);
		if (!item->is_empty_save) cpymo_save_slot_date(e, slot, date);
	}
	const cpymo_localization *l = cpymo_localization_get(e);
	char *heading = NULL;
	const char *chapter = item->is_empty_save ? "" : title.title;
#ifndef DISABLE_AUTOSAVE
	error_t err = slot ? l->save_title(&heading, slot, chapter) : l->save_auto_title(&heading, chapter);
#else
	error_t err = l->save_title(&heading, slot, chapter);
#endif
	const float fontsize = cpymo_gameconfig_font_size(&e->gameconfig);
	const float scale = nds_ui_scale();
	const float width = e->gameconfig.imagesize_w - 16 / scale;
	item->large = fontsize * scale >= 15;
	if (!err) {
		cpymo_str text = cpymo_str_pure(heading);
		err = nds_save_make_line(&item->text, &text, fontsize, width, true);
	}
	if (!err && !item->is_empty_save) {
		size_t len = strlen(title.say_name) + strlen(title.say_text) + 2;
		char *body = malloc(len);
		if (!body) err = CPYMO_ERR_OUT_OF_MEM;
		else {
			snprintf(body, len, "%s%s%s", title.say_name, *title.say_name ? " " : "", title.say_text);
			cpymo_utils_replace_str_newline_n(body);
			cpymo_str rest = cpymo_str_pure(body);
			for (unsigned i = 0; i < 2 && rest.len && !err; ++i)
				err = nds_save_make_line(&item->body[i], &rest, fontsize, width, i == 1);
#ifdef ENABLE_TEXT_EXTRACT
			const size_t accessible_len = strlen(heading) + strlen(body) + strlen(date) + 3;
			item->orginal_text = malloc(accessible_len);
			if (item->orginal_text)
				snprintf(item->orginal_text, accessible_len, "%s\n%s\n%s", heading, body, date);
#endif
			free(body);
		}
		if (!err && date[0]) {
			cpymo_str text = cpymo_str_pure(date);
			err = nds_save_make_line(&item->date, &text, fontsize, width, true);
		}
	}
#ifdef ENABLE_TEXT_EXTRACT
	if (heading && !item->orginal_text)
		item->orginal_text = cpymo_str_copy_malloc(cpymo_str_pure(heading));
#endif
	free(heading);
	free(title.title);
	free(title.say_name);
	free(title.say_text);
	return err;
}
#endif

error_t cpymo_save_ui_enter(cpymo_engine *e, bool is_load_ui)
{
	cpymo_save_ui *ui = NULL;
	uintptr_t first = is_load_ui ? 0 : first_slot;
	error_t err = cpymo_list_ui_enter(
		e,
		(void **)&ui,
		sizeof(*ui),
		&cpymo_save_ui_draw_node,
		&cpymo_save_ui_ok,
		&cpymo_save_ui_deleter,
		cpymo_list_ui_encode_uint_node_enc(first),
		&cpymo_save_ui_get_next,
		&cpymo_save_ui_get_prev,
		false,
		3);
	CPYMO_THROW(err);
	
	cpymo_list_ui_enable_loop(e);

#ifdef ENABLE_TEXT_EXTRACT
	cpymo_list_ui_set_selection_changed_callback(e, &cpymo_save_ui_visual_impaired_selection_changed);
	ui->items[0].orginal_text = NULL;
#endif

	ui->is_load_ui = is_load_ui;

#ifdef __NDS__
	memset(ui->items, 0, sizeof ui->items);
	for (unsigned i = first; i < CPYMO_MAX_SAVES; ++i) {
		err = nds_save_make_item(e, &ui->items[i], i);
		if (err) {
			cpymo_ui_exit(e);
			return err;
		}
	}
#else
	for (size_t i = 0; i < CPYMO_MAX_SAVES; ++i) {
		ui->items[i].text = NULL;
		ui->items[i].is_empty_save = false;
	}

	const float fontsize = cpymo_gameconfig_font_size(&e->gameconfig);
	size_t characters = (size_t)((float)e->gameconfig.imagesize_w / fontsize * 1.3f);

	char text_buf[1024];
	for (size_t i = is_load_ui ? 0 : first_slot; i < CPYMO_MAX_SAVES; ++i) {
		FILE *save = cpymo_save_open_read(e, (unsigned short)i);
		cpymo_save_title title;
		title.say_name = NULL;
		title.say_text = NULL;
		title.title = NULL;

		if (save) {
			error_t err = cpymo_save_load_title(&title, save);
			fclose(save);
			if (err != CPYMO_ERR_SUCC) ui->items[i].is_empty_save = true;
		}
		else {
			ui->items[i].is_empty_save = true;
		}

		if (!ui->items[i].is_empty_save) {
			assert(title.say_name != NULL);
			assert(title.title != NULL);
			assert(title.say_text != NULL);

			char *tmp_str = (char *)malloc(strlen(title.say_name) + strlen(title.say_text) + 16);
			if (tmp_str == NULL) {
				cpymo_ui_exit(e);
				free(title.say_name);
				free(title.say_text);
				free(title.title);
				return CPYMO_ERR_OUT_OF_MEM;
			}

			*tmp_str = '\0';
			if (strlen(title.say_name) > 0) {
				strcat(tmp_str, title.say_name);
				strcat(tmp_str, " ");
			}

			strcat(tmp_str, title.say_text);

			cpymo_str say_preview_text_tail = cpymo_str_pure(tmp_str);
			cpymo_str say_preview_text = 
				cpymo_str_split(&say_preview_text_tail, characters);

			if (say_preview_text_tail.len > 0) {
				*(char *)say_preview_text_tail.begin = '\0';

				if (say_preview_text.len > 3) {
					((char *)say_preview_text.begin)[say_preview_text.len - 1] = '.';
					((char *)say_preview_text.begin)[say_preview_text.len - 2] = '.';
					((char *)say_preview_text.begin)[say_preview_text.len - 3] = '.';
				}
			}

			const cpymo_localization *l = cpymo_localization_get(e);

			char *str = NULL;
			error_t err;

			#ifndef DISABLE_AUTOSAVE
			if (i == 0) err = l->save_auto_title(&str, title.title);
			else 
			#endif
				err = l->save_title(&str, (int)i, title.title);

			if (err != CPYMO_ERR_SUCC) {
				free(tmp_str);
				free(title.say_name);
				free(title.title);
				free(title.say_text);
				return err;
			}

			snprintf(text_buf, sizeof(text_buf), "%s\n%s", str, tmp_str);
			free(str);

			free(tmp_str);

			free(title.say_name);
			free(title.title);
			free(title.say_text);

			cpymo_utils_replace_str_newline_n(text_buf);
		}
		else {
			error_t err;
			const cpymo_localization *l = cpymo_localization_get(e);
			char *msg = NULL;

			#ifndef DISABLE_AUTOSAVE
			if (i == 0) err = l->save_auto_title(&msg, "");
			else 
			#endif
				err = l->save_title(&msg, (int)i, "");

			CPYMO_THROW(err);
			strncpy(text_buf, msg, sizeof(text_buf) - 1);
			free(msg);
		}

		float w;
		error_t err = cpymo_backend_text_create(
			&ui->items[i].text, &w, cpymo_str_pure(text_buf), fontsize);

#ifdef ENABLE_TEXT_EXTRACT
		ui->items[i].orginal_text = 
			cpymo_str_copy_malloc(cpymo_str_pure(text_buf));
#endif

		if (err != CPYMO_ERR_SUCC) {
			cpymo_ui_exit(e);
			return err;
		}
	}
#endif

	return CPYMO_ERR_SUCC;
}

static error_t cpymo_save_ui_load_savedata_yesnobox_confirm(
	cpymo_engine *e, void *save_id_x, bool confirm)
{
	if (!confirm) return CPYMO_ERR_SUCC;
	FILE *file = cpymo_save_open_read(e, (unsigned short)(uintptr_t)save_id_x);
	if (file == NULL) return CPYMO_ERR_CAN_NOT_OPEN_FILE;
	error_t err = cpymo_save_load_savedata(e, file);
	fclose(file);
	return err;
}

error_t cpymo_save_ui_load_savedata_yesnobox(cpymo_engine * e, unsigned short save_id)
{
	char *str = NULL;
	error_t err = CPYMO_ERR_SUCC;
	const cpymo_localization *l = cpymo_localization_get(e);

	#ifndef DISABLE_AUTOSAVE
	if (save_id) 
	#endif
	{
		err = l->save_are_you_sure_load(&str, (int)save_id);
		CPYMO_THROW(err);
	}

	err = cpymo_msgbox_ui_enter(
		e,
		cpymo_str_pure(str ? str : l->save_are_you_sure_load_auto_save),
		&cpymo_save_ui_load_savedata_yesnobox_confirm,
		(void *)(uintptr_t)save_id);

	if (str) free(str);

	return err;
}



/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

struct matte_clipper_data {
	gs_effect_t *effect;
};

static const char *matte_clipper_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Matte Clipper";
}

static void *matte_clipper_create(obs_data_t *settings, obs_source_t *source)
{
	struct matte_clipper_data *filter = bzalloc(sizeof(struct matte_clipper_data));
	char *effect_path = obs_module_file("matte_clipper.effect");

	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	obs_leave_graphics();

	bfree(effect_path);

	if (!filter->effect) {
		bfree(filter);
		return NULL;
	}

	return filter;
}

static void matte_clipper_destroy(void *data)
{
	struct matte_clipper_data *filter = data;

	if (filter->effect) {
		obs_enter_graphics();
		gs_effect_destroy(filter->effect);
		obs_leave_graphics();
	}

	bfree(filter);
}

static uint32_t matte_clipper_get_width(void *data, uint32_t width)
{
	return width;
}

static uint32_t matte_clipper_get_height(void *data, uint32_t height)
{
	return height;
}

static void matte_clipper_render(void *data, gs_effect_t *effect)
{
	struct matte_clipper_data *filter = data;

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
					     OBS_ALLOW_DIRECT_RENDERING))
		return;

	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_color(true, true, true, false);

	if (filter->effect) {
		obs_source_process_filter_end(filter->context, filter->effect,
					      0, 0);
	} else {
		obs_source_process_filter_end(filter->context, effect, 0, 0);
	}

	gs_blend_state_pop();
}

struct obs_source_info matte_clipper = {
	.id = "matte_clipper",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = matte_clipper_get_name,
	.create = matte_clipper_create,
	.destroy = matte_clipper_destroy,
	.get_width = matte_clipper_get_width,
	.get_height = matte_clipper_get_height,
	.video_render = matte_clipper_render,
};

bool obs_module_load(void)
{
	obs_register_source(&matte_clipper);
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}

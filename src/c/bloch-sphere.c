// copyrights (c) 2022 - thyung

#include <pebble.h>
#include "coordmap.h"

static Window *s_window;
static TextLayer *s_text_layer;

static Layer *s_canvas_layer;
static struct tm s_tick_time;
char s_str_time[20];

float prv_sin(float angle) {
  float value = sin_lookup(TRIG_MAX_ANGLE * angle / 360);
  return value / TRIG_MAX_RATIO;
}

float prv_cos(float angle) {
  float value = cos_lookup(TRIG_MAX_ANGLE * angle / 360);
  return value / TRIG_MAX_RATIO;
}

GPoint prv_gpoint_from_3d(float x, float y, float z) {
  float point_x, point_y;
  coordmap_3d2d(x, y, z, &point_x, &point_y);  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "from (%d, %d, %d) to (%d, %d)", 
          (int)(x*1000), (int)(y*1000), (int)(z*1000), (int)point_x, (int)point_y);
  return GPoint((int)point_x, (int)point_y);
}

void prv_draw_line(GContext *ctx, float x1, float y1, float z1, float x2, float y2, float z2) {
  if (x1==0 && y1==0 && z1==0) {
    if (coordmap_is_front(x2, y2, z2)) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
    } else {
      graphics_context_set_stroke_color(ctx, GColorDarkGray);
    }
  } else {
    if (coordmap_is_front(x1, y1, z1)) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
    } else {
      graphics_context_set_stroke_color(ctx, GColorDarkGray);
    }
  }
  graphics_draw_line(ctx, 
                    prv_gpoint_from_3d(x1, y1, z1), 
                    prv_gpoint_from_3d(x2, y2, z2));
}

void prv_draw_text(GContext *ctx, const char *str, float x, float y, float z) {
  GPoint point = prv_gpoint_from_3d(x, y, z);
  if (coordmap_is_front(x, y, z)) {
    graphics_context_set_text_color(ctx, GColorBlack);
  } else {
    graphics_context_set_text_color(ctx, GColorDarkGray);
  }
  graphics_draw_text(ctx, 
                    str, 
                    fonts_get_system_font(FONT_KEY_GOTHIC_14), 
                    GRect(point.x, point.y, 20, 20),
                    GTextOverflowModeTrailingEllipsis,
                    GTextAlignmentLeft, 
                    NULL);

}
void prv_canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int scale = 1;
  float theta, phi;
  float psi_x, psi_y, psi_z;

  if (bounds.size.w <= bounds.size.h) {
    scale = bounds.size.w / 2 - 20;
  } else {
    scale = bounds.size.h / 2 - 20;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "prv_canvas_update_proc %d", scale);
  
  graphics_draw_circle(ctx, 
                    GPoint(bounds.origin.x + bounds.size.w/2, bounds.origin.y + bounds.size.h/2),
                    scale);

  float view_angle = s_tick_time.tm_sec * 360 / 60;
  coordmap_set_normal(prv_cos(view_angle), prv_sin(view_angle), 0.4);
  coordmap_set_scale(scale, -scale);
  coordmap_set_offset(bounds.origin.x + bounds.size.w/2, bounds.origin.y + bounds.size.h/2);
  
  // draw axis
  float arrow_peak = 1.2;
  float arrow_len = 0.1;
  prv_draw_line(ctx, 0, 0, 0, arrow_peak, 0, 0);
  prv_draw_line(ctx, 0, 0, 0, 0, arrow_peak, 0);
  prv_draw_line(ctx, 0, 0, 0, 0, 0, arrow_peak);
  prv_draw_line(ctx, 0, 0, 0, -1, 0, 0);
  prv_draw_line(ctx, 0, 0, 0, 0, -1, 0);
  prv_draw_line(ctx, 0, 0, 0, 0, 0, -1);
  // draw arrow
  prv_draw_line(ctx, arrow_peak, 0, 0, arrow_peak-arrow_len, 0, arrow_len);
  prv_draw_line(ctx, arrow_peak, 0, 0, arrow_peak-arrow_len, 0, -arrow_len);
  prv_draw_line(ctx, 0, arrow_peak, 0, 0, arrow_peak-arrow_len, arrow_len);
  prv_draw_line(ctx, 0, arrow_peak, 0, 0, arrow_peak-arrow_len, -arrow_len);
  prv_draw_line(ctx, 0, 0, arrow_peak, arrow_len, 0, arrow_peak-arrow_len);
  prv_draw_line(ctx, 0, 0, arrow_peak, -arrow_len, 0, arrow_peak-arrow_len);

  prv_draw_text(ctx, "x", arrow_peak, 0, 0);
  prv_draw_text(ctx, "y", 0, arrow_peak, 0);
  prv_draw_text(ctx, "z", 0, 0, arrow_peak + 3 * arrow_len);

  // calculate theta and phi
  theta = s_tick_time.tm_hour * 180.0 / 24;
  phi = s_tick_time.tm_min * 360.0 / 60;
  // calculate psi coordinates
  psi_z = prv_cos(theta);
  psi_x = prv_sin(theta) * prv_cos(phi);
  psi_y = prv_sin(theta) * prv_sin(phi);

  if (coordmap_is_front(psi_x, psi_y, psi_z)) {
    graphics_context_set_fill_color(ctx, GColorBlack);
  } else {
    graphics_context_set_fill_color(ctx, GColorDarkGray);
  }
  graphics_fill_circle(ctx, prv_gpoint_from_3d(psi_x, psi_y, psi_z), 5);
  prv_draw_line(ctx, psi_x, psi_y, psi_z, 0, 0, 0);
  prv_draw_line(ctx, psi_x, psi_y, psi_z, psi_x, psi_y, 0);
  prv_draw_line(ctx, 0, 0, 0, psi_x, psi_y, 0);
}

void prv_handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  s_tick_time = *tick_time;
  snprintf(s_str_time, sizeof(s_str_time), "%02d:%02d:%02d", 
            s_tick_time.tm_hour, s_tick_time.tm_min, s_tick_time.tm_sec);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_text_layer, s_str_time);

  layer_mark_dirty(s_canvas_layer);
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up");
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down");
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, bounds.size.h-30, bounds.size.w, 30));
  text_layer_set_text(s_text_layer, "Press a button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h-30));
  layer_add_child(window_layer, s_canvas_layer);
  layer_set_update_proc(s_canvas_layer, prv_canvas_update_proc);
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, prv_handle_tick);

}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}

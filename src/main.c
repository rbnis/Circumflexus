#include <pebble.h>
#include <time.h>

// Settings
#define KEY_COLOR_TEXT 1
#define KEY_COLOR_INDICATOR 2
#define KEY_COLOR_RING_1 3
#define KEY_COLOR_RING_2 4
#define KEY_COLOR_BG_IN 5
#define KEY_COLOR_BG_OUT 6
#define KEY_SHOW_DAY 7
#define KEY_SHOW_DATE 8
#define KEY_VIBE_BT 9
	
static Window *s_window_main;
TextLayer *s_layer_day;
TextLayer *s_layer_time;
TextLayer *s_layer_date;
Layer *layer_bg_in;
Layer *layer_ring;
	
static const GPathInfo PATH_POINTS_RING = {
  3,
  (GPoint []) {
    {0, 0},
    {-6, -73}, // 58 = radius + fudge; 6 = 58*tan(6 degrees); 30 degrees per hour;
    {6,  -73},
  }
};
static GPath *s_path_ring_segment;

static GColor s_color_text;
static GColor s_color_indicator;
static GColor s_color_ring_1;
static GColor s_color_ring_2;
static GColor s_color_bg_in;
static GColor s_color_bg_out;

static GFont s_font_date;
static GFont s_font_time;

char buffer_layer_day[] = "aaaaaaaaa";
char buffer_layer_time[] = "00:00";
char buffer_layer_date[] = "00/00/0000";

void layer_bg_in_update_callback(Layer *layer, GContext* ctx) {
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	
	graphics_context_set_fill_color(ctx, s_color_bg_in);
	graphics_fill_circle(ctx, center, 64);
}
void layer_ring_update_callback(Layer *layer, GContext* ctx) {	
	time_t tt = time(NULL);
	struct tm *t = localtime(&tt);
	
	unsigned int angle = 0;
	if (t->tm_min != 0) {
		angle = ( t->tm_min + 1 ) * 6;
	}
	
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	
	graphics_context_set_fill_color(ctx, s_color_ring_1);
	graphics_fill_circle(ctx, center, 73);
	
	graphics_context_set_fill_color(ctx, s_color_ring_2);
	for(; angle < 355; angle += 6) {
		gpath_rotate_to(s_path_ring_segment, (TRIG_MAX_ANGLE / 360) * angle);
		gpath_draw_filled(ctx, s_path_ring_segment);
	}
	
	graphics_context_set_stroke_color(ctx, s_color_bg_out);
	graphics_context_set_stroke_width(ctx, 4);
	graphics_draw_circle(ctx, center, 73);
}

static void main_window_load(Window *window) {
	// Create ring layer
	Layer *window_layer = window_get_root_layer(s_window_main);
	GRect bounds = layer_get_bounds(window_layer);
	
	layer_ring = layer_create(bounds);
	layer_set_update_proc(layer_ring, layer_ring_update_callback);
	layer_add_child(window_layer, layer_ring);
	
	s_path_ring_segment = gpath_create(&PATH_POINTS_RING);
	gpath_move_to(s_path_ring_segment, grect_center_point(&bounds));

	// Create inner background
	layer_bg_in = layer_create(bounds);
	layer_set_update_proc(layer_bg_in, layer_bg_in_update_callback);
	layer_add_child(window_layer, layer_bg_in);
	
	// Create TextLayers
	s_layer_day = text_layer_create(GRect(0, 41, 144, 20));
	text_layer_set_background_color(s_layer_day, GColorClear);
	text_layer_set_font(s_layer_day, s_font_date);
	text_layer_set_text_color(s_layer_day, s_color_text);
	text_layer_set_text(s_layer_day, "");
	text_layer_set_text_alignment(s_layer_day, GTextAlignmentCenter);
	layer_add_child(layer_bg_in, text_layer_get_layer(s_layer_day));
	
	s_layer_time = text_layer_create(GRect(0, 59, 144, 50));
	text_layer_set_background_color(s_layer_time, GColorClear);
	text_layer_set_font(s_layer_time, s_font_time);
	text_layer_set_text_color(s_layer_time, s_color_text);
	text_layer_set_text(s_layer_time, "");
	text_layer_set_text_alignment(s_layer_time, GTextAlignmentCenter);
	layer_add_child(layer_bg_in, text_layer_get_layer(s_layer_time));
	
	s_layer_date = text_layer_create(GRect(0, 107, 144, 20));
	text_layer_set_background_color(s_layer_date, GColorClear);
	text_layer_set_font(s_layer_date, s_font_date);
	text_layer_set_text_color(s_layer_date, s_color_text);
	text_layer_set_text(s_layer_date, "");
	text_layer_set_text_alignment(s_layer_date, GTextAlignmentCenter);
	layer_add_child(layer_bg_in, text_layer_get_layer(s_layer_date));
}
static void main_window_unload(Window *window) {
	// Destroy Layers
	text_layer_destroy(s_layer_date);
	text_layer_destroy(s_layer_time);
	text_layer_destroy(s_layer_day);
	layer_destroy(layer_bg_in);
	layer_destroy(layer_ring);
}


void update_day(struct tm *tick_time) {
	strftime(buffer_layer_day, sizeof("aaaaaaaaa"), "%A", tick_time);
	text_layer_set_text(s_layer_day, buffer_layer_day);
}
void update_time(struct tm *tick_time) {
	// Write the current hours and minutes into the buffer
	if (clock_is_24h_style()) {
		strftime(buffer_layer_time, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		strftime(buffer_layer_time, sizeof("00:00"), "%I:%M", tick_time);
	}
	
	// Display this time on the TextLayer
	text_layer_set_text(s_layer_time, buffer_layer_time);
	
	// Force ring to refresh
	layer_mark_dirty(layer_ring);
}
void update_date(struct tm *tick_time) {
	strftime(buffer_layer_date, sizeof("00/00/0000"), "%d/%m/%Y", tick_time);
	text_layer_set_text(s_layer_date, buffer_layer_date);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if (persist_read_bool(KEY_SHOW_DAY)) {
		update_day(tick_time);
	}
	update_time(tick_time);
	if (persist_read_bool(KEY_SHOW_DATE)) {
		update_date(tick_time);
	}
}

void bt_handler(bool connected) {
	if (!connected) {
		if (persist_read_bool(KEY_VIBE_BT)) {
			vibes_short_pulse();
		}
		
		//show_message(COLOR_FALLBACK(GColorBlueMoon, GColorWhite), 5000, "BT lost");
	}
}

void update_settings_colors() {
	#ifdef PBL_COLOR
		GColor8 color;
		color.argb = persist_read_int(KEY_COLOR_TEXT);
		s_color_text = color;
		color.argb = persist_read_int(KEY_COLOR_INDICATOR);
		s_color_indicator = color;
		color.argb = persist_read_int(KEY_COLOR_RING_1);
		s_color_ring_1 = color;
		color.argb = persist_read_int(KEY_COLOR_RING_2);
		s_color_ring_2 = color;
		color.argb = persist_read_int(KEY_COLOR_BG_IN);
		s_color_bg_in = color;
		color.argb = persist_read_int(KEY_COLOR_BG_OUT);
		s_color_bg_out = color;
	#endif
		
	text_layer_set_text_color(s_layer_day, s_color_text);
	text_layer_set_text_color(s_layer_time, s_color_text);
	text_layer_set_text_color(s_layer_date, s_color_text);
	window_set_background_color(s_window_main, s_color_bg_out);
		
	layer_mark_dirty(layer_bg_in);
	layer_mark_dirty(layer_ring);
}
void update_settings_visuals() {
	if (persist_read_bool(KEY_SHOW_DAY)) {
		text_layer_set_text_color(s_layer_day, s_color_text);
	} else {
		text_layer_set_text_color(s_layer_day, GColorClear);
	}
	if (persist_read_bool(KEY_SHOW_DATE)) {
		text_layer_set_text_color(s_layer_date, s_color_text);
	} else {
		text_layer_set_text_color(s_layer_date, GColorClear);
	}
}
void update_settings() {
	update_settings_colors();
	update_settings_visuals();
}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
	// Get tuple
	Tuple *t = dict_read_first(iterator);
	
	while (t != NULL) {
		switch (t->key) {
			case KEY_COLOR_TEXT:
				persist_write_int(KEY_COLOR_TEXT, t->value->int32);
				break;
			case KEY_COLOR_INDICATOR:
				persist_write_int(KEY_COLOR_INDICATOR, t->value->int32);
				break;
			case KEY_COLOR_RING_1:
				persist_write_int(KEY_COLOR_RING_1, t->value->int32);
				break;
			case KEY_COLOR_RING_2:
				persist_write_int(KEY_COLOR_RING_2, t->value->int32);
				break;
			case KEY_COLOR_BG_IN:
				persist_write_int(KEY_COLOR_BG_IN, t->value->int32);
				break;
			case KEY_COLOR_BG_OUT:
				persist_write_int(KEY_COLOR_BG_OUT, t->value->int32);
				break;
			case KEY_SHOW_DAY:
				if (strcmp(t->value->cstring, "true") == 0) {
					persist_write_bool(KEY_SHOW_DAY, true);
				} else if (strcmp(t->value->cstring, "false") == 0) {
					persist_write_bool(KEY_SHOW_DAY, false);
				}
				break;
			case KEY_SHOW_DATE:
				if (strcmp(t->value->cstring, "true") == 0) {
					persist_write_bool(KEY_SHOW_DATE, true);
				} else if (strcmp(t->value->cstring, "false") == 0) {
					persist_write_bool(KEY_SHOW_DATE, false);
				}
				break;
			case KEY_VIBE_BT:
				if (strcmp(t->value->cstring, "true") == 0) {
					persist_write_bool(KEY_VIBE_BT, true);
				} else if (strcmp(t->value->cstring, "false") == 0) {
					persist_write_bool(KEY_VIBE_BT, false);
				}
				break;
		}
		
		// Get next pair, if any
    	t = dict_read_next(iterator);
	}
	
	update_settings();
}

static void init() {
	// Initialyze settings
	app_message_register_inbox_received(inbox_received_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	// Set default settings if needed
	if (persist_exists(KEY_COLOR_TEXT) == false) {
		persist_write_int(KEY_COLOR_TEXT, -1);
	}
	if (persist_exists(KEY_COLOR_INDICATOR) == false) {
		persist_write_int(KEY_COLOR_INDICATOR, -1);
	}
	if (persist_exists(KEY_COLOR_RING_1) == false) {
		persist_write_int(KEY_COLOR_RING_1, -1);
	}
	if (persist_exists(KEY_COLOR_RING_2) == false) {
		persist_write_int(KEY_COLOR_RING_2, -53);
	}
	if (persist_exists(KEY_COLOR_BG_IN) == false) {
		persist_write_int(KEY_COLOR_BG_IN, -63);
	}
	if (persist_exists(KEY_COLOR_BG_OUT) == false) {
		persist_write_int(KEY_COLOR_BG_OUT, -58);
	}
	if (persist_exists(KEY_SHOW_DAY) == false) {
		persist_write_bool(KEY_SHOW_DAY, true);
	}
	if (persist_exists(KEY_SHOW_DATE) == false) {
		persist_write_bool(KEY_SHOW_DATE, true);
	}
	if (persist_exists(KEY_VIBE_BT) == false) {
		persist_write_bool(KEY_VIBE_BT, true);
	}
		
	// Set custom fonts.
	s_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_REGULAR_16));
	s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_REGULAR_38));
	
	// Create main Window element and assign to pointer
	s_window_main = window_create();
	
	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_window_main, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
		
	// Show the Window on the watch, with animated=true
	window_stack_push(s_window_main, true);
	
	// Initially show the correct settings and time
	update_settings();
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);
    update_time(tick_time);
	
	// Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

	// Register with BluetoothConnectionService
    bluetooth_connection_service_subscribe(bt_handler);
}

static void deinit() {
	// Unregister from TickTimerService
    tick_timer_service_unsubscribe();
	
	// Unregister from BluetoothConnectionService
	bluetooth_connection_service_unsubscribe();
	
	// Destroy Window
	window_destroy(s_window_main);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
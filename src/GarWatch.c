#include "pebble.h"

static Window *window;
static Layer *window_layer;

static uint8_t batteryPercent;

static bool appStarted = false;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static GBitmap *logo_image;
static BitmapLayer *logo_layer;

static GBitmap *logob_image;
static BitmapLayer *logob_layer;

static GBitmap *separator_image;
static BitmapLayer *separator_layer;

static GBitmap *meter_bar_image;
static BitmapLayer *meter_bar_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;

static GBitmap *time_format_image;
static BitmapLayer *time_format_layer;

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

static GBitmap *month_name_image;
static BitmapLayer *month_name_layer;

static Layer *big_time_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

const int MONTH_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MONTH_NAME_JAN,
  RESOURCE_ID_IMAGE_MONTH_NAME_FEB,
  RESOURCE_ID_IMAGE_MONTH_NAME_MAR,
  RESOURCE_ID_IMAGE_MONTH_NAME_APR,
  RESOURCE_ID_IMAGE_MONTH_NAME_MAY,
  RESOURCE_ID_IMAGE_MONTH_NAME_JUN,
  RESOURCE_ID_IMAGE_MONTH_NAME_JUL,
  RESOURCE_ID_IMAGE_MONTH_NAME_AUG,
  RESOURCE_ID_IMAGE_MONTH_NAME_SEP,
  RESOURCE_ID_IMAGE_MONTH_NAME_OCT,
  RESOURCE_ID_IMAGE_MONTH_NAME_NOV,
  RESOURCE_ID_IMAGE_MONTH_NAME_DEC
};

#define TOTAL_DATE_DIGITS 2 
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_BATTERY_PERCENT_DIGITS 3
static GBitmap *battery_percent_image[TOTAL_BATTERY_PERCENT_DIGITS];
static BitmapLayer *battery_percent_layers[TOTAL_BATTERY_PERCENT_DIGITS];

const int TINY_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_TINY_0,
  RESOURCE_ID_IMAGE_TINY_1,
  RESOURCE_ID_IMAGE_TINY_2,
  RESOURCE_ID_IMAGE_TINY_3,
  RESOURCE_ID_IMAGE_TINY_4,
  RESOURCE_ID_IMAGE_TINY_5,
  RESOURCE_ID_IMAGE_TINY_6,
  RESOURCE_ID_IMAGE_TINY_7,
  RESOURCE_ID_IMAGE_TINY_8,
  RESOURCE_ID_IMAGE_TINY_9,
  RESOURCE_ID_IMAGE_TINY_PERCENT
};

void change_battery_icon(bool charging)
{
  gbitmap_destroy(battery_image);
  if(charging)
  {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  }
  else
  {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  }  
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed);

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin)
{
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
	
  if (old_image != NULL)
  {
    gbitmap_destroy(old_image);
    old_image = NULL;
  }
}

static void update_battery(BatteryChargeState charge_state)
{
  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100)
  {
    change_battery_icon(false);
    layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
    for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i)
	 {
      layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), true);
    }  
    return;
  }

  layer_set_hidden(bitmap_layer_get_layer(battery_layer), charge_state.is_charging);
  change_battery_icon(charge_state.is_charging);
    
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i)
  {
    layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), false);
  }  
  set_container_image(&battery_percent_image[0], battery_percent_layers[0], TINY_IMAGE_RESOURCE_IDS[charge_state.charge_percent/10], GPoint(89, 40));
  set_container_image(&battery_percent_image[1], battery_percent_layers[1], TINY_IMAGE_RESOURCE_IDS[charge_state.charge_percent%10], GPoint(95, 40));
  set_container_image(&battery_percent_image[2], battery_percent_layers[2], TINY_IMAGE_RESOURCE_IDS[10], GPoint(101, 41)); 
}

static void toggle_bluetooth_icon(bool connected)
{
  if(appStarted && !connected)
  {
    vibes_long_pulse();
  }
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

void bluetooth_connection_callback(bool connected)
{
  toggle_bluetooth_icon(connected);
}

void battery_layer_update_callback(Layer *me, GContext* ctx)
{        
  //draw the remaining battery percentage
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(2, 2, ((batteryPercent/100.0)*11.0), 6), 0, GCornerNone);
}

unsigned short get_display_hour(unsigned short hour)
{
  if (clock_is_24h_style())
  {
    return hour;
  }
  unsigned short display_hour = hour % 12; //Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_days(struct tm *tick_time)
{
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint(18, 125));
  set_container_image(&month_name_image, month_name_layer, MONTH_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(63, 125));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(105, 125));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(116, 125));
}

static void update_hours(struct tm *tick_time)
{
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(12, 66));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(41, 66));
		
	if (display_hour/10 == 0)
	{
		layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
	}
	else
	{
		layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
	}

  if (!clock_is_24h_style())
  {
    if (tick_time->tm_hour >= 12)
	 {
      set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(50, 40));
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
    } 
    else
	 {
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);
    }
  }
}
static void update_minutes(struct tm *tick_time)
{
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(77, 66));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(106, 66));
}
static void update_seconds(struct tm *tick_time)
{
  layer_set_hidden(bitmap_layer_get_layer(separator_layer), tick_time->tm_sec%2);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
  if (units_changed & DAY_UNIT)
  {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT)
  {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT)
  {
    update_minutes(tick_time);
  }
  if (units_changed & SECOND_UNIT)
  {
    update_seconds(tick_time);
  }  
}

static void init(void)
{
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  memset(&battery_percent_layers, 0, sizeof(battery_percent_layers));
  memset(&battery_percent_image, 0, sizeof(battery_percent_image));

  const int inbound_size = 128;
  const int outbound_size = 128;
  app_message_open(inbound_size, outbound_size);  

  window = window_create();
  if (window == NULL)
  {
      return;
  }
  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
  
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));	
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
	
  logo_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);
  GRect logoframe = (GRect) {
    .origin = { .x = 6, .y = 9 },
    .size = logo_image->bounds.size
  };
  logo_layer = bitmap_layer_create(logoframe);
  bitmap_layer_set_bitmap(logo_layer, logo_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));
	
  logob_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGOB);
  GRect logobframe = (GRect) {
    .origin = { .x = 10, .y = 156 },
    .size = logob_image->bounds.size
  };
  logob_layer = bitmap_layer_create(logobframe);
  bitmap_layer_set_bitmap(logob_layer, logob_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(logob_layer));
  
  big_time_layer = layer_create(layer_get_frame(window_layer));
  layer_add_child(window_layer, big_time_layer);
  
  separator_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SEPARATOR);
  GRect frame = (GRect) {
    .origin = { .x = 70, .y = 75 },
    .size = separator_image->bounds.size
  };
  separator_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(separator_layer, separator_image);
  layer_add_child(big_time_layer, bitmap_layer_get_layer(separator_layer));

  meter_bar_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_METER_BAR);
  GRect frame2 = (GRect) {
    .origin = { .x = 17, .y = 40 },
    .size = meter_bar_image->bounds.size
  };
  meter_bar_layer = bitmap_layer_create(frame2);
  bitmap_layer_set_bitmap(meter_bar_layer, meter_bar_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(meter_bar_layer));  

  bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  GRect frame3 = (GRect) {
    .origin = { .x = 33, .y = 40 },
    .size = bluetooth_image->bounds.size
  };
  bluetooth_layer = bitmap_layer_create(frame3);
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
  
  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  GRect frame4 = (GRect) {
    .origin = { .x = 111, .y = 39 },
    .size = battery_image->bounds.size
  };
  battery_layer = bitmap_layer_create(frame4);
  battery_image_layer = bitmap_layer_create(frame4);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_set_update_proc(bitmap_layer_get_layer(battery_layer), battery_layer_update_callback);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));

  // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
  day_name_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));
  month_name_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(month_name_layer));
  
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i)
  {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(big_time_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i)
  {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i)
  {
    battery_percent_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(battery_percent_layers[i]));
  }
  
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  update_battery(battery_state_service_peek());
	
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);

  appStarted = true;

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(&update_battery);
}

static void deinit(void)
{  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(logo_layer));
  bitmap_layer_destroy(logo_layer);
  gbitmap_destroy(logo_image);
  logo_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(logob_layer));
  bitmap_layer_destroy(logob_layer);
  gbitmap_destroy(logob_image);
  logob_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer));
  bitmap_layer_destroy(separator_layer);
  gbitmap_destroy(separator_image);
  separator_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(meter_bar_layer));
  bitmap_layer_destroy(meter_bar_layer);
  gbitmap_destroy(meter_bar_image);
  meter_bar_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_image);
  bluetooth_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(battery_image);
  battery_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);
  time_format_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  day_name_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(month_name_layer));
  bitmap_layer_destroy(month_name_layer);
  gbitmap_destroy(month_name_image);
  month_name_image = NULL;
  
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++)
  {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    date_digits_images[i] = NULL;
    bitmap_layer_destroy(date_digits_layers[i]);
    date_digits_layers[i] = NULL;
  }
  for (int i = 0; i < TOTAL_TIME_DIGITS; i++)
  {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    time_digits_images[i] = NULL;
    bitmap_layer_destroy(time_digits_layers[i]);
    time_digits_layers[i] = NULL;
  }
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; i++)
  {
    layer_remove_from_parent(bitmap_layer_get_layer(battery_percent_layers[i]));
    gbitmap_destroy(battery_percent_image[i]);
    battery_percent_image[i] = NULL;
    bitmap_layer_destroy(battery_percent_layers[i]); 
    battery_percent_layers[i] = NULL;
  } 
  
  layer_remove_from_parent(big_time_layer);
  big_time_layer = NULL;
  
  appStarted = NULL;
  
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);  
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
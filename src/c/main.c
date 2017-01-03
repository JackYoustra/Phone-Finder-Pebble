#include <pebble.h>
#include <stdint.h>

#define RING_KEY 0
#define SINGLE_RING_VALUE 0
#define TOGGLE_RING_VALUE 1
#define FART_VALUE 2

Window *mainWindow;
TextLayer *instructions[3];
GRect windowBounds;

static void window_load(Window *window){
  const char* messages[] = {
    "Ring once »",
    "Repeat On/Off»",
    "Surprise »"
  };
  windowBounds = layer_get_bounds(window_get_root_layer(mainWindow));
  const int16_t third_height = windowBounds.size.h/3;
  
  // do common processing
  for(uint8_t i = 0; i < 3; i++){
    instructions[i] = text_layer_create(GRect(0, third_height*i, windowBounds.size.w, third_height));
    text_layer_set_text(instructions[i], messages[i]);
    text_layer_set_background_color(instructions[i], GColorClear);
    text_layer_set_text_color(instructions[i], GColorBlack);
    text_layer_set_font(instructions[i], fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(instructions[i], GTextAlignmentRight);
    
    layer_add_child(window_get_root_layer(mainWindow), text_layer_get_layer(instructions[i]));
  }
}

// use this function to free memory
static void window_unload(Window *window){
  //We will safely destroy the Window's elements here!
  for(int i = 0; i < 3; i++){
    text_layer_destroy(instructions[i]);
  }
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void sendMessage(int key, uint8_t value){
  // Prepare dictionary
  DictionaryIterator *iterator;
  app_message_outbox_begin(&iterator);
  // Write data
  dict_write_data(iterator, key, &value, sizeof(value));
  app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  sendMessage(RING_KEY, SINGLE_RING_VALUE);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  sendMessage(RING_KEY, TOGGLE_RING_VALUE);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // unlabeled, make fart sound
  sendMessage(RING_KEY, FART_VALUE);
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void networkInit(){
  // Open AppMessage with sensible buffer sizes
  app_message_open(64, 64);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

void init(){
  //Create app elements here
  // make main window
  mainWindow = window_create();
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    // delegate window functions / hooks
    .load = window_load,
    .unload = window_unload,
  });

  // setup network stuff
  networkInit();
  
  // make app move to foreground
  window_set_click_config_provider(mainWindow, click_config_provider);

  window_stack_push(mainWindow, true);
}

void deinit(){
  //We will safely destroy the Window's elements here!
  window_destroy(mainWindow);
}


int main(){
  init();
  app_event_loop();
  deinit();
}
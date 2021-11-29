enum Screen {
  screen_intro,
  screen_timer_fog_display,
  screen_timer_fog_start,
  screen_timer_oven_display,
  screen_timer_oven_start,
  screen_complete,
  screen_config,
};
//constants
const char blankBuff[21] PROGMEM = "                    ";
const byte fog_address      = 100;
const byte oven_address     = 102;
const byte cycle_address    = 104;
const int  max_fog_counter  = 300;
const int  max_oven_counter = 480;
//pins
const byte setButtonPin     = 8;     // digital input
const byte decButtonPin     = 9;     // digital input
const byte incButtonPin     = 10;    // digital input
const byte backButtonPin    = 13;    // digital input
const byte fogRelay         = 12;
const byte ovenRelay        = 11;
//settings variable
int config_fog_counter      = 0; //save in eeprom // used for user input // copied to actual working variable
int config_oven_counter     = 0; //save in eeprom // used for user input // copied to actual working variable
int cycle_counter           = 0; //save in eeprom // machine cycles counter increase after each complete cycle 

enum Screen{
  screen_intro,
  screen_timer_fog_display,
  screen_timer_fog_start,
  screen_timer_oven_display,
  screen_timer_oven_start,
  screen_complete,
  screen_config,
};
//pins
const byte  setButtonPin      = 8;    // digital input
const byte  decButtonPin      = 9;    // digital input
const byte  incButtonPin      = 10;    // digital input
const byte  backButtonPin     = 13;    // digital input

int fog_counter   = 0; // working variable // max 5 min: 300 sec//display time as min:sec
int oven_counter  = 0; // working variable // max 8 min: 480 sec//display time as min:sec
int cycle_counter = 0; // machine cycles counter increase after each complete cycle //save in eeprom


int config_fog_counter = 0; //save in eeprom // copied to actual working variable
int config_oven_counter = 0; //save in eeprom // copied to actual working variable
const int max_fog_counter = 300;
const int max_oven_counter = 480;

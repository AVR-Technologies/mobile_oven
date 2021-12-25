//constants
const byte fog_address    = 100;
const byte oven_address   = 102;
const byte cycle_address  = 104;
const byte oven_temp_address= 106;

int config_fog_counter = 0; //save in eeprom // copied to actual working variable
int config_oven_counter = 0; //save in eeprom // copied to actual working variable
int config_oven_temp    = 0;      // save in eeprom // used for user input // compared with actual working variable
int cycle_counter = 0; // machine cycles counter increase after each complete cycle //save in eeprom

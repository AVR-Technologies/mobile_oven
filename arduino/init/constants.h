//constants
const byte fog_address    = 100;
const byte oven_address   = 102;
const byte cycle_address  = 104;
int config_fog_counter = 0; //save in eeprom // copied to actual working variable
int config_oven_counter = 0; //save in eeprom // copied to actual working variable
int cycle_counter = 0; // machine cycles counter increase after each complete cycle //save in eeprom

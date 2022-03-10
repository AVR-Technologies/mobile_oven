//constants
const char blankBuff[21] PROGMEM = "                    ";
const float fogMotorCurrentThreshold = 0.5;
const float ovenCurrentThreshold = 0.5;
//settings default constants
const int default_fog_counter      = 300; // 5min : 5 * 60
const int default_oven_counter     = 480; // 8min : 8 * 60
const int default_oven_temp        = 40;  //

//eeprom address
const byte fog_address      = 100;
const byte oven_address     = 102;
const byte cycle_address    = 104;
const byte oven_temp_address = 106;
//i2c address
const byte i2c_address      = 8;

const int  max_fog_counter  = 300;
const int  max_oven_counter = 480;
const int  max_oven_temp    = 40;     // lm35 temperature range is -55cel to 150cel
const int  oven_temp_low_error = 5;  // config temp = 40;
// if (temp > 40) turn off relay else if(temp < (40-oven_temp_low_error)) turn on relay
// if config temp <= 10 then minimum temp threshold is 0;

//pins
const byte setButtonPin     = 8;      // digital input
const byte decButtonPin     = 9;      // digital input
const byte incButtonPin     = 10;     // digital input
const byte backButtonPin    = 13;     // digital input
const byte fogRelayPin      = 12;
const byte ovenRelayPin     = 11;
const byte temperaturePin   = A2;
const byte fogCurrentPin    = A3;     // fog current
const byte ovenCurrentPin   = A1;     // over current
//settings variable
int config_fog_counter      = 0;      // save in eeprom // used for user input // copied to actual working variable
int config_oven_counter     = 0;      // save in eeprom // used for user input // copied to actual working variable
int config_oven_temp        = 0;      // save in eeprom // used for user input // compared with actual working variable
int cycle_counter           = 0;      // save in eeprom // machine cycles counter increase after each complete cycle

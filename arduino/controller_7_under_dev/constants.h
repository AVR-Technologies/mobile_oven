//constants
const char blankBuff[21] PROGMEM = "                    ";
const float fogMotorCurrentThreshold = 0.5;
const float ovenCurrentThreshold = 0.5;
const int  ovenTempThreshold = 5;  // config temp = 40;
//i2c address
const byte i2c_address      = 8;

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

struct Config {
  unsigned int fogCounter;
  unsigned int ovenCounter;
  unsigned int ovenTemp;
  unsigned int cycleCounter;// 0 not used
};
Config configMax = {300, 480, 40, 0};
Config configAddress = {100, 102, 106, 104}; //eeprom address
Config configSettings = {0, 0, 0, 0}; // settings
Config configDefault = {300, 480, 40, 0};

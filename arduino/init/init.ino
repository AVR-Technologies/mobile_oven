#include "EEPROM.h"
#include "constants.h"

void setup() {
  Serial.begin(115200);
  save();
  Serial.println("init complete");
}

void loop() {}

//eeprom
void save(){
  EEPROM.put(fog_address, config_fog_counter);
  EEPROM.put(oven_address, config_oven_counter);
  EEPROM.put(cycle_address, cycle_counter);
}

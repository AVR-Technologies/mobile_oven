#include "EEPROM.h"
#include "LiquidCrystal.h"
#include "common.h"
#include "constants.h"
#include "Wire.h"

Screen screen = screen_home;
EditIndex editIndex = edit_cycle;
I2cCommand command = i2c_null;
Button  setButton(setButtonPin),
        decButton(decButtonPin),
        incButton(incButtonPin),
        backButton(backButtonPin);
Relay   fogRelay(fogRelayPin),
        ovenRelay(ovenRelayPin);

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//lcd
char row1[21];
char row2[21];
char row3[21];
char row4[21];
//counter
unsigned int fog_counter   = 0; // working variable // max 5 min: 300 sec//display time as min:sec
unsigned int oven_counter  = 0; // working variable // max 8 min: 480 sec//display time as min:sec
unsigned int oven_temp     = 0; // working variable // max   cel:
//log delay timer
unsigned long last_logged_on = 0;

void setup() {
  Wire.begin(i2c_address);
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(115200);
  Serial.println(">>> machine: booting");
  lcd.begin(20, 4);
  pinMode(fogRelayPin, OUTPUT);
  pinMode(ovenRelayPin, OUTPUT);
  read();
}

void loop() {
  read_temperature();
  screen_handle();
  screen_print();
}
void screen_handle() {
  switch (screen) {
    case screen_home : {
        if (command == i2c_set) {
          command = i2c_null;
          screen = screen_timer_select;
        } else if (command == i2c_back) {
          command = i2c_null;
          screen = screen_config;
        } else if (setButton.onRelease()) {
          screen = screen_timer_select;
        } else if (backButton.onRelease()) {
          screen = screen_config;
        }
        break;
      }
    case screen_timer_select : {
        fog_counter = config_fog_counter;
        oven_counter = config_oven_counter;
        if (command == i2c_set) {
          command = i2c_null;
          screen = screen_home;
        } else if (command == i2c_dec) {
          command = i2c_null;
          screen = screen_timer_fog_start;
          resetMsDelay();
          fogRelay.on();
        } else if (command == i2c_inc) {
          command = i2c_null;
          screen = screen_timer_oven_start;
          resetMsDelay();
          ovenRelay.on();
        } else if (command == i2c_back) {
          command = i2c_null;
          screen = screen_home;
        } else if (setButton.onRelease()) {
          screen = screen_home;
        } else if (decButton.onRelease()) {
          screen = screen_timer_fog_start;
          resetMsDelay();
          fogRelay.on();
        } else if (incButton.onRelease()) {
          screen = screen_timer_oven_start;
          resetMsDelay();
          ovenRelay.on();
        } else if (backButton.onRelease()) {
          screen = screen_home;
        }
        break;
      }
    case screen_timer_fog_start : {
        if (onMsDelayPass(1000)) {
          if (fog_counter > 0) {
            --fog_counter;
            float _current = read_current(fogCurrentPin);
            if (_current < fogMotorCurrentThreshold) {
              screen = screen_error;
              fogRelay.off();
            }
          } else {
            screen = screen_timer_fog_completed;
            fogRelay.off();
          }
        }

        if (command == i2c_back) {
          command = i2c_null;
          fogRelay.off();
          screen = screen_home;
        } else if (backButton.onRelease()) {
          fogRelay.off();
          screen = screen_home;
        }
        break;
      }
    case screen_timer_oven_start : {
        maintain_temperature();

        if (onMsDelayPass(1000)) {
          if (oven_counter > 0) {
            --oven_counter;
            float _current = read_current(ovenCurrentPin);
            Serial.print("ovenRelay.state:");
            Serial.println(ovenRelay.value);
            if ( ovenRelay.value &&  _current < ovenCurrentThreshold) {
              screen = screen_error;
              ovenRelay.off();
            }
          } else {
            screen = screen_timer_oven_completed;
            ovenRelay.off();
            increament_cycle();
          }
        }

        if (command == i2c_back) {
          command = i2c_null;
          ovenRelay.off();
          screen = screen_home;
        } else if (backButton.onRelease()) {
          ovenRelay.off();
          screen = screen_home;
        }
        break;
      }
    case screen_error : {
        if (command == i2c_set) {
          command = i2c_null;
          screen = screen_home;
        } else if (setButton.onRelease()) {
          screen = screen_home;
        }
        break;
      }
    case screen_config : {
        config();
        break;
      }
    case screen_reset_to_default : {
        if (command == i2c_set) {
          command = i2c_null;
          reset_to_default();
          screen = screen_home;
        } else if (command == i2c_back) {
          command = i2c_null;
          screen = screen_home;
        } else if (setButton.onRelease()) {
          reset_to_default();
          screen = screen_home;
        } else if (backButton.onRelease()) {
          screen = screen_home;
        }
        break;
      }
    default: break;
  }
}
void config() {
  switch (editIndex) {
    case edit_cycle: {
        if (command == i2c_set) {
          command = i2c_null;
          editIndex = edit_fog_timer;
        } else if (command == i2c_back) {
          command = i2c_null;
          back_from_config();
        } else if (command == i2c_inc) {
          command = i2c_null;
          reset_cycle();
        } else if (command == i2c_dec) {
          command = i2c_null;
          screen = screen_reset_to_default;
        } else if (setButton.onRelease()) {
          editIndex = edit_fog_timer;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();                         // 200ms delay
          int counter = 0;
          while (incButton.onPress() && counter <= 18) { // already 200ms delay: so count to 18 instead of 20
            ++counter;
            delay(90); //10ms delay in onIncButton(), total: 100ms
          }
          if (counter >= 18) {
            reset_cycle();
          }
        } else if (decButton.onPress()) {
          waitForButtonRelease();                         // 200ms delay
          int counter = 0;
          while (decButton.onPress() && counter <= 18) { // already 200ms delay: so count to 18 instead of 20
            ++counter;
            delay(90); //10ms delay in onIncButton(), total: 100ms
          }
          if (counter >= 18) {
            screen = screen_reset_to_default;
          }
        }
        break;
      }
    case edit_fog_timer: {
        if (command == i2c_set) {
          command = i2c_null;
          editIndex = edit_oven_timer;
        } else if (command == i2c_back) {
          command = i2c_null;
          back_from_config();
        } else if (command == i2c_inc) {
          command = i2c_null;
          config_fog_counter = config_fog_counter < max_fog_counter  ? config_fog_counter + 1 : 0;
        } else if (command == i2c_dec) {
          command = i2c_null;
          config_fog_counter = config_fog_counter > 0 ? config_fog_counter - 1 : max_fog_counter;
        } else if (setButton.onRelease()) {
          editIndex = edit_oven_timer;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();
          do {
            config_fog_counter = config_fog_counter < max_fog_counter  ? config_fog_counter + 1 : 0;
            screen_print();
          } while (incButton.onPress());
        } else if (decButton.onPress()) {
          waitForButtonRelease();
          do {
            config_fog_counter = config_fog_counter > 0 ? config_fog_counter - 1 : max_fog_counter;
            screen_print();
          } while (decButton.onPress());
        }
        break;
      }
    case edit_oven_timer: {
        if (command == i2c_set) {
          command = i2c_null;
          editIndex = edit_oven_temp;
        } else if (command == i2c_back) {
          command = i2c_null;
          back_from_config();
        } else if (command == i2c_inc) {
          command = i2c_null;
          config_oven_counter = config_oven_counter < max_oven_counter ? config_oven_counter + 1 : 0;
        } else if (command == i2c_dec) {
          command = i2c_null;
          config_oven_counter = config_oven_counter > 0 ? config_oven_counter - 1 : max_oven_counter;
        } else if (setButton.onRelease()) {
          editIndex = edit_oven_temp;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();
          do {
            config_oven_counter = config_oven_counter < max_oven_counter ? config_oven_counter + 1 : 0;
            screen_print();
          } while (incButton.onPress());
        } else if (decButton.onPress()) {
          waitForButtonRelease();
          do {
            config_oven_counter = config_oven_counter > 0 ? config_oven_counter - 1 : max_oven_counter;
            screen_print();
          } while (decButton.onPress());
        }
        break;
      }
    case edit_oven_temp: {
        if (command == i2c_set) {
          command = i2c_null;
          editIndex = edit_complete;
        } else if (command == i2c_back) {
          command = i2c_null;
          back_from_config();
        } else if (command == i2c_inc) {
          command = i2c_null;
          config_oven_temp = config_oven_temp < max_oven_temp ? config_oven_temp + 1 : 0;
        } else if (command == i2c_dec) {
          command = i2c_null;
          config_oven_temp = config_oven_temp > 0 ? config_oven_temp - 1 : max_oven_temp;
        } if (setButton.onRelease()) {
          editIndex = edit_complete;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();
          do {
            config_oven_temp = config_oven_temp < max_oven_temp ? config_oven_temp + 1 : 0;
            screen_print();
          } while (incButton.onPress());
        } else if (decButton.onPress()) {
          waitForButtonRelease();
          do {
            config_oven_temp = config_oven_temp > 0 ? config_oven_temp - 1 : max_oven_temp;
            screen_print();
          } while (decButton.onPress());
        }
        break;
      }
    default: {
        if (command == i2c_set) {
          command = i2c_null;
          save();
          back_from_config();
        } else if (command == i2c_back) {
          command = i2c_null;
          back_from_config();
        } else if (setButton.onRelease()) {
          save();
          back_from_config();
        } else if (backButton.onRelease()) {
          back_from_config();
        }
      }
  }
}
void back_from_config() {
  read();
  editIndex = edit_cycle;
  screen = screen_home;
}

void screen_print() {
  switch (screen) {
    case screen_home: {
        sprintf_P(row1, PSTR(" Unique Technology  "));
        sprintf_P(row2, PSTR("       Sangli       "));
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Start         Config"));
        delay(100);
        break;
      }
    case screen_timer_select: {
        sprintf_P(row1, PSTR("    Select timer    "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home Fog  Oven Abort"));
        delay(100);
        break;
      }
    case screen_timer_fog_start:  {
        sprintf_P(row1, PSTR("     Fog timer      "));
        sprintf_P(row2, PSTR("Timer: %.2d:%.2d        "), (int) fog_counter / 60, (int) fog_counter % 60);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("               Abort"));
        delay(100);
        break;
      }
    case screen_timer_oven_start: {
        sprintf_P(row1, PSTR("     Oven timer     "));
        sprintf_P(row2, PSTR("Timer: %.2d:%.2d        "), (int) oven_counter / 60, (int) oven_counter % 60);
        sprintf_P(row3, PSTR("Temp.: %5d        "), oven_temp);
        sprintf_P(row4, PSTR("               Abort"));
        delay(100);
        break;
      }
    case screen_timer_fog_completed: {
        sprintf_P(row1, PSTR("    Fog complete    "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home                "));
        delay(100);
        break;
      }
    case screen_timer_oven_completed: {
        sprintf_P(row1, PSTR("    Oven complete   "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home                "));
        delay(100);
        break;
      }
    case screen_config: {
        sprintf_P(row1, PSTR("%.6s: %.4d Reset:+"),
                  editIndex == 0 ? ">Cycle" : "Cycle ",
                  cycle_counter);
        sprintf_P(row2, PSTR("%.10s   : %.2d:%.2d"),
                  editIndex == edit_fog_timer ? ">Fog Timer" : "Fog Timer ",
                  (int) config_fog_counter / 60,
                  (int) config_fog_counter % 60);
        sprintf_P(row3, PSTR("%.11s  : %.2d:%.2d"),
                  editIndex == edit_oven_timer ? ">Oven Timer" : "Oven Timer ",
                  (int) config_oven_counter / 60,
                  (int) config_oven_counter % 60);
        sprintf_P(row4, PSTR("%.10s   : %.2d   "),
                  editIndex == edit_oven_temp ? ">Oven Temp" : "Oven Temp ",
                  config_oven_temp);
        //        sprintf_P(row1,
        //                  editIndex == 0 ? PSTR(">Cycle: %.4d Reset:+") : PSTR("Cycle : %.4d Reset:+"),
        //                  cycle_counter);
        //
        //        sprintf_P(row2,
        //                  editIndex == edit_fog_timer  ? PSTR(">Fog Timer   : %.2d:%.2d") : PSTR("Fog Timer    : %.2d:%.2d"),
        //                  (int) config_fog_counter / 60, (int) config_fog_counter % 60);
        //
        //        sprintf_P(row3,
        //                  editIndex == edit_oven_timer ? PSTR(">Oven Timer  : %.2d:%.2d") : PSTR("Oven Timer   : %.2d:%.2d"),
        //                  (int) config_oven_counter / 60, (int) config_oven_counter % 60);
        //
        //        sprintf_P(row4,
        //                  editIndex == edit_oven_temp  ? PSTR(">Oven Temp   : %.2d   ")   : PSTR("Oven Temp    : %.2d   "),
        //                  config_oven_temp);
        delay(100);
        break;
      }
    case screen_reset_to_default : {
        sprintf_P(row1, PSTR(" Reset to default?  "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR(" OK           Cancel"));
        delay(100);
        break;
      }
    case screen_error : {
        sprintf_P(row1, PSTR("Error: No Load Found"));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home                "));
        delay(100);
        break;
      }
    default : {
        break;
      }
  }
  lcd.setCursor(0, 0);
  lcd.print(row1);
  lcd.setCursor(0, 1);
  lcd.print(row2);
  lcd.setCursor(0, 2);
  lcd.print(row3);
  lcd.setCursor(0, 3);
  lcd.print(row4);

  if (millis() > last_logged_on + 1000) {
    Serial.println(F("--------------------"));
    Serial.println(row1);
    Serial.println(row2);
    Serial.println(row3);
    Serial.println(row4);
    Serial.println(F("--------------------"));
    last_logged_on = millis();
  }
}

//lm35 temperature measurement
void read_temperature() {
  float value = analogRead(temperaturePin);
  value = value * 500 / 1024; // equavalent voltage in v
  oven_temp = (int) value;
}
void maintain_temperature() {
  int oven_start_temp = config_oven_temp - oven_temp_low_error;
  oven_start_temp = oven_start_temp >= 0 ? oven_start_temp : 0;

  if ( oven_temp > config_oven_temp) ovenRelay.off();
  else if (oven_temp < oven_start_temp) ovenRelay.on();
}

//ct current measurement
float read_current(byte _pin) {
  float nVPP = getVPP(_pin);
  nVPP = (nVPP / 2) * 0.707 * 2500.0;//turn ratio : 1 : 2500
  float nCurrThruResistorPP = (nVPP) / 110.0;//secondary resistance : 110
  float nCurrThruResistorRMS = (nCurrThruResistorPP); //-8.06
  float nCurrentThruWire = nCurrThruResistorRMS * 1000;

  if (nCurrThruResistorRMS < 1.0) {
    nCurrThruResistorRMS = 0;
  }
  Serial.print(F(">>> ct: current \t"));
  Serial.println(nCurrThruResistorPP);
  return (nCurrThruResistorPP);
}
float getVPP(byte _pin) {              //for ac current measurement
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  long int avgVal = 0;
  uint32_t start_time = millis();
  int i = 0;
  while ((millis() - start_time) < 100) {
    readValue = analogRead(_pin);
    avgVal = avgVal + readValue;
    if (readValue > maxValue) maxValue = readValue;
    i++;
  }
  maxValue = (maxValue - (avgVal / i));
  return maxValue * 5.0 / 1024.0;
}

//eeprom and config
void save() {
  Serial.println(F(">>> eeprom: saving config"));
  EEPROM.put(fog_address, config_fog_counter);
  EEPROM.put(oven_address, config_oven_counter);
  EEPROM.put(cycle_address, cycle_counter);
  EEPROM.put(oven_temp_address, config_oven_temp);
  Serial.println(F(">>> eeprom: saved config"));
}
void read() {
  Serial.println(F(">>> eeprom: reading config"));
  EEPROM.get(fog_address, config_fog_counter);
  EEPROM.get(oven_address, config_oven_counter);
  EEPROM.get(cycle_address, cycle_counter);
  EEPROM.get(oven_temp_address, config_oven_temp);
  Serial.println(F(">>> eeprom: read config"));
}
void reset_to_default() {
  Serial.println(F(">>> eeprom: reseting to default"));
  config_fog_counter = default_fog_counter;
  config_oven_counter = default_oven_counter;
  config_oven_temp = default_oven_temp;
  save();
  Serial.println(F(">>> eeprom: reset to default"));
}

//cycle counter
void reset_cycle() {
  cycle_counter = 0;
  save();// save reset counter to eeprom
}
void increament_cycle() {
  cycle_counter = cycle_counter < 10000 ? cycle_counter + 1 : 0;
  save();// save increased counter to eeprom
}

//i2c
void receiveEvent(int howMany) {
  byte buff[howMany];
  Wire.readBytes(buff, howMany);
  if (howMany == 1) {
    command = buff[0] == (int)i2c_inc ? i2c_inc
              : buff[0] == (int)i2c_dec ? i2c_dec
              : buff[0] == (int)i2c_set ? i2c_set
              : buff[0] == (int)i2c_back ? i2c_back
              : i2c_null;
  }
}

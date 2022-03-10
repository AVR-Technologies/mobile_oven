#include "EEPROM.h"
//#include "LiquidCrystal.h"
#include "LiquidCrystal_I2C.h"
#include "common.h"
#include "constants.h"
#include "Wire.h"

Screen screen = screen_timer_select;
EditIndex editIndex = edit_cycle;
I2cCommand command = i2c_null;
Button  setButton(setButtonPin),
        decButton(decButtonPin),
        incButton(incButtonPin),
        backButton(backButtonPin);
Relay   fogRelay(fogRelayPin),
        ovenRelay(ovenRelayPin);

//LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
LiquidCrystal_I2C lcd(0x27, 20, 4);

//lcd
char row1[21];
char row2[21];
char row3[21];
char row4[21];
//counter
Config configCurrent = {0, 0, 0, 0};
//log delay timer
unsigned long last_logged_on = 0;

void setup() {
  Wire.begin(i2c_address);
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(115200);
  Serial.println(">>> machine: booting");
  //  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();
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
    case screen_timer_select : {
        configCurrent.fogCounter  = configSettings.fogCounter;
        configCurrent.ovenCounter = configSettings.fogCounter;
        if (command == i2c_dec) {
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
          screen = screen_config;
        } else if (decButton.onRelease()) {
          screen = screen_timer_fog_start;
          resetMsDelay();
          fogRelay.on();
        } else if (incButton.onRelease()) {
          screen = screen_timer_oven_start;
          resetMsDelay();
          ovenRelay.on();
        } else if (backButton.onRelease()) {
          screen = screen_config;
        }
        break;
      }
    case screen_timer_fog_start : {
        if (onMsDelayPass(1000)) {
          if (configCurrent.fogCounter > 0) {
            --configCurrent.fogCounter;
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
          screen = screen_timer_select;
        } else if (backButton.onRelease()) {
          fogRelay.off();
          screen = screen_timer_select;
        }
        break;
      }
    case screen_timer_oven_start : {
        maintain_temperature();

        if (onMsDelayPass(1000)) {
          if (configCurrent.ovenCounter > 0) {
            --configCurrent.ovenCounter;
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
          screen = screen_timer_select;
        } else if (backButton.onRelease()) {
          ovenRelay.off();
          screen = screen_timer_select;
        }
        break;
      }
    case screen_timer_fog_completed:
    case screen_timer_oven_completed: {
        if (command == i2c_set) {
          command = i2c_null;
          screen = screen_timer_select;
        } else if (setButton.onRelease()) {
          screen = screen_timer_select;
        }
        break;
      }
    case screen_error : {
        if (command == i2c_set) {
          command = i2c_null;
          screen = screen_timer_select;
        } else if (setButton.onRelease()) {
          screen = screen_timer_select;
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
          screen = screen_timer_select;
        } else if (command == i2c_back) {
          command = i2c_null;
          screen = screen_timer_select;
        } else if (setButton.onRelease()) {
          reset_to_default();
          screen = screen_timer_select;
        } else if (backButton.onRelease()) {
          screen = screen_timer_select;
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
          configSettings.fogCounter = configSettings.fogCounter < configMax.fogCounter ? configSettings.fogCounter + 1 : 0;
        } else if (command == i2c_dec) {
          command = i2c_null;
          configSettings.fogCounter = configSettings.fogCounter > 0 ? configSettings.fogCounter - 1 : configMax.fogCounter;
        } else if (setButton.onRelease()) {
          editIndex = edit_oven_timer;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();
          do {
            configSettings.fogCounter = configSettings.fogCounter < configMax.fogCounter ? configSettings.fogCounter + 1 : 0;
            screen_print();
          } while (incButton.onPress());
        } else if (decButton.onPress()) {
          waitForButtonRelease();
          do {
            configSettings.fogCounter = configSettings.fogCounter > 0 ? configSettings.fogCounter - 1 : configMax.fogCounter;
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
          configSettings.ovenCounter = configSettings.ovenCounter < configMax.ovenCounter ? configSettings.ovenCounter + 1 : 0;
        } else if (command == i2c_dec) {
          command = i2c_null;
          configSettings.ovenCounter = configSettings.ovenCounter > 0 ? configSettings.ovenCounter - 1 : configMax.ovenCounter;
        } else if (setButton.onRelease()) {
          editIndex = edit_oven_temp;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();
          do {
            configSettings.ovenCounter = configSettings.ovenCounter < configMax.ovenCounter ? configSettings.ovenCounter + 1 : 0;
            screen_print();
          } while (incButton.onPress());
        } else if (decButton.onPress()) {
          waitForButtonRelease();
          do {
            configSettings.ovenCounter = configSettings.ovenCounter > 0 ? configSettings.ovenCounter - 1 : configMax.ovenCounter;
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
          configSettings.ovenTemp = configSettings.ovenTemp < configMax.ovenTemp ? configSettings.ovenTemp + 1 : 0;
        } else if (command == i2c_dec) {
          command = i2c_null;
          configSettings.ovenTemp = configSettings.ovenTemp > 0 ? configSettings.ovenTemp - 1 : configMax.ovenTemp;
        } if (setButton.onRelease()) {
          editIndex = edit_complete;
        } else if (backButton.onRelease()) {
          back_from_config();
        } else if (incButton.onPress()) {
          waitForButtonRelease();
          do {
            configSettings.ovenTemp = configSettings.ovenTemp < configMax.ovenTemp ? configSettings.ovenTemp + 1 : 0;
            screen_print();
          } while (incButton.onPress());
        } else if (decButton.onPress()) {
          waitForButtonRelease();
          do {
            configSettings.ovenTemp = configSettings.ovenTemp > 0 ? configSettings.ovenTemp - 1 : configMax.ovenTemp;
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
  screen = screen_timer_select;
}

void screen_print() {
  switch (screen) {
    case screen_timer_select: {
        sprintf_P(row1, PSTR(" Unique Technology  "));
        sprintf_P(row2, PSTR("       Sangli       "));
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR(" Ok  Fog Oven Config"));
        break;
      }
    case screen_timer_fog_start:  {
        sprintf_P(row1, PSTR("     Fog timer      "));
        sprintf_P(row2, PSTR("Timer: %.2d:%.2d        "), (int) configCurrent.fogCounter / 60, (int) configCurrent.fogCounter % 60);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("               Abort"));
        break;
      }
    case screen_timer_oven_start: {
        sprintf_P(row1, PSTR("     Oven timer     "));
        sprintf_P(row2, PSTR("Timer: %.2d:%.2d        "), (int) configCurrent.ovenCounter  / 60, (int) configCurrent.ovenCounter % 60);
        sprintf_P(row3, PSTR("Temp.: %5d        "), configCurrent.ovenTemp);
        sprintf_P(row4, PSTR("               Abort"));
        break;
      }
    case screen_timer_fog_completed: {
        sprintf_P(row1, PSTR("    Fog complete    "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home                "));
        break;
      }
    case screen_timer_oven_completed: {
        sprintf_P(row1, PSTR("    Oven complete   "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home                "));
        break;
      }
    case screen_config: {
        sprintf_P(row1, PSTR("%.6s: %.4d Reset:+"),
                  editIndex == 0 ? ">Cycle" : "Cycle ",
                  configSettings.cycleCounter);
        sprintf_P(row2, PSTR("%.10s   : %.2d:%.2d"),
                  editIndex == edit_fog_timer ? ">Fog Timer" : "Fog Timer ",
                  (int) configSettings.fogCounter / 60,
                  (int) configSettings.fogCounter % 60);
        sprintf_P(row3, PSTR("%.11s  : %.2d:%.2d"),
                  editIndex == edit_oven_timer ? ">Oven Timer" : "Oven Timer ",
                  (int) configSettings.ovenCounter / 60,
                  (int) configSettings.ovenCounter % 60);
        sprintf_P(row4, PSTR("%.10s   : %.2d   "),
                  editIndex == edit_oven_temp ? ">Oven Temp" : "Oven Temp ",
                  configSettings.ovenTemp);
        break;
      }
    case screen_reset_to_default : {
        sprintf_P(row1, PSTR(" Reset to default?  "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR(" OK           Cancel"));
        break;
      }
    case screen_error : {
        sprintf_P(row1, PSTR("Error: No Load Found"));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Home                "));
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
  configCurrent.ovenTemp = (int) value;
}
void maintain_temperature() {
  int oven_start_temp = configSettings.ovenTemp - ovenTempThreshold;
  oven_start_temp = oven_start_temp >= 0 ? oven_start_temp : 0;

  if ( configCurrent.ovenTemp > configSettings.ovenTemp) ovenRelay.off();
  else if (configCurrent.ovenTemp < oven_start_temp) ovenRelay.on();
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
  EEPROM.put(configAddress.fogCounter, configSettings.fogCounter);
  EEPROM.put(configAddress.ovenCounter, configSettings.ovenCounter);
  EEPROM.put(configAddress.cycleCounter, configSettings.cycleCounter);
  EEPROM.put(configAddress.ovenTemp, configSettings.ovenTemp);
  Serial.println(F(">>> eeprom: saved config"));
}
void read() {
  Serial.println(F(">>> eeprom: reading config"));
  EEPROM.get(configAddress.fogCounter, configSettings.fogCounter);
  EEPROM.get(configAddress.ovenCounter, configSettings.ovenCounter);
  EEPROM.get(configAddress.cycleCounter, configSettings.cycleCounter);
  EEPROM.get(configAddress.ovenTemp, configSettings.ovenTemp);
  Serial.println(F(">>> eeprom: read config"));
}
void reset_to_default() {
  Serial.println(F(">>> eeprom: reseting to default"));
  configSettings.fogCounter = configDefault.fogCounter;
  configSettings.ovenCounter = configDefault.ovenCounter;
  configSettings.ovenTemp = configDefault.ovenTemp;
  save();
  Serial.println(F(">>> eeprom: reset to default"));
}

//cycle counter
void reset_cycle() {
  configCurrent.cycleCounter = 0;
  save();// save reset counter to eeprom
}
void increament_cycle() {
  configCurrent.cycleCounter = configCurrent.cycleCounter < 10000 ? configCurrent.cycleCounter + 1 : 0;
  save();// save increased counter to eeprom
}

//i2c
void receiveEvent(int howMany) {
  byte buff[howMany];
  Wire.readBytes(buff, howMany);
  if (howMany == 1) {
    command = buff[0] == (int)i2c_inc ? i2c_inc :
              buff[0] == (int)i2c_dec ? i2c_dec :
              buff[0] == (int)i2c_set ? i2c_set :
              buff[0] == (int)i2c_back ? i2c_back :
              i2c_null;
  }
}

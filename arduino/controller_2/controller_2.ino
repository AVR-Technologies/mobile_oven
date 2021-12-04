#include "EEPROM.h"
#include "LiquidCrystal.h"
#include "constants.h"
Screen screen = screen_intro;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//working variable
//lcd
char row1[21];
char row2[21];
char row3[21];
char row4[21];
//config screen
byte editIndex = 0;
long start_time = 0;
//counter
int fog_counter   = 0; // working variable // max 5 min: 300 sec//display time as min:sec
int oven_counter  = 0; // working variable // max 8 min: 480 sec//display time as min:sec
int oven_temp     = 0; // working variable // max   cel:
//log delay timer
long last_logged_on = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("machine: booting");
  lcd.begin(20, 4);
  pinMode(temperaturePin, INPUT);
  pinMode(fogRelayPin, OUTPUT);
  pinMode(ovenRelayPin, OUTPUT);
  read();
}

void loop() {
  read_temperature();
  handle();
  lcd_print();
}

void handle() {
  switch (screen) {
    case screen_intro : {
        if (onSetButton()) screen = screen_timer_fog_display;
        else if (onBackButton()) screen = screen_config;
        break;
      }
    case screen_timer_fog_display : {
        fog_counter = config_fog_counter;
        if (onSetButton()) {
          screen = screen_timer_fog_start;
          start_time = millis();
          fogRelayOn();
        }
        else if (onBackButton()) screen = screen_intro;
        break;
      }
    case screen_timer_oven_display : {
        oven_counter = config_oven_counter;
        if (onSetButton())  {
          screen = screen_timer_oven_start;
          start_time = millis();
          ovenRelayOn();
        }
        else if (onBackButton()) screen = screen_intro;
        break;
      }
    case screen_timer_fog_start : {
        if (millis() > start_time + 1000) {
          start_time = millis();
          if (fog_counter > 0) --fog_counter;
          else {
            screen = screen_timer_oven_display;
            fogRelayOff();
          }
        }
        if (onBackButton()) {
          fogRelayOff();
          screen = screen_intro;
        }
        break;
      }
    case screen_timer_oven_start : {
        if (millis() > start_time + 1000) {
          start_time = millis();
          if (oven_counter > 0) {
            --oven_counter;
            //maintain temperature
            if( oven_temp > config_oven_temp){
              ovenRelayOff();
            }else if(oven_temp < (config_oven_temp - oven_temp_low_error)){
              ovenRelayOn();
            }
            ///////////////////////
          }
          else {
            screen = screen_complete;
            ovenRelayOff();
            ++cycle_counter;
            save();// save increased counter to eeprom
          }
        }

        if (onBackButton()) {
          ovenRelayOff();
          screen = screen_intro;
        }
        break;
      }
    case screen_complete : {
        if (onSetButton()) screen = screen_intro;
        break;
      }
    case screen_config : {
        if (onSetButton()) {
          if (editIndex < 3) ++editIndex;
          else {
            screen = screen_intro;
            save();
            editIndex = 0;
          }
        }
        else if (onDecButton()) {
          delay(200);
          do {
            if (editIndex == 0)
              config_fog_counter = config_fog_counter > 0 ? config_fog_counter - 1 : max_fog_counter;
            else if (editIndex == 1)
              config_oven_counter = config_oven_counter > 0 ? config_oven_counter - 1 : max_oven_counter;
            else if (editIndex == 2)
              config_oven_temp = config_oven_temp > 0 ? config_oven_temp - 1 : max_oven_temp;
            delay(20);
            lcd_print();
          }
          while (digitalRead(decButtonPin));
        }
        else if (onIncButton()) {
          delay(200);
          do {
            if (editIndex == 0)
              config_fog_counter = config_fog_counter < max_fog_counter  ? config_fog_counter + 1 : 0;
            else if (editIndex == 1)
              config_oven_counter = config_oven_counter < max_oven_counter ? config_oven_counter + 1 : 0;
            else if (editIndex == 2)
              config_oven_temp = config_oven_temp < max_oven_temp ? config_oven_temp + 1 : 0;
            delay(20);
            lcd_print();
          }
          while (digitalRead(incButtonPin));
        }
        else if (onBackButton()) {
          screen = screen_intro;
          editIndex = 0;
        }
        break;
      }
    default: break;
  }
}
void lcd_print() {
  switch (screen) {
    case screen_intro: {
        sprintf_P(row1, PSTR(" Unique Technology  "));
        sprintf_P(row2, PSTR("       Sangli       "));
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Start         Config"));
        break;
      }
    case screen_timer_fog_display:
    case screen_timer_fog_start:  {
        sprintf_P(row1, PSTR("     Fog timer      "));
        sprintf_P(row2, PSTR("Timer: %.2d:%.2d        "), (int) fog_counter / 60, (int) fog_counter % 60);
        sprintf_P(row3, PSTR("Temp.: %5d        "), oven_temp);
        sprintf_P(row4, screen == screen_timer_fog_display ? PSTR("Start          Abort") : PSTR("               Abort"));
        break;
      }
    case screen_timer_oven_display:
    case screen_timer_oven_start: {
        sprintf_P(row1, PSTR("     Oven timer     "));
        sprintf_P(row2, PSTR("Timer: %.2d:%.2d        "), (int) oven_counter / 60, (int) oven_counter % 60);
        sprintf_P(row3, PSTR("Temp.: %5d        "), oven_temp);
        sprintf_P(row4, screen == screen_timer_oven_display ? PSTR("Start          Abort") : PSTR("               Abort"));
        break;
      }
    case screen_complete: {
        sprintf_P(row1, PSTR("   Cycle complete   "));
        sprintf_P(row2, blankBuff);
        sprintf_P(row3, blankBuff);
        sprintf_P(row4, PSTR("Restart             "));
        break;
      }
    case screen_config: {
        sprintf_P(row1, PSTR(" Configuration Mode "));
        sprintf_P(row2, editIndex == 0 ? PSTR(">Fog Timer   : %.2d:%.2d") : PSTR("Fog Timer    : %.2d:%.2d"), (int) config_fog_counter / 60, (int) config_fog_counter % 60);
        sprintf_P(row3, editIndex == 1 ? PSTR(">Oven Timer  : %.2d:%.2d") : PSTR("Oven Timer   : %.2d:%.2d"), (int) config_oven_counter / 60, (int) config_oven_counter % 60);
        sprintf_P(row4, editIndex == 2 ? PSTR(">Oven Temp   : %.2d   ")   : PSTR("Oven Temp    : %.2d   "), config_oven_temp);
//        sprintf_P(row4, PSTR("Machine cycle: %5d"), cycle_counter);
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

// user input
bool onBackButton() {
  if (digitalRead(backButtonPin)) {
    delay(10);
    while (digitalRead(backButtonPin));
    return true;
  }
  else return false;
}
bool onSetButton() {
  if (digitalRead(setButtonPin)) {
    delay(10);
    while (digitalRead(setButtonPin));
    return true;
  }
  else return false;
}
bool onDecButton() {
  if (digitalRead(decButtonPin)) {
    delay(10);
    return true;
  }
  else return false;
}
bool onIncButton() {
  if (digitalRead(incButtonPin)) {
    delay(10);
    return true;
  }
  else return false;
}

//fog relay
void fogRelayOn() {
  digitalWrite(fogRelayPin, HIGH);
  Serial.println(F(">>> fogRelay: turning on"));
}
void fogRelayOff() {
  digitalWrite(fogRelayPin, LOW);
  Serial.println(F(">>> fogRelay: turning off"));
}

//oven relay
void ovenRelayOn() {
  digitalWrite(ovenRelayPin, HIGH);
  Serial.println(F(">>> ovenRelay: turning on"));
}
void ovenRelayOff() {
  digitalWrite(ovenRelayPin, LOW);
  Serial.println(F(">>> ovenRelay: turning off"));
}

//lm35 temperature measurement
void read_temperature(){
  float oven_temp = analogRead(temperaturePin);
  value = value * 500 / 1024; // equavalent voltage in v
  oven_temp = (int) value;
  Serial.print(">>> oven temp: ");
  Serial.print(oven_temp);
  Serial.println(" cel");
}

//eeprom
void save() {
  EEPROM.put(fog_address, config_fog_counter);
  EEPROM.put(oven_address, config_oven_counter);
  EEPROM.put(cycle_address, cycle_counter);
  EEPROM.put(oven_temp_address, config_oven_temp);
  Serial.println(">>> eeprom: saving config");
}
void read() {
  EEPROM.get(fog_address, config_fog_counter);
  EEPROM.get(oven_address, config_oven_counter);
  EEPROM.get(cycle_address, cycle_counter);
  EEPROM.get(oven_temp_address, config_oven_temp);
  Serial.println(">>> eeprom: reading config");
}

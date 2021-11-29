#include "constants.h"
#include "EEPROM.h"
#include "LiquidCrystal.h"

Screen screen = screen_intro;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
byte editIndex = 0;
long start_time = 0;


void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);
}

void loop() {
  screen_handle();
  screen_display();
}

void screen_handle() {
  switch (screen) {
    case screen_intro : {
        if (onSetButton())  screen = screen_timer_fog_display;
        else if (onBackButton()) screen = screen_config;
        break;
      }
    case screen_timer_fog_display : {
        fog_counter = config_fog_counter;
        if (onSetButton())  {
          screen = screen_timer_fog_start;
          start_time = millis();
        }
        else if (onBackButton()) screen = screen_intro;
        break;
      }
    case screen_timer_fog_start : {
        if(millis() > start_time + 1000){
          if(fog_counter > 0) --fog_counter; 
          else screen = screen_timer_oven_display;
          start_time = millis();
        }
        
        if (onBackButton()) screen = screen_intro;

        break;
      }
    case screen_timer_oven_display : {
        oven_counter = config_oven_counter;
        if (onSetButton())  {
          screen = screen_timer_oven_start;
          start_time = millis();
        }
        else if (onBackButton()) screen = screen_intro;

        break;
      }
    case screen_timer_oven_start : {
        if(millis() > start_time + 1000){
          if(oven_counter > 0) --oven_counter; 
          else screen = screen_complete;
          start_time = millis();
        }
        if (onBackButton()) screen = screen_intro;

        break;
      }
    case screen_complete : {
        if (onSetButton())  screen = screen_intro;
        break;
      }
    case screen_config : {
        if (onSetButton()) {
          if (editIndex < 2) ++editIndex;
          else {
            screen = screen_intro;
            editIndex = 0;
          }
        }
        else if (onDecButton()) {
          delay(200);
          if (editIndex == 0) {
            do {
              config_fog_counter  = config_fog_counter  > 0 ? config_fog_counter  - 1 : max_fog_counter;
              delay(20);
              screen_display();
            }
            while (digitalRead(decButtonPin));
          }
          if (editIndex == 1) {
            do {
              config_oven_counter = config_oven_counter > 0 ? config_oven_counter - 1 : max_oven_counter;
              delay(20);
              screen_display();
            }
            while (digitalRead(decButtonPin));
          }
        }
        else if (onIncButton()) {
          delay(200);
          if (editIndex == 0) {
            do {
            config_fog_counter  = config_fog_counter  < max_fog_counter  ? config_fog_counter  + 1 : 0;
              delay(20);
              screen_display();
            }
            while (digitalRead(incButtonPin));
          }
          if (editIndex == 1) {
            do {
            config_oven_counter = config_oven_counter < max_oven_counter ? config_oven_counter + 1 : 0;
              delay(20);
              screen_display();
            }
            while (digitalRead(incButtonPin));
          }
        }
        else if (onBackButton()) screen = screen_intro;
        break;
      }
    default: break;
  }
}

void screen_display() {
  switch (screen) {
    case screen_intro : {
        lcd.setCursor(0, 0);
        lcd.print(F(" Unique Technology  "));
        lcd.setCursor(0, 1);
        lcd.print(F("       Sangli       "));
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        lcd.setCursor(0, 3);
        lcd.print(F("Start         Config"));
        break;
      }
    case screen_timer_fog_display : {
        lcd.setCursor(0, 0);
        lcd.print(F("     Fog timer      "));
        lcd.setCursor(0, 1);
        char buff[20];//mm:ss
        sprintf(buff, "Timer: %.2d:%.2d        ", (int) fog_counter / 60, (int) fog_counter % 60);
        lcd.print(buff);
        lcd.setCursor(0, 2);
        lcd.print(F("Temp.:              "));
        lcd.setCursor(0, 3);
        lcd.print(F("Start          Abort"));
        break;
      }
    case screen_timer_fog_start : {
        lcd.setCursor(0, 0);
        lcd.print(F("     Fog timer      "));
        lcd.setCursor(0, 1);
        char buff[20];//mm:ss
        sprintf(buff, "Timer: %.2d:%.2d        ", (int) fog_counter / 60, (int) fog_counter % 60);
        lcd.print(buff);
        lcd.setCursor(0, 2);
        lcd.print(F("Temp.:              "));
        lcd.setCursor(0, 3);
        lcd.print(F("               Abort"));
        break;
      }
    case screen_timer_oven_display : {
        lcd.setCursor(0, 0);
        lcd.print(F("     Oven timer     "));
        lcd.setCursor(0, 1);
        char buff[20];//mm:ss
        sprintf(buff, "Timer: %.2d:%.2d        ", (int) oven_counter / 60, (int) oven_counter % 60);
        lcd.print(buff);
        lcd.setCursor(0, 2);
        lcd.print(F("Temp.:              "));
        lcd.setCursor(0, 3);
        lcd.print(F("Start          Abort"));
        break;
      }
    case screen_timer_oven_start : {
        lcd.setCursor(0, 0);
        lcd.print(F("     Oven timer     "));
        lcd.setCursor(0, 1);
        char buff[20];//mm:ss
        sprintf(buff, "Timer: %.2d:%.2d        ", (int) oven_counter / 60, (int) oven_counter % 60);
        lcd.print(buff);
        lcd.setCursor(0, 2);
        lcd.print(F("Temp.:              "));
        lcd.setCursor(0, 3);
        lcd.print(F("               Abort"));
        break;
      }
    case screen_complete : {
        lcd.setCursor(0, 0);
        lcd.print(F("   Cycle complete   "));
        lcd.setCursor(0, 1);
        lcd.print(F("                    "));
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        lcd.setCursor(0, 3);
        lcd.print(F("Restart             "));
        break;
      }
    case screen_config : {
        char buff[20];//mm:ss
        lcd.setCursor(0, 0);
        lcd.print(F(" Configuration Mode "));
        lcd.setCursor(0, 1);
        sprintf(buff, editIndex == 0 ? ">Fog Timer   : %.2d:%.2d" : "Fog Timer    : %.2d:%.2d", (int) config_fog_counter / 60, (int) config_fog_counter % 60);
        lcd.print(buff );// display in min:sec format
        lcd.setCursor(0, 2);
        sprintf(buff, editIndex == 1 ? ">Oven Timer  : %.2d:%.2d" : "Oven Timer   : %.2d:%.2d", (int) config_oven_counter / 60, (int) config_oven_counter % 60);
        lcd.print(buff);// display in min:sec format
        lcd.setCursor(0, 3);
        sprintf(buff, "Machine cycle: %5d", cycle_counter);
        lcd.print(buff);// display machine cycle counter
        break;
      }
    default: break;
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

enum Screen {
//  screen_home,
  screen_timer_select,
  screen_timer_fog_start,
  screen_timer_fog_completed,
  screen_timer_oven_start,
  screen_timer_oven_completed,
  screen_config,
  screen_reset_to_default,
  screen_error
};

enum EditIndex {
  edit_cycle,
  edit_fog_timer,
  edit_oven_timer,
  edit_oven_temp,
  edit_complete
};
enum I2cCommand {
  i2c_null = 0,
  i2c_inc = 1,
  i2c_dec = 2,
  i2c_set = 3,
  i2c_back = 4,
};
void waitForButtonRelease() {
  delay(200);
}

struct Button {
  byte pin;
  Button(byte _pin): pin(_pin) {}

  bool onPress() {
    if (digitalRead(pin)) {
      delay(10);
      return true;
    } else return false;
  }

  bool onRelease() {
    if (digitalRead(pin)) {
      delay(10);
      while (digitalRead(pin));
      return true;
    } else return false;
  }
};

struct Relay {
  byte pin;
  byte value;
  Relay(byte _pin) : pin(_pin) {
    pinMode(pin, OUTPUT);
  }
  void on() {
    //    Serial.println("on");
    value = 1;
    digitalWrite(pin, HIGH);
  }
  void off() {
    //    Serial.println("off");
    value = 0;
    digitalWrite(pin, LOW);
  }
};

// millis // for timer counter
long lastChanged = 0;
void resetMsDelay() {
  lastChanged = millis();
}
bool onMsDelayPass(int _time) {
  if (millis() > lastChanged + _time) {
    resetMsDelay();
    return true;
  } else {
    return false;
  }
}

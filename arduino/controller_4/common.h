enum Screen {
  screen_home,
  screen_timer_select,
  screen_timer_fog_display,
  screen_timer_fog_start,
  screen_timer_oven_display,
  screen_timer_oven_start,
  screen_complete,
  screen_config,
  screen_error
};

enum EditIndex {
  edit_cycle,
  edit_fog_timer,
  edit_oven_timer,
  edit_oven_temp,
  edit_complete
};

void waitForButtonRelease(){
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

struct Relay{
  byte pin;
  Relay(byte _pin) : pin(_pin){
    pinMode(pin, OUTPUT);
  }
  void on(){
    digitalWrite(pin, HIGH);
  }
  void off(){
    digitalWrite(pin, LOW);
  }
};

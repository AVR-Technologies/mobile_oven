#include "Wire.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//i2c address
const byte i2c_address      = 8;
BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (int length = SerialBT.available()) {
    byte ind = SerialBT.read();
    Serial.print(ind);
    Wire.beginTransmission(i2c_address);
    Wire.write(ind);
    Wire.endTransmission(true);
  }
}

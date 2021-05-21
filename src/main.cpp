#include <Arduino.h>

/* BLE Example for SparkFun Pro nRF52840 Mini 
 *  
 *  This example demonstrates how to use the Bluefruit
 *  library to both send and receive data to the
 *  nRF52840 via BLE.
 *  
 *  Using a BLE development app like Nordic's nRF Connect
 *  https://www.nordicsemi.com/eng/Products/Nordic-mobile-Apps/nRF-Connect-for-Mobile
 *  The BLE UART service can be written to to turn the
 *  on-board LED on/off, or read from to monitor the 
 *  status of the button.
 *  
 *  See the tutorial for more information:
 *  https://learn.sparkfun.com/tutorials/nrf52840-development-with-arduino-and-circuitpython#arduino-examples  
*/

#include <bluefruit.h>
#include <arduino-timer.h>
#include <Bounce2.h>
#include "Joystick.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// display
#define OLED_RESET 4

// define joystick pins and joystick button pins
#define JOYSTICK_XPIN 5 
#define JOYSTICK_YPIN 4 
#define JOYSTICK_BUTTON_PIN 9
#define JOYSTICK_BUTTON_SHIFT 0

// define time to display voltage 
#define VOLTAGE_DISPLAY_PERIOD 10000

BLEUart bleuart; // uart over ble

Joystick joystick(JOYSTICK_XPIN, JOYSTICK_YPIN, 1024, 0.15);

// create timer 
auto timer = timer_create_default(); // create a timer with default settings

// display 
Adafruit_SSD1306 display(OLED_RESET);

int batteryVoltage;
double percent;

bool sendMessage(void *)
{
  static uint8_t mes[5];

  uint16_t *x = (uint16_t *)&mes[0];
  uint16_t *y = (uint16_t *)&mes[2];
  uint8_t *_buttons = &mes[4];

  *x = tu_htons(joystick.getXraw());
  *y = tu_htons(joystick.getYraw());

  bleuart.write(mes, 5);
  return true;
}

void setup()
{
  // Initialize hardware:
  Serial.begin(9600);       // Serial is the USB serial port

  // Initialize Bluetooth:
  Bluefruit.begin();
  Bluefruit.setTxPower(8);
  Bluefruit.setName("MuggCaseRemote");
  bleuart.begin();

  // Start advertising device and bleuart services
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  // Set advertising interval (in unit of 0.625ms):
  Bluefruit.Advertising.setInterval(40, 180);
  // number of seconds in fast mode:
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);

  timer.every(20, sendMessage);

  // display 
  Wire.setPins(20,19); // configure i2c pins

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  batteryVoltage = analogRead(28);

  percent = mapF((double)batteryVoltage, 0.0, 1023.0, 0.0, 5.0) / 3.7;
  percent *= 100;

  // display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.print(percent);
  display.println(" %");

  display.display();
  delay(VOLTAGE_DISPLAY_PERIOD);

  display.clearDisplay();
  display.display();
}

void loop()
{
  // If data has come in via BLE:
  if (bleuart.available())
  {
    uint8_t c;
    // use bleuart.read() to read a character sent over BLE
    c = (uint8_t)bleuart.read();
  }

  timer.tick();
}
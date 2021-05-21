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

// Define hardware: LED and Button pins and states
const int LED_PIN = 7;
#define LED_OFF LOW
#define LED_ON HIGH

const int RGBLED_CLOCK_PIN = 19;
const int RGBLED_DATA_PIN = 20;
const int RGBLED_LATCH_PIN = 21;

#define JOYSTICK_XPIN 5 // stimmt
#define JOYSTICK_YPIN 4 
#define JOYSTICK_BUTTON_PIN 9
#define JOYSTICK_BUTTON_SHIFT 0

const int MAIN_BUTTON_PIN = 22;
#define MAIN_BUTTON_SHIFT 1

const int BUTTON_PIN = 13;
#define BUTTON_ACTIVE LOW
int lastButtonState = -1;

#define NUM_BUTTONS 2
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {JOYSTICK_BUTTON_PIN, MAIN_BUTTON_PIN};

BLEUart bleuart; // uart over ble
Joystick joystick(JOYSTICK_XPIN, JOYSTICK_YPIN, 1024, 0.15);
auto timer = timer_create_default(); // create a timer with default settings
Bounce *buttons = new Bounce[NUM_BUTTONS];

// display 
Adafruit_SSD1306 display(OLED_RESET);

int voltageDisplayPeriod = 20000;
int currentTime;

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

  // Serial.printf("X: %d, Y: %d\n", tu_ntohs(*x), tu_ntohs(*y));
  *_buttons = (!buttons[0].read()) << JOYSTICK_BUTTON_SHIFT;
  *_buttons |= ((!buttons[1].read()) << MAIN_BUTTON_SHIFT);

  bleuart.write(mes, 5);
  return true;
}

bool setRGBLED(void *data)
{
  return false;
  return true;
}

void setup()
{
  // Initialize hardware:
  Serial.begin(9600);       // Serial is the USB serial port
  pinMode(LED_PIN, OUTPUT); // Turn on-board blue LED off
  digitalWrite(LED_PIN, LED_OFF);
  pinMode(RGBLED_CLOCK_PIN, OUTPUT);
  pinMode(RGBLED_DATA_PIN, OUTPUT);
  pinMode(RGBLED_LATCH_PIN, OUTPUT);
  digitalWrite(RGBLED_LATCH_PIN, HIGH);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLDOWN);

  pinMode(BUTTON_PIN, INPUT);
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP); //setup the bounce instance for the current button
    buttons[i].interval(25);                         // interval in ms
  }

  //Bluefruit.autoConnLed(false);

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
  // timer.every(50, [](void*) -> bool { digitalWrite(LED_BLUE, !digitalRead(LED_BLUE)); return true;});
  timer.every(1000, setRGBLED, (void *)0xFF0000);

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
  display.println("not connected");
  display.display();
  delay(voltageDisplayPeriod);

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

    // If the character is one of our expected values,
    // do something:
    switch (c)
    {
    // 0 number or character, turn the LED off:
    case 0:
    case '0':
      digitalWrite(LED_PIN, LED_OFF);
      break;
    // 1 number or character, turn the LED on:
    case 1:
    case '1':
      digitalWrite(LED_PIN, LED_ON);
      break;
    default:
      break;
    }
  }

  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].update();
  }

  timer.tick();
}
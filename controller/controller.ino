//#include <avr/stof.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <RH_RF69.h>

// set up radio
#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4
#define LED           13
#define RF69_FREQ     915.0

// set up power pin
#define POWER_PIN     A7

// set up button LEDs
#define LEFTLED       A3
#define RIGHTLED      A0
#define LEFTBUTTON    A2
#define RIGHTBUTTON   A1

const int noBlinker = 0;
const int leftBlinker = 1;
const int rightBlinker = 2;
const int visibilitySignal = 3;

const long debounceTime = 300;
const int centerButton = 13;

volatile int blinkMode = noBlinker;
volatile unsigned long last_micros;

//until the blinker has reported its battery level, this is false
bool blinkerBatteryChecked = false;

//Initialize the radio
RH_RF69 radio(RFM69_CS, RFM69_INT);

int checkIncrement = 0;

void setup() {
  Serial.begin(9600);

  pinMode(RIGHTBUTTON, INPUT_PULLUP);
  pinMode(LEFTBUTTON, INPUT_PULLUP);
  pinMode(centerButton, INPUT_PULLUP);
  pinMode(RFM69_RST, OUTPUT);
  pinMode(POWER_PIN, INPUT);
  digitalWrite(RFM69_RST, LOW);
  pinMode(LED_BUILTIN, OUTPUT);

  //  Reset the radio
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!radio.init()) {
    Serial.println("Radio failed to initialize");
    while (1);
  }
  Serial.println("Radio initialized OK");
  if (!radio.setFrequency(RF69_FREQ)) {
    Serial.println("Failed to set frequency");
  }

  radio.setTxPower(15, true);
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  radio.setEncryptionKey(key);

  while (!listenForHandshake()) {
    delay(500);
  }

  attachInterrupt(digitalPinToInterrupt(RIGHTBUTTON), rightButtonListener, FALLING);
  attachInterrupt(digitalPinToInterrupt(LEFTBUTTON), leftButtonListener, FALLING);
  attachInterrupt(digitalPinToInterrupt(centerButton), centerButtonListener, FALLING);
  last_micros = micros();
} 

void loop() {
  // TODO: create the indicator states
  switch (blinkMode) {
    case noBlinker:
      delay(10);
      break;
    case leftBlinker:
      // indicate left blinker
      runBlinker(LEFTLED);
      break;
    case rightBlinker:
      // indicate right blinker
      runBlinker(RIGHTLED);
      break;
    case visibilitySignal:
      // indicate visibility signal
      break;
  }
}

void runBlinker(int led) {
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);
}

void runHazard() {
  digitalWrite(LEFTLED, HIGH);
  digitalWrite(RIGHTLED, HIGH);
  delay(500);
  digitalWrite(LEFTLED, LOW);
  digitalWrite(RIGHTLED, LOW);
}

void rightButtonListener () {
  if (long(micros() - last_micros) >= debounceTime * 1000) {
    if (blinkMode == rightBlinker) {
      Serial.println("unset right");
      blinkMode = noBlinker;
    } else {
      Serial.println("set right");
      blinkMode = rightBlinker;
    }
    sendState();

    last_micros = micros();
  }
}

void leftButtonListener () {
  if (long(micros() - last_micros) >= debounceTime * 1000) {
    if (blinkMode == leftBlinker) {
      Serial.println("unset left");
      blinkMode = noBlinker;
    } else {
      Serial.println("set left");
      blinkMode = leftBlinker;
    }
    sendState();

    last_micros = micros();
  }
}

void centerButtonListener () {
  if (long(micros() - last_micros) >= debounceTime * 1000) {
//    Serial.println("unset blinker");
//    blinkMode = noBlinker;
    if (blinkMode == visibilitySignal) {
      Serial.println("unset visibility");
      blinkMode = noBlinker;
    } else {
      Serial.println("set visibility");
      blinkMode = visibilitySignal;
    }
    sendState();
    
    last_micros = micros();
  }
}

void sendState () {
  char stateMessage[1];
  itoa(blinkMode, stateMessage, 10);
  radio.send((uint8_t *)stateMessage, 1);
  //  radio.waitPacketSent();
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  delay(500);
}

void checkBattery() {
  float currPower = analogRead(POWER_PIN) * 2 * 3.3 / 1024;
  if (currPower <= 3.4) {
    // low power animation

  }
  delay(5000);
}

boolean listenForHandshake() {
  // TODO: indicate listen 
  for (int i = 0; i < 128; i ++) {
    analogWrite(LED_BUILTIN, i * 2);
    delay(5);
  }
  for (int i = 128; i > 0; i --) {
    analogWrite(LED_BUILTIN, i * 2);
    delay(5);
  }
  
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  char alertChecked[1];
  alertChecked[0] = 'b';
  float blinkerBatteryLevel = 0;
  if (radio.available()) {
    if (radio.recv(buf, &len)) {
      if (!len) return false;
      buf[len] = 0;
      String bufferString = (char *)buf;
      blinkerBatteryLevel = bufferString.toFloat();
      radio.send((uint8_t *)alertChecked, 1);
      // blinker low battery indication
      delay(4000);
      return true;
    }
  }
  return false;
}

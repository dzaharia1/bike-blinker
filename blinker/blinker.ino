#include <avr/dtostrf.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <RH_RF69.h>

// Radio properties
#define RF69_FREQ 915.0
#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4
#define LED           13

// power measure pin
#define POWER_PIN A7

// Blink mode definitions
const char noBlinker = '0';
const char left = '1';
const char right = '2';
char blinkMode = noBlinker;

// Display properties
const int maxBrightness = 255;
const int minBrightness = 0;
const int ledWidth = 15;
const int ledHeight = 7;

Adafruit_IS31FL3731_Wing ledMatrix = Adafruit_IS31FL3731_Wing();
RH_RF69 radio(RFM69_CS, RFM69_INT);

void setup () {
  Serial.begin(9600);

//  set up the radio
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!radio.init()) {
    Serial.println("Radio didn't initialize");
    while (1);
  }
  if (!radio.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }
  
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  radio.setEncryptionKey(key);

//  initialize the led matrix
  if (!ledMatrix.begin()) {
    Serial.println("display not found");
    while(1);
  }

  ledMatrix.fillScreen(minBrightness);

  pinMode(POWER_PIN, INPUT);

  while (!attemptHandshake()) {
    delay(500);
  }
}

void loop () {
  switch (blinkMode) {
    case noBlinker:
      ledMatrix.fillScreen(minBrightness);
      break;
    case left:
      ledMatrix.setRotation(0);
      runBlinker();
      break;
    case right:
      ledMatrix.setRotation(2);
      runBlinker();
      break;
  }

  if (radio.available()) {
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (radio.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received: ");
      Serial.println((char*)buf);
      blinkMode = buf[0];
    } else {
      Serial.println("Didn't get anything");
    }

    // todo: reply to confirm reciept
  }
}

void runBlinker() {
  int numFrames = 5;
  int maxFactor = pow(maxBrightness, 2);
  int increment = maxFactor / numFrames;
  
  for (int i = 0; i < ledWidth + numFrames; i ++) {
    for (int j = numFrames; j >= 0; j --) {
      ledMatrix.drawLine(i - j, 0, i - j, ledHeight, maxBrightness - sqrt(increment * j));
    }
    delay(50);
  }
  
  ledMatrix.fillScreen(minBrightness);
}

boolean attemptHandshake() {
  Serial.println("Attempting handshake");
  // flash the led matrix to indicate attempting pairing
  for (int i = 0; i < 2; i ++) {
    ledMatrix.fillScreen(maxBrightness / 2);
    delay(50);
    ledMatrix.fillScreen(minBrightness);
    delay(50);
  }
  
  // send the controller the current battery level
  float currPower = analogRead(POWER_PIN) * 2 * 3.3 / 1024;
  char powerString[RH_RF69_MAX_MESSAGE_LEN];
  dtostrf(currPower, 4, 2, powerString);
  Serial.print("  sending battery level ");
  Serial.println(powerString);
  radio.send((uint8_t *)powerString, strlen(powerString));

//  listen for a reply from the controller
  uint8_t replyBuf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t replyLen = sizeof(replyBuf);
  boolean handshook = false;
  if (radio.waitAvailableTimeout(500)) {
    if (radio.recv(replyBuf, &replyLen)) {
      if (replyBuf[0] == 'b') {
        Serial.println("  Got a reply");
        return true;
      }
    }
  }
  Serial.println("  Haven't heard back");
  return false;
}

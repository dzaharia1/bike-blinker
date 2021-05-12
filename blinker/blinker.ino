#include <Adafruit_IS31FL3731.h>
#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <RH_RF69.h>

//Set up the radio
#define RF69_FREQ 915.0
#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4
#define LED           13

//set up blink modes
const char noBlinker = '0';
const char left = '1';
const char right = '2';
char blinkMode = left;

//set up display properties
const int maxBrightness = 255;
const int minBrightness = 0;
const int ledWidth = 15;
const int ledHeight = 7;

Adafruit_IS31FL3731_Wing ledMatrix = Adafruit_IS31FL3731_Wing();
RH_RF69 radio(RFM69_CS, RFM69_INT);

void setup () {
  Serial.begin(9600);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

//  Reset the radio
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!radio.init()) {
    Serial.println("Radio didn't initialize");
    while (1);
  }

  Serial.println("Radio started ok");

  if (!ledMatrix.begin()) {
    Serial.println("display not found");
    while(1);
  }
  if (!radio.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  radio.setEncryptionKey(key);

  ledMatrix.fillScreen(minBrightness);
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
      Serial.println(blinkMode);
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

//#include <avr/stof.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <RH_RF69.h>

// set up tft properties
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128
#define SCLK_PIN 21   // CL
#define MOSI_PIN 20   // SI
#define DC_PIN   5    // DC
#define CS_PIN   10   // OC
#define RST_PIN  6    // R

// Color definitions
#define  BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

//set up radio
#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4
#define LED           13
#define RF69_FREQ 915.0

#define POWER_PIN A7

const int noBlinker = 0;
const int leftBlinker = 1;
const int rightBlinker = 21;

const long debounceTime = 200;
const int rightButton = 12;
const int leftButton = 11;
const int centerButton = 13;

volatile int blinkMode = noBlinker;
volatile unsigned long last_micros;

//until the blinker has reported its battery level, this is false
bool blinkerBatteryChecked = false;

//Initialize the display
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);

//Initialize the radio
RH_RF69 radio(RFM69_CS, RFM69_INT);

void setup() {
  Serial.begin(9600);

  pinMode(rightButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(centerButton, INPUT_PULLUP);
  pinMode(RFM69_RST, OUTPUT);
  pinMode(POWER_PIN, INPUT);
  digitalWrite(RFM69_RST, LOW);
  tft.begin();

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

  radio.setTxPower(20, true);

  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  radio.setEncryptionKey(key);

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

  attachInterrupt(digitalPinToInterrupt(rightButton), rightButtonListener, FALLING);
  attachInterrupt(digitalPinToInterrupt(leftButton), leftButtonListener, FALLING);
  attachInterrupt(digitalPinToInterrupt(centerButton), centerButtonListener, FALLING);
  last_micros = micros();

  checkPower();

  tft.fillScreen(BLACK);
}

void loop() {
  switch (blinkMode) {
    case noBlinker:
      tft.fillScreen(BLACK);
      break;
    case leftBlinker:
      tft.setRotation(1);
      runBlinker();
      break;
    case rightBlinker:
      tft.setRotation(3);
      runBlinker();
      break;
  }

  if (!blinkerBatteryChecked) {
    checkBlinkerPower();
  }
}

void runBlinker () {
  int initialBlinkMode = blinkMode;
  int triangleHeight = 40;
  for (int i = 0; i < SCREEN_WIDTH + triangleHeight; i ++) {
    if (blinkMode != initialBlinkMode) {
      break;
    }
    //    tft.drawLine(0, i, SCREEN_WIDTH, i, YELLOW);
    tft.drawTriangle(SCREEN_WIDTH / 2, i, SCREEN_WIDTH, i - triangleHeight, 0, i - triangleHeight, YELLOW);

  }
  tft.fillScreen(BLACK);
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
    Serial.println("unset blinker");
    blinkMode = noBlinker;
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

  //  if (radio.waitAvailableTimeout(500)) {
  //    if (radio.recv(buf, &len)) {
  //      Serial.print("Got a reply: ");
  //      Serial.println((char*)buf);
  //    }
  //  } else {
  //    Serial.println("No reply");
  //  }
}

void checkPower() {
  float currPower = analogRead(POWER_PIN) * 2 * 3.3 / 1024;

  if (currPower <= 3.4) {
    tft.setTextSize(2.5);
    tft.setTextColor(RED);
    tft.setCursor(25, 0);
    tft.println("Please");
    tft.setCursor(10, 20);
    tft.println("recharge!");
    tft.fillRect(15, 70, 5, 20, RED);
    tft.drawRect(20, 50, 90, 60, RED);
    tft.fillRect(100, 50, 10, 60, RED);
    //    tft.drawRect

  }
  delay(5000);
}

void checkBlinkerPower () {
  Serial.println("checking blinker power");
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  char alertChecked[1];
  alertChecked[0] = 'b';
  float blinkerBatteryLevel = 0;
  noInterrupts();
  if (radio.available()) {
    if (radio.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      String bufferString = (char *)buf;
      blinkerBatteryLevel = bufferString.toFloat();
      Serial.println(blinkerBatteryLevel);
      radio.send((uint8_t *)alertChecked, 1);
      blinkerBatteryChecked = true;
      delay(200);

      if (blinkerBatteryLevel <= 3.4) {
        tft.fillScreen(BLACK);
        tft.setTextSize(2.5);
        tft.setTextColor(RED);
        tft.setCursor(18, 0);
        tft.println("Recharge");
        tft.setCursor(18, 20);
        tft.println("blinker!");
        tft.fillRect(15, 70, 5, 20, RED);
        tft.drawRect(20, 50, 90, 60, RED);
        tft.fillRect(100, 50, 10, 60, RED);
        delay(3000);
      }
    } else {
      tft.println("Didn't get anything");
    }
  }
  
  interrupts();
}

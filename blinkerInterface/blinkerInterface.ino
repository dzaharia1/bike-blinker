#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

// You can use any (4 or) 5 pins 
#define SCLK_PIN 8
#define MOSI_PIN 9
#define DC_PIN   4
#define CS_PIN   5
#define RST_PIN  6

// Color definitions
#define  BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#define NO_BLINKER 0
#define LEFT_BLINKER 1
#define RIGHT_BLINKER 2

volatile int blinkMode = NO_BLINKER;
volatile unsigned long last_micros;
long debounceTime = 200;

const int rightButton = 2;
const int leftButton = 3;

struct coordinate {
  int x;
  int y;
};

// Option 1: use any pins but a little slower
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  
void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(BLACK);
  
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(rightButton), rightButtonListener, FALLING);
  attachInterrupt(digitalPinToInterrupt(leftButton), leftButtonListener, FALLING);

  blinkMode = NO_BLINKER;
  last_micros = micros();
}

void loop() {
  drawBlinker();
}

void drawBlinker () {
  switch (blinkMode) {
    case NO_BLINKER:
      Serial.println("none");
      tft.fillScreen(BLACK);
      break;
    case LEFT_BLINKER:
      leftBlinker();
      break;
    case RIGHT_BLINKER:
      rightBlinker();
      break;
  }
}

void rightBlinker () {
  Serial.println("right");
  tft.setRotation(3);
  for (int i = 0; i < SCREEN_WIDTH; i ++) {
    if (blinkMode != RIGHT_BLINKER) { break; }
    tft.drawLine(0, i, SCREEN_WIDTH, i, YELLOW);
  }
  tft.fillScreen(BLACK);
}

void leftBlinker () {
  Serial.println("left");
  tft.setRotation(1);
  for (int i = 0; i < SCREEN_WIDTH; i ++) {
    if (blinkMode != LEFT_BLINKER) { break; }
    tft.drawLine(0, i, SCREEN_WIDTH, i, YELLOW);
  }
  tft.fillScreen(BLACK);
}

void rightButtonListener () {
  if (long(micros() - last_micros) >= debounceTime * 1000) {
    if (blinkMode == RIGHT_BLINKER) {
      Serial.println("unset right");
      blinkMode = NO_BLINKER;
    } else {
      Serial.println("set right");
      blinkMode = RIGHT_BLINKER;
    }
    
    last_micros = micros();
  }
}

void leftButtonListener () {
  if (long(micros() - last_micros) >= debounceTime * 1000) {
    if (blinkMode == LEFT_BLINKER) {
      Serial.println("unset left");
      blinkMode = NO_BLINKER;
    } else {
      Serial.println("set left");
      blinkMode = LEFT_BLINKER;
    }
    
    last_micros = micros();
  }
}

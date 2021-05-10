#include <Adafruit_SSD1351.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <gfxfont.h>

// #include <nRF24L01.h>
// #include <printf.h>
// #include <RF24_config.h>
// #include <RF24.h>

// #include <SoftwareSerial.h>

// int inputHorizontalPin = 2;
// int inputVerticalPin = 3;

// void setup() {
//     Serial.begin(9600);
//     pinMode(inputHorizontalPin, INPUT);
// }

// void loop() {
//     int horizontalValue = analogRead(inputHorizontalPin);
//     int verticalValue = analogRead(inputVerticalPin);

//     Serial.print(horizontalValue + " | " + verticalValue);
// }

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
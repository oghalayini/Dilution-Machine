#include <Arduino.h>

const int dir = 2;
const int step = 3;

const int colorButton = 8;

const int MOSFET = 9;

const int metalButton = 10;
const int metalButtonLED = 11;

const int encoderRight = 18;
const int encoderLeft = 19;
const int encoderButton = 30;

void setup() {
  pinMode(dir,OUTPUT);
  pinMode(step,OUTPUT);

  pinMode(colorButton,INPUT);

  pinMode(MOSFET,OUTPUT);

  pinMode(metalButton,INPUT);
  pinMode(metalButtonLED,OUTPUT);

  Serial.begin(9600);
}

void loop() {
  /*
  Serial.print("Color Button Status: ");
  Serial.println(digitalRead(colorButton));

  Serial.print("Metal Button Status: ");
  Serial.println(digitalRead(metalButton));

  if (digitalRead(metalButton)) {
    digitalWrite(metalButtonLED,HIGH);
  } else {
    digitalWrite(metalButtonLED,LOW);
  }
  */

  delay(1500);
}
#include <Arduino.h>

#include <Keypad.h>

const int dir = 2;
const int step = 3;

const int colorButton = 8;

const int MOSFET = 9;

const int metalButton = 10;
const int metalButtonLED = 11;

const int encoderRight = 18;
const int encoderLeft = 19;
const int encoderButton = 30;

const int ROWS = 4;
const int COLS = 4;

byte rowPins[4] = {45, 44, 43, 42};
byte colPins[4] = {41, 40, 39, 38};

char key;

char numkeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad numpad = Keypad(makeKeymap(numkeys),rowPins,colPins,ROWS,COLS);

void setup() {
  pinMode(dir,OUTPUT);
  pinMode(step,OUTPUT);

  pinMode(colorButton,INPUT);

  pinMode(MOSFET,OUTPUT);

  pinMode(metalButton,INPUT);
  pinMode(metalButtonLED,OUTPUT);

  numpad.begin(makeKeymap(numkeys));
  numpad.setHoldTime(500);

  Serial.begin(9600);
}

void loop() {
  /* Button Tests
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

  /* Keypad Test
  char key = numpad.getKey();
  if (key) {
    Serial.println(key);
  }
  */
}
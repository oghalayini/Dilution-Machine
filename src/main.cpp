#include <Arduino.h>

#include <A4988.h>

#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include <Adafruit_TFTLCD.h>
#include <Adafruit_SPIDevice.h>
#include <Wire.h>

#include <RotaryEncoder.h>
 
// Color definitions
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define DILUTBLU    0x04B9

volatile int flow_frequency;
float vol = 0.0;
float l_minute=0;

unsigned long currentTime;
unsigned long cloopTime;

const int cs = 7;
const int cd = 6;
const int wr = 5;
const int rd = 4;
const int rst = 3;
const int lite = 2;

const int dir = 8;
const int step = 9;
const int en = 10;

const int MOSFET = 11;

const int metalButton = A1;
const int metalButtonLED = 30;

const int encoderRight = 18;
const int encoderLeft = 19;
const int encoderButton = A0;

const int flowSensor = 20;

const int xp = A12;
const int yp = A13;
const int xm = A14;
const int ym = A15;

const int steps = 200;
const int rpm = 120;

A4988 motor = A4988(steps, dir, step);

Adafruit_TFTLCD tft(cs, cd, wr, rd, rst);

TouchScreen ts = TouchScreen(xp, yp, xm, ym, 285);

Adafruit_GFX_Button buttons[5];

Adafruit_GFX_Button dButtons[3];

RotaryEncoder *encoder = nullptr;

bool refresh = true;
bool dilution = true;

int TSRight1 = 125; //Need better calibration
int TSLeft1 = 901;
int TSTop1 = 78;
int TSBot1 = 940;

int stack[20], n = 20, top = -1;

char buttonLabels[5][15] = {"Button 0", "Button 1", "Button 2","Button 3", "Button 4"};

char dButtonLabels[7][3][15] = {
  {"Dilution", "Statistics"}, 
  {"Percent","Fill"}, 
  {"Percent","Fill"},
  {"Custom", "Preset1"},
  {"Hold to Fill","Preset 1"},
  {"Encoder","Touch"},
  {"Backlight","Sleep"}
  };

/******************* UI details */
#define BUTTON_X 52
#define BUTTON_Y 150
#define BUTTON_W 80
#define BUTTON_H 45
#define BUTTON_SPACING_X 26
#define BUTTON_SPACING_Y 30
#define BUTTON_TEXTSIZE 3

int pixel_x;
int pixel_y;

void checkPosition() {
  encoder->tick(); // just call tick() to check the state.
}

void drawOutline(int buttonX0, int buttonY0, int buttonW, int buttonH, int thickness, uint16_t color) {
  for (int i = 1; i <= thickness; i++) {
    tft.drawRoundRect(buttonX0-i, buttonY0-i, buttonW+i*2,buttonH+i*2, (min(buttonH, buttonW)/4), color);
  }
}

void updateButton(int position, int direction) {
  //tft.fillRoundRect(buttonCoords[position - direction][0] - 10,buttonCoords[position - direction][1] - 10,125+10*2,75+6*2,19,WHITE);
  //buttons[position - direction].drawButton();
  //if ((int)encoder->getDirection() != 0) {
  for (int i = 1; i <= 6; i++) {
    //tft.drawRoundRect(buttonCoords[position - direction][0] - i,buttonCoords[position - direction][1] - i,125+2*i,75+2*i,(min(75, 125)/4),WHITE);
  }
  //}
}

void drawSideMenu() {
  tft.drawFastVLine(71,38,282,WHITE);
  tft.drawFastVLine(72,38,282,WHITE);
  tft.drawFastVLine(73,38,282,WHITE);
  tft.drawFastVLine(74,38,282,WHITE);
  tft.drawFastVLine(75,38,282,WHITE);
  tft.drawFastVLine(76,38,282,WHITE);

  tft.drawFastHLine(0,32,480,WHITE);
  tft.drawFastHLine(0,33,480,WHITE);
  tft.drawFastHLine(0,34,480,WHITE);
  tft.drawFastHLine(0,35,480,WHITE);
  tft.drawFastHLine(0,36,480,WHITE);
  tft.drawFastHLine(0,37,480,WHITE);

  // Back Button
  tft.fillCircle(35,81,25,WHITE);
  tft.fillRect(37,74,12,14,DILUTBLU);
  tft.fillTriangle(17,81,37,68,37,94,DILUTBLU);

  tft.setCursor(11,110);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Back");

  // Home Button
  tft.fillCircle(35,169,25,WHITE); 
  tft.fillRoundRect(27,155,5,8,1,DILUTBLU);
  tft.fillTriangle(15,173,55,173,35,154,WHITE);
  tft.fillTriangle(18,173,52,173,35,156,DILUTBLU);
  tft.fillTriangle(23,173,47,173,35,161,WHITE);
  tft.fillRect(25,173,20,10,DILUTBLU); //Base
  tft.fillRect(33,176,4,7,WHITE);
  tft.fillTriangle(25,173,45,173,35,163,DILUTBLU);

  tft.setCursor(11,198);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Home");

  // Settings Button
  tft.fillCircle(35,257,25,WHITE);
  tft.setCursor(17,286);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Set");
}

void drawHome() {
  // Home Button
  tft.fillCircle(35,169,25,WHITE); 
  tft.fillRoundRect(27,155,5,8,1,DILUTBLU);
  tft.fillTriangle(15,173,55,173,35,154,WHITE);
  tft.fillTriangle(18,173,52,173,35,156,DILUTBLU);
  tft.fillTriangle(23,173,47,173,35,161,WHITE);
  tft.fillRect(25,173,20,10,DILUTBLU); //Base
  tft.fillRect(33,176,4,7,WHITE);
  tft.fillTriangle(25,173,45,173,35,163,DILUTBLU);

  tft.setCursor(11,198);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Home");
}

void drawHomeRed() { // draws a red home icon
  tft.fillCircle(35,169,25,WHITE); 
  tft.fillRoundRect(27,155,5,8,1,RED);
  tft.fillTriangle(15,173,55,173,35,154,WHITE);
  tft.fillTriangle(18,173,52,173,35,156,RED);
  tft.fillTriangle(23,173,47,173,35,161,WHITE);
  tft.fillRect(25,173,20,10,RED); //Base
  tft.fillRect(33,176,4,7,WHITE);
  tft.fillTriangle(25,173,45,173,35,163,RED);
  tft.setCursor(11,198);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.print("Home");
}

void drawBack() {
    // Back Button
  tft.fillCircle(35,81,25,WHITE);
  tft.fillRect(37,74,12,14,DILUTBLU);
  tft.fillTriangle(17,81,37,68,37,94,DILUTBLU);

  tft.setCursor(11,110);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Back");
}

void drawBackRed() {
    // Back Button
  tft.fillCircle(35,81,25,WHITE);
  tft.fillRect(37,74,12,14,RED);
  tft.fillTriangle(17,81,37,68,37,94,RED);

  tft.setCursor(11,110);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.print("Back");
}

void drawSet() {
  // Settings Button
  tft.fillCircle(35,257,25,WHITE);
  tft.setCursor(17,286);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Set");
}

void drawSetRed() {
  // Settings Button
  tft.fillCircle(35,257,25,WHITE);
  tft.setCursor(17,286);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.print("Set");
}

void flow() {
   flow_frequency++;
}
//GFXcanvas1 canvas(128, 32);

void push(int val) {
   if(top>=n-1) {

   }
   else {
      top++;
      stack[top]=val;
   }
}

void pop() {
   if(top<=-1) {

   }
   else {
      stack[top]=0;
      top--;
   }
}

void setup() {
  pinMode(dir,OUTPUT); //Use port registers for cleaner code
  pinMode(step,OUTPUT);

  pinMode(MOSFET,OUTPUT);

  pinMode(metalButton,INPUT);
  pinMode(metalButtonLED,OUTPUT);

  pinMode(flowSensor, INPUT);
  digitalWrite(flowSensor, HIGH);
  attachInterrupt(digitalPinToInterrupt(flowSensor), flow, RISING); // Setup Interrupt
  currentTime = millis();
  cloopTime = currentTime;

  motor.begin(rpm, 1);

  Serial.begin(9600);

  tft.reset();

  tft.begin(tft.readID());

  tft.setRotation(3);
  tft.fillScreen(DILUTBLU);
  drawSideMenu();

/*
  // Draw buttons
  buttons[0].initButtonUL(&tft,26,57,125,75,BLACK,BLUE,WHITE,buttonLabels[0],2);
  buttons[1].initButtonUL(&tft,(2*26+125),57,125,75,BLACK,BLUE,WHITE,buttonLabels[1],2);
  buttons[2].initButtonUL(&tft,(3*26+125*2),57,125,75,BLACK,BLUE,WHITE,buttonLabels[2],2);
  buttons[3].initButtonUL(&tft,77,(2*57+75),125,75,BLACK,BLUE,WHITE,buttonLabels[3],2);
  buttons[4].initButtonUL(&tft,(2*77+125),(2*57+75),125,75,BLACK,BLUE,WHITE,buttonLabels[4],2);

  buttons[0].drawButton();
  drawOutline(26,57,125,75,6,YELLOW);
  buttons[1].drawButton();
  buttons[2].drawButton();
  buttons[3].drawButton();
  buttons[4].drawButton();
  */

  // setup the rotary encoder functionality

  // use FOUR3 mode when PIN_IN1, PIN_IN2 signals are always HIGH in latch position.
  // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

  // use FOUR0 mode when PIN_IN1, PIN_IN2 signals are always LOW in latch position.
  // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR0);

  // use TWO03 mode when PIN_IN1, PIN_IN2 signals are both LOW or HIGH in latch position.
  encoder = new RotaryEncoder(encoderLeft, encoderRight, RotaryEncoder::LatchMode::FOUR3);

  attachInterrupt(digitalPinToInterrupt(encoderRight), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderLeft), checkPosition, CHANGE);
}

void loop() {

  currentTime = millis();

  TSPoint p = ts.getPoint();
  //f(p.z > ts.pressureThreshhold){         
    pixel_x = map(p.y,TSBot1,TSTop1,0,480);
    pixel_y = map(p.x, TSRight1,TSLeft1, 0, 320); 
  //}

  static int pos = 0;

  encoder->tick(); // just call tick() to check the state.

/*
  if(currentTime >= (cloopTime + 1000)) {
    cloopTime = currentTime; // Updates cloopTime
  }
  if(flow_frequency != 0){
    // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
    l_minute = (flow_frequency / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
    l_minute = l_minute/60;
    vol = vol +l_minute;
    flow_frequency = 0; // Reset Counter
    Serial.println("-----------------------------------------------");
    Serial.print("Flow rate per second:");
    Serial.print(l_minute, DEC); // Print litres/sec
    Serial.println(" L/Sec");
    Serial.println("-----------------------------------------------");
    l_minute = l_minute*60;
    Serial.print("Flow rate per minute:");
    Serial.print(l_minute, DEC); // Print litres/sec
    Serial.println(" L/min");
  }
  else {
    Serial.println(" flow rate = 0 ");
  }
  */
  
  int newPos = encoder->getPosition();
  if (pos != newPos) {
    refresh = true;
    pos = newPos;
  }

  static int screen = 1;

  if (dilution) {
    // Make seperate functions to draw each screen and to draw recurring elements
    // Consolidate the adafruit buttons and re initialize the same buttons on different screens
    // Make proper variables for UI parameters
    //TODO: add encoder support (outlines and selection)
    //TODO: add pump and valve functionality 
    if (pixel_x > 10 && pixel_x < 60 && pixel_y > 56 && pixel_y < 106) {
      if (top>-1) {
      screen = stack[top];
      pop();
      refresh = true;
      pixel_x = 0;
      pixel_y = 0;
      } else {
        drawBackRed();
        delay(50);
        drawBack();
      }
    }
    if (pixel_y > 144 && pixel_y < 194 && pixel_x > 10 && pixel_x < 60) {
        if (screen == 1) {
          drawHomeRed();
          delay(50);
          drawHome();
        } else {
          push(screen);
          screen = 1;
          refresh = true;
          pixel_x = 0;
          pixel_y = 0;
        }
    }
    if (pixel_y > 232 && pixel_y < 282 && pixel_x > 10 && pixel_x < 60) {
      if (screen == 7) {
        drawSetRed();
        delay(75);
        drawSet();
      } else {
        push(screen);
        screen = 7;
        refresh = true;
        pixel_x = 0;
        pixel_y = 0;
      }
    }
    if (screen == 1) {
      if(refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,300,30,DILUTBLU);
        tft.setCursor(20,5);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Main Menu");

        //memset(buttons, 0, sizeof(buttons));
        for (int i = 0; i<2; i++) {
          dButtons[i].initButtonUL(&tft,153,38+((i+1)*27+i*100),250,100,WHITE,0x04B9,WHITE,dButtonLabels[0][i],3);
          dButtons[i].drawButton();
        }
      }
      if (dButtons[0].contains(pixel_x,pixel_y)) {
        dButtons[0].drawButton(true);
        delay(75);
        push(screen);
        screen = 2;
        refresh = true;
        pixel_x = 0;
        pixel_y = 0;
      }
      if (dButtons[1].contains(pixel_x,pixel_y)) {
        dButtons[1].drawButton(true);
        delay(75);
        push(screen);
        screen = 3;
        refresh = true;
        pixel_x = 0;
        pixel_y = 0;
      }
    }
    if (screen == 2) {
      if (refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,300,30,DILUTBLU);
        tft.setCursor(20,5);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Dilution");
        
        pixel_x=0;
        pixel_y=0;

        for (int i = 0; i<2; i++) {
          dButtons[i].initButtonUL(&tft,153,38+((i+1)*27+i*100),250,100,WHITE,0x04B9,WHITE,dButtonLabels[1][i],3);
          dButtons[i].drawButton();
        }
      }
      if (dButtons[0].contains(pixel_x,pixel_y)) {
        dButtons[0].drawButton(true);
        delay(75);
        push(screen);
        screen = 4;
        refresh = true;
        pixel_x = 0;
        pixel_y = 0;
      }
      if (dButtons[1].contains(pixel_x,pixel_y)) {
        dButtons[1].drawButton(true);
        delay(75);
        push(screen);
        screen = 5;
        refresh = true;
        pixel_x = 0;
        pixel_y = 0;
      }
    } 
    if (screen == 3) {
      if (refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,200,30,DILUTBLU);
        tft.setCursor(20,5);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Statistics");
      }
    }     
    if (screen == 4) {
      if(refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,300,30,DILUTBLU);
        tft.setCursor(20,5);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Percentage of Dilution");

        //memset(buttons, 0, sizeof(buttons));
        for (int i = 0; i<2; i++) {
          dButtons[i].initButtonUL(&tft,153,38+((i+1)*27+i*100),250,100,WHITE,0x04B9,WHITE,dButtonLabels[3][i],3);
          dButtons[i].drawButton();
        }
      }
      if (dButtons[0].contains(pixel_x,pixel_y)) {
        push(screen);
        screen = 6;
        refresh = true;
        pixel_x = 0;
        pixel_y = 0;
      }
    }
    if (screen == 5) {
      if (refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,300,30,DILUTBLU);
        tft.setCursor(20,5);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Fill");

        for (int i = 0; i<3; i++) {
          dButtons[i].initButtonUL(&tft,153,40+(i*16+i*75),200,75,WHITE,0x04B9,WHITE,dButtonLabels[4][i],3);
          dButtons[i].drawButton();
        }
      }
      if (dButtons[0].contains(pixel_x,pixel_y)) {
        tft.setCursor(350,10);
        tft.setTextColor(BLACK);
        tft.setTextSize(2);
        tft.print("Filling");
      } else {
        tft.fillRect(300,0,180,30,DILUTBLU);
      }
    }

    if (screen == 6) {
      if (refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,400,30,DILUTBLU);
        tft.setCursor(20,10);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Custom Dilution Percentage");

        for (int i = 0; i<2; i++) {
          dButtons[i].initButtonUL(&tft,153,40+(i*16+i*75),200,75,WHITE,0x04B9,WHITE,dButtonLabels[5][i],3);
          dButtons[i].drawButton();
        }
      }
    }

    if (screen == 7) {
      if (refresh) {
        refresh = false;
        tft.fillRect(78,38,403,283,DILUTBLU);
        tft.fillRect(0,0,400,30,DILUTBLU);
        tft.setCursor(20,5);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Settings");

        for (int i = 0; i<2; i++) {
          dButtons[i].initButtonUL(&tft,153,40+(i*16+i*75),200,75,WHITE,0x04B9,WHITE,dButtonLabels[6][i],3);
          dButtons[i].drawButton();
        }
      }
    }
  }
}
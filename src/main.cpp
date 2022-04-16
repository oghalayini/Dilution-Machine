#include <Arduino.h>

#include <A4988.h>

#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include <Adafruit_TFTLCD.h>
#include <Adafruit_SPIDevice.h>
#include <Wire.h>
 
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

const int metalButton = 31;
const int metalButtonLED = 30;

const int xp = A12;
const int yp = A13;
const int xm = A14;
const int ym = A15;

const int steps = 200;
int rpm = 120;

bool refresh = true;
float dilution = 0.5;

int TSRight1 = 125; //Need better calibration
int TSLeft1 = 901;
int TSTop1 = 78;
int TSBot1 = 940;

int stack[20], n = 20, top = -1;

char dButtonLabels[7][4][15] = {
  {"Dilution", "Statistics"}, 
  {"Percent","Fill"}, 
  {"Percent","Fill"},
  {"2%", "1%","0.5%","Custom"},
  {"Hold to Fill","RPM + 10","RPM - 10"},
  {"Encoder","Touch"},
  {"Backlight","Sleep"}
};

char numButtonLabels[14][5] = {"1","2","3","4","5","6","7","8","9","Del","0",".","Rst","Sel"};

/******************* UI details */
int buttonX = 0; //Button Starting X
int buttonY = 0; //Button Starting Y
int buttonW = 0; //Button Width
int buttonH = 0; //Button Height
int buttonSX = 0; //Button Spacing X
int buttonSY = 0; //Button Spacing Y
int buttonTxt = 0; //Button Text Size
int menuHStart = 72;
int menuVStart = 32;
int menuThickness = 6;
int menuXEnd = menuHStart + menuThickness;
int menuYEnd = menuVStart + menuThickness;

int pixel_x;
int pixel_y;

bool sel = false;
bool dec = false;

static int screen = 1;

float number = 0;
float num1 = 0;

volatile int pulseCounter;
float volume = 0.0;
float qPerMin;
float qPerSec;
unsigned char flowSensor = 20;
unsigned long currentTime;
unsigned long loopTime;

A4988 motor = A4988(steps, dir, step, en);

Adafruit_TFTLCD tft(cs, cd, wr, rd, rst);

TouchScreen ts = TouchScreen(xp, yp, xm, ym, 285);

Adafruit_GFX_Button dButtons[4];

Adafruit_GFX_Button numButtons[14];

void flow () {
   pulseCounter++;
}

void drawSideMenu() {
  for (int i = 0; i < menuThickness; i++) {
    tft.drawFastVLine(i+menuHStart,menuVStart+menuThickness,282,WHITE);
    tft.drawFastHLine(0,i+menuVStart,480,WHITE);
  }

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

void drawHomeRed() {
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

void drawTopMenu(int screen) {
  tft.fillRect(0,0,480,31,DILUTBLU);
  tft.setCursor(20,5);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  switch (screen) {
    case 1:
      tft.print("Main Menu");
      break;
    case 2:
      tft.print("Dilution");
      break;
    case 3:
      tft.print("Statistics");
      break;
    case 4:
      tft.print("% of Dilution = ");
      tft.print(dilution);
      tft.print("%");
      break;
    case 5:
      tft.print("Fill");
      tft.setCursor(184,5);
      tft.print("RPM = ");
      tft.print(motor.getRPM());
      break;
    case 6:
      tft.setCursor(6,5);
      tft.print("Custom Dilution Percentage");
      break;
    case 7:
      tft.print("Settings");
      break;
  }
}

void getButtonDimensions(int screen) { //Continue the rest of the cases
  switch (screen)
  {
  case 1:
    /* code */
    break;
  case 4:
    buttonH = 96;
    buttonW = 82;
    buttonSX = 39; // 402 = 3W + 4SX
    buttonSY = 30; // 282 = 2H + 3SY
    buttonX = menuXEnd+buttonSX;
    buttonY = menuYEnd+buttonSY;
    buttonTxt = 3;
    break;
  case 6:
    buttonH = 40;
    buttonW = 68;
    buttonSY = 10; // 282 = 29*2 + 24 + 4H + 4SY
    buttonSX = 26; // 402 = 4W + 5SX
    buttonY = menuYEnd+29*2+24;
    buttonX = menuXEnd+buttonSX;
    buttonTxt = 3;
    break;
  default:
    break;
  }
}

void drawButtons(int screen) { // Try to make only 1 for loop with input to the function as the number of buttons and remove cases
  getButtonDimensions(screen);
  switch (screen)
  {
  case 4: {
    for (int i = 0; i < 3; i++) {
      dButtons[i].initButtonUL(&tft,buttonX+i*(buttonSX+buttonW),buttonY,buttonW,buttonH,WHITE,DILUTBLU,WHITE,dButtonLabels[screen-1][i],buttonTxt);
      dButtons[i].drawButton();
    }
    buttonSX = 76;
    buttonW = 250;
    dButtons[3].initButtonUL(&tft,menuXEnd+buttonSX,buttonY+buttonSY+buttonH,buttonW,buttonH,WHITE,DILUTBLU,WHITE,dButtonLabels[screen-1][3],buttonTxt);
    dButtons[3].drawButton();
    break;
  }
  case 6: {
    int counter = 0;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 3; j++) {
        numButtons[counter].initButtonUL(&tft,(j+1)*buttonSX+menuXEnd+j*buttonW,120+i*(buttonH+buttonSY),buttonW,buttonH,WHITE,DILUTBLU,WHITE,numButtonLabels[counter],buttonTxt);
        numButtons[counter].drawButton();
        counter++;
      }
    }
    for (int i = 0; i<2; i++) {
    numButtons[i+counter].initButtonUL(&tft,buttonW*3+buttonSX*4+menuXEnd,120+i*(2*(buttonH+buttonSY)),buttonW,buttonH*2+buttonSY,WHITE,DILUTBLU,WHITE,numButtonLabels[i+counter],3);
    numButtons[i+counter].drawButton();
    }
    break;
  }
  default: {
    break;
  }
  }
}

void detectButtons() { 
  if (numButtons[0].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 1;
    numButtons[0].drawButton(true);
    delay(100);
    numButtons[0].drawButton();
    sel = true;
  }
  if (numButtons[1].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 2;
    numButtons[1].drawButton(true);
    delay(100);
    numButtons[1].drawButton();
    sel = true;
  }
  if (numButtons[2].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 3;
    numButtons[2].drawButton(true);
    delay(100);
    numButtons[2].drawButton();
    sel = true;
  }
  if (numButtons[3].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 4;
    numButtons[3].drawButton(true);
    delay(100);
    numButtons[3].drawButton();
    sel = true;
  }
  if (numButtons[4].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 5;
    numButtons[4].drawButton(true);
    delay(100);
    numButtons[4].drawButton();
    sel = true;
  }
  if (numButtons[5].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 6;
    numButtons[5].drawButton(true);
    delay(100);
    numButtons[5].drawButton();
    sel = true;
  }
  if (numButtons[6].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 7;
    numButtons[6].drawButton(true);
    delay(100);
    numButtons[6].drawButton();
    sel = true;
  }
  if (numButtons[7].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 8;
    numButtons[7].drawButton(true);
    delay(100);
    numButtons[7].drawButton();
    sel = true;
  }
  if (numButtons[8].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 9;
    numButtons[8].drawButton(true);
    delay(100);
    numButtons[8].drawButton();
    sel = true;
  }
  if (numButtons[9].contains(pixel_x,pixel_y)) {
    number = num1;
    num1 = 0;
    numButtons[9].drawButton(true);
    delay(100);
    numButtons[9].drawButton();
    sel = true;
  }
  if (numButtons[10].contains(pixel_x,pixel_y)) {
    num1 = number;
    number = (number*10) + 0;
    numButtons[10].drawButton(true);
    delay(100);
    numButtons[10].drawButton();
    sel = true;
  }
  if (numButtons[11].contains(pixel_x,pixel_y)) {
    sel = true;
    dec = true;
    numButtons[11].drawButton(true);
    delay(100);
    numButtons[11].drawButton();
  }
  if (numButtons[12].contains(pixel_x,pixel_y)) {
    sel = true;
    dec = false;
    number = 0;
    numButtons[12].drawButton(true);
    delay(100);
    numButtons[12].drawButton();
  }
  if (numButtons[13].contains(pixel_x,pixel_y)) {
    sel = true;
    dec = false;
    dilution = number;
    numButtons[13].drawButton(true);
    delay(100);
    numButtons[13].drawButton();
    number = 0;
    screen = 4;
    refresh = true;
  }
}

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
  Serial.begin(9600);

  pinMode(dir,OUTPUT); //Use port registers for cleaner code
  pinMode(step,OUTPUT);
  pinMode(MOSFET,OUTPUT);
  pinMode(metalButton,INPUT);
  pinMode(metalButtonLED,OUTPUT);
  pinMode(flowSensor, INPUT);
  
  digitalWrite(flowSensor, HIGH);

  motor.begin(rpm, 1);
  motor.setEnableActiveState(LOW);
  motor.disable();

  tft.reset();
  tft.begin(tft.readID());
  tft.setRotation(3);
  tft.fillScreen(DILUTBLU);
  
  drawSideMenu();

  currentTime = millis();
  loopTime = currentTime;

  attachInterrupt(digitalPinToInterrupt(flowSensor), flow, RISING);
}

void loop() {

  currentTime = millis();
  if(currentTime >= (loopTime + 1000)) {
    loopTime = currentTime;
    if(pulseCounter != 0){
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      qPerMin = pulseCounter / 7.54; //7.5 (1)
      Serial.print("Rate: ");
      Serial.print(qPerMin);
      Serial.print(" L/min");
      qPerSec = qPerMin/60;
      Serial.print("\t");
      Serial.print(qPerSec);
      Serial.print(" L/sec");
      volume = volume + qPerSec;
      Serial.print("\t");
      Serial.print("Vol:");
      Serial.print(volume);
      Serial.println(" L");
      pulseCounter = 0; // Reset Counter
    }
    else {
      Serial.print("Flow rate = 0");
      Serial.print("\t");
      Serial.print("Vol:");
      Serial.print(volume);
      Serial.println(" L");
    }
   }

  TSPoint p = ts.getPoint();
  if (p.z > 10 && p.z < 1000) {    
    pixel_x = map(p.y,TSBot1,TSTop1,0,480);
    pixel_y = map(p.x, TSRight1,TSLeft1, 0, 320); 
  } 
  else {
    pixel_x = 0;
    pixel_y = 0;
    motor.startBrake();
    motor.disable();
  }
  
  // Make seperate functions to draw each screen and to draw recurring elements
  // Get water sensor and send message when disinfectant level is low
  // Make proper variables for UI parameters
  // TODO: add red button outline to dilution percentage screen to indicate dilution percentage is already chosen
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
    //Main Menu
    if(refresh) {
      refresh = false;
      tft.fillRect(78,38,403,283,DILUTBLU);
      drawTopMenu(screen);
      for (int i = 0; i<2; i++) {
        dButtons[i].initButtonUL(&tft,153,38+((i+1)*27+i*100),250,100,WHITE,DILUTBLU,WHITE,dButtonLabels[0][i],3);
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
    if (digitalRead(metalButton)) {
      motor.enable();
      
      motor.rotate(25);

      digitalWrite(MOSFET,HIGH);
    } else {
      digitalWrite(MOSFET,LOW);
  }
  }
  if (screen == 2) {
    //Dilution
    if (refresh) {
      refresh = false;
      tft.fillRect(78,38,403,283,DILUTBLU);
      drawTopMenu(screen);
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
    //Statistics
    if (refresh) {
      refresh = false;
      tft.fillRect(78,38,403,283,DILUTBLU);
      drawTopMenu(screen);
    }
  }     
  if (screen == 4) {
    //Percentage of Dilution
    if(refresh) {
      refresh = false;
      number = 0;
      num1 = 0;
      tft.fillRect(78,38,403,283,DILUTBLU);
      drawTopMenu(screen);
      drawButtons(screen);
    }
    if (dButtons[0].contains(pixel_x,pixel_y)) {
      if (dilution != 2) {
        dButtons[0].drawButton(true);
        delay(75);
        dButtons[0].drawButton();
        dilution = 2;
        tft.fillRect(308,0,108,32,DILUTBLU);
        tft.setCursor(308,5);
        tft.print(dilution);
        tft.print("%");
        pixel_x = 0;
        pixel_y = 0;
      }
    }
    if (dButtons[1].contains(pixel_x,pixel_y)) {
      if (dilution != 1) {
        dButtons[1].drawButton(true);
        delay(75);
        dButtons[1].drawButton();
        dilution = 1;
        tft.fillRect(308,0,108,32,DILUTBLU);
        tft.setCursor(308,5);
        tft.print(dilution);
        tft.print("%");
        pixel_x = 0;
        pixel_y = 0;
      }
    }
    if (dButtons[2].contains(pixel_x,pixel_y)) {
      if (dilution != 0.5) {
        dButtons[2].drawButton(true);
        delay(75);
        dButtons[2].drawButton();
        dilution = 0.5;
        tft.fillRect(308,0,108,32,DILUTBLU);
        tft.setCursor(308,5);
        tft.print(dilution);
        tft.print("%");
        pixel_x = 0;
        pixel_y = 0;
      }
    }
    if (dButtons[3].contains(pixel_x,pixel_y)) {
      dButtons[3].drawButton(true);
      delay(75);
      push(screen);
      screen = 6;
      refresh = true;
      pixel_x = 0;
      pixel_y = 0;
    }
  }
  if (screen == 5) {
    //Fill
    if (refresh) { 
      refresh = false;
      tft.fillRect(78,38,402,282,DILUTBLU);
      drawTopMenu(screen);
      for (int i = 0; i<3; i++) {
        dButtons[i].initButtonUL(&tft,153,40+(i*16+i*75),250,75,WHITE,0x04B9,WHITE,dButtonLabels[4][i],3);
        dButtons[i].drawButton();
      }
    }
    if (dButtons[0].contains(pixel_x,pixel_y)) {
      motor.enable();
      
      motor.rotate(25);
      //motor.nextAction();

      digitalWrite(MOSFET,HIGH);
    } else {
      digitalWrite(MOSFET,LOW);
    }
    if (dButtons[1].contains(pixel_x,pixel_y)) {
      motor.setRPM(rpm+10);
      rpm = motor.getRPM();
      tft.fillRect(92,0,480,31,DILUTBLU);
      tft.setCursor(184,5);
      tft.print("RPM = ");
      tft.print(motor.getRPM());
      delay(75);
    }
    if (dButtons[2].contains(pixel_x,pixel_y)) {
      motor.setRPM(rpm-10);
      rpm = motor.getRPM();
      tft.fillRect(92,0,480,31,DILUTBLU);
      tft.setCursor(184,5);
      tft.print("RPM = ");
      tft.print(motor.getRPM());
      delay(75);
    }
  }
  if (screen == 6) {
    //Custom Dilution Percentage
    if (refresh) {
      refresh = false;
      tft.fillRect(78,38,403,283,DILUTBLU);
      drawTopMenu(screen);
      tft.fillRect(197,61,90+34*2+6,30+6,BLACK);
      tft.fillRect(200,64,90+34*2,30,WHITE);
      tft.setCursor(234,67);
      tft.setTextColor(BLACK);
      tft.setTextSize(3);
      tft.print(dilution);
      tft.print("%");
      drawButtons(screen);
    }
    detectButtons();
    if (sel) {
      if (number < 100) {
        sel = false;
        tft.fillRect(200,64,90+34*2,30,WHITE);
        if (number < 10) {
          tft.setCursor(234,67);
          tft.setTextColor(BLACK);
          tft.setTextSize(3);
          tft.print(number);
          tft.print("%");
        } 
        else if (number >= 10) {
          tft.setCursor(225,67);
          tft.setTextColor(BLACK);
          tft.setTextSize(3);
          tft.print(number);
          tft.print("%");
        }
      } else {
        sel = false;
        tft.fillRect(200,64,90+34*2,30,WHITE);
        tft.setCursor(218,67);
        tft.setTextColor(RED);
        tft.setTextSize(3);
        tft.print("% Error");
      }
    }
  }
  if (screen == 7) {
    //Settings
    if (refresh) {
      refresh = false;
      tft.fillRect(78,38,403,283,DILUTBLU);
      drawTopMenu(screen);
      for (int i = 0; i<2; i++) {
        dButtons[i].initButtonUL(&tft,153,40+(i*16+i*75),200,75,WHITE,0x04B9,WHITE,dButtonLabels[6][i],3);
        dButtons[i].drawButton();
      }
    }
  }
}
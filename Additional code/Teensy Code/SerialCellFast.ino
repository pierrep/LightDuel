/*  OctoWS2811 Rainbow.ino - Rainbow Shifting Test
    http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
    Copyright (c) 2013 Paul Stoffregen, PJRC.COM, LLC

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.œ

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.


  Required Connections
  --------------------
    pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
    pin 14: LED strip #2    All 8 are the same length.
    pin 7:  LED strip #3
    pin 8:  LED strip #4    A 100 ohm resistor should used
    pin 6:  LED strip #5    between each Teensy pin and the
    pin 20: LED strip #6    wire to the LED strip, to minimize
    pin 21: LED strip #7    high frequency ringining & noise.
    pin 5:  LED strip #8
    pin 15 & 16 - Connect together, but do not use
    pin 4 - Do not use
    pin 3 - Do not use as PWM.  Normal use is ok.
    pin 1 - Output indicating CPU usage, monitor with an oscilloscope,
            logic analyzer or even an LED (brighter = CPU busier)
*/
/*
  Name    : SerialCell
  Author : Alistair Riddell
  DLM    : Mon 14 Dec 2015 13:41:40 AEDT
  Comments : Controlling a LED display with a USB Joystick

          Fri  6 Nov 2015 22:13:36 AEDT
          There have been some issues with this code and the Teensy3.1
          I've been experiementing with setup() in order to allow for the Odroid to create /dev/ttyACM0
          Moving some code around and introducing delays into setup() seems to help.
          I think code was executing too fast for the linux kernal at powerup of the Teensy.
          This might have caused the others to be bricked.

          Mon 23 Nov 2015 15:35:03 AEDT
          Need to review layout config. There's something wrong with the LED addressing.
          Fixed. Had to increase the LED id buffer size from 4 to 5. Probably here in this code, 4 bytes conversion using atoi, results in 3 byte values max.

          Thu 26 Nov 2015 12:57:03 AEDT
          This code is working for Odroid-X2+ using the program LEDdatasend.c (see comments there)
          More functionality could be added for the SELF joystick but not limited for that of the Thrustmaster

          Mon 14 Dec 2015 13:41:40 AEDT
          Change colour determination to initial call.

          Mon 28 Dec 2015 22:28:48 AEDT
          Added the Mondrian patterns. As yet untested.
          NOTE. Should also convert this to use FastLED library.ıı

          Thu  7 Jan 2016 21:20:46 AEDT
          Yesterday, converted to FastLED library and it works well.
          All button control works well here too.

          But... there is a problem with the serial reading of direct pixel control data with the new Mad Catz JS.
          The old version of this code works fine with the new JS. Still looking into the problem with this code.
          See problem area in loop() switch code.

          Switched to using FastLEDs random functions. Untested.

          Fri  8 Jan 2016 14:51:23 AEDT
          Previous problem with serial reading solved. Noted that there was a discrepency in the size of data written.
          Also, no longer sending colour data, just LED ids.

*/

#define USE_OCTOWS2811
#include<OctoWS2811.h>
#include<FastLED.h>

#define FALSE 0
#define TRUE 1

#define NUM_LEDS_PER_STRIP 180
#define NUM_STRIPS 8

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

const int ledsPerStrip = 180;
const int ledsPerSubStrip = 90; //  number of leds per segment of a strip. i.e. What is thought of as a strip
const int ledsPerLine = ledsPerStrip/2;
const int ledsPerLineEnd = ledsPerLine-1;
const int strips = 8;
const int lines = 16;

//DMAMEM int displayMemory[ledsPerStrip*6]; // Must initialize each strip with 6 integers/LED for RGB vals
//int drawingMemory[ledsPerStrip*6];

const int Mcolours[4] = {0xFF0000,0x0000FF,0xFFFF00, 0xFFFFFF};

const int config = WS2811_GRB | WS2811_800kHz;

//OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

unsigned int maxelements = ledsPerStrip*strips;
unsigned int i,j;
int up = 1;
int x = 0, y = 0;

unsigned int Lmatrix[lines][ledsPerLine]; //  This  is 16 x 90 and not based on Octo hardware config

int multiRows[lines];
int multiCols[lines];

int rxr[5] = {3,5,7,9,11};

int dim(int color, int);
int myColor();
void edge(int phaseShift, int cycleTime);
void centreCollapse(int, int);
void centreExplode(int, int);
void edgeSubMatrix(int phaseShift, int cycleTime);
void fillSubMatrix(int phaseShift, int cycleTime);
void vhlLine (int phaseShift, int cycleTime);
void vLine (int phaseShift, int cycleTime);
void hLine (int phaseShift, int cycleTime);
void lLine (int phaseShift, int cycleTime);
void rDiagonal (int phaseShift, int cycleTime);
void lDiagonal (int phaseShift, int cycleTime);
void plus ( int phaseShift, int cycleTime);
void xcross ( int phaseShift, int cycleTime);
void lessThan ( int phaseShift, int cycleTime);
void greaterThan ( int phaseShift, int cycleTime);
void pointLED(int cycleTime, int x, int y);
//void clusterLEDs();
void afterMondrian(int dur, int x[], int y[], int form);
void clearScreen ();

int color, hue = 0;
int led = 13;
boolean LedOn = FALSE;
int count = 0;
unsigned char  formBuf[4];
unsigned char  ledIdBuf[5];
unsigned char  buf[3];
int formId = 0;
unsigned int LEDiD = 0;
uint8_t dval = 0; // serial read val
uint8_t bval = 0; // serial read val
uint8_t cdim = 255; // control dim/bright val
unsigned char Rr, Gg, Bb;

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void setup()
 *  Description: initialize stuff
 * =====================================================================================
 */

void setup() {

  LEDS.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP);
/*
  LEDS.addLeds<NEOPIXEL, 2>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 5
  LEDS.addLeds<NEOPIXEL, 14>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 6
  LEDS.addLeds<NEOPIXEL, 7>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 7
  LEDS.addLeds<NEOPIXEL, 8>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 4
  LEDS.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 5
  LEDS.addLeds<NEOPIXEL, 20>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 6
  LEDS.addLeds<NEOPIXEL, 21>(leds, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 7
  LEDS.addLeds<NEOPIXEL, 5>(leds, NUM_LEDS_PER_STRIP);
*/
  clearScreen();
//  LEDS.setBrightness(0);
//  LEDS.show();
  LEDS.delay(500);

  pinMode(13, OUTPUT);

  randomSeed(analogRead(1));
  random16_set_seed (random16_get_seed ());

  digitalWrite(led, HIGH);
  delay(1000);
  Serial.begin(9600); // USB is always 12 Mbit/sec
  delay(500);

  while (i < maxelements) {
    for (y=0; y < ledsPerLine; y++ ) {
      Lmatrix[x][y] = i++;
     }
     x++;
     for (y=ledsPerLineEnd; y >= 0; y-- ) {
       Lmatrix[x][y] = i++;
     }
     x++;
  }
  delay(500);
  digitalWrite(led, LOW);

  delay(1000);
  color = dim(myColor(), random8(6,15));
  hue = 0;
}

/* For individual function tests

void loop() {
//  LEDS.setBrightness(100);

  for (unsigned int n = 0; n < random8(10); n++) { // generate patterns a few times.
      for(int i = 0; i< lines; i++){
           multiRows[i] = (int) random8(0,lines);
           multiCols[i] = (int) random8(0,ledsPerSubStrip);
      }

      afterMondrian(random(200000,5000000) / maxelements, multiRows, multiCols, (int) random8(2,7));
  }

//  edgeSubMatrix( 0, 2000); //
//  vhlLine(0, 2000); // "—" + "|" patterns
//  vLine(0, 2000); // "—" pattern
//  hLine(0, 2000); // "—" pattern
//  plus(0, 2000); // "+" pattern
//  lDiagonal(0, 2000); // "\"pattern
//  rDiagonal(0, 2000); // "/" pattern
//  greaterThan ( 0, 2000); // >
//  lessThan ( 0, 2000); // <
//  xcross ( 0, 2000);
//  fillSubMatrix(0,random8(1000,8000));
//  centreCollapse(0,random8(1000,3000));
//  centreExplode(0,random8(1000,3000)); // illuminate all of a submatrix
//  edge(0,1000);

  delay(500);
  clearScreen();
//  delay(1000);
}
*/

 /* ===  FUNCTION  ======================================================================
 *         Name:  void loop()
 *  Description: loop
 * =====================================================================================
 */

void loop() {
  if (Serial.available() > 0) {
    count = Serial.readBytes((char *)formBuf, 4);
    if (count != 4) {return;}
    if (count == 4) {
      formId = atoi((char *)formBuf);
    }

    switch (formId) {
     case 7000 : {
                    count = Serial.readBytes((char *)ledIdBuf, 1);
                    if (count != 1) {break;}
                    if (count == 1 ) {dval = atoi((char *)ledIdBuf);} // first val needs to be int, others unsigned char
                    cdim = dim8_raw(dval);
                    break;}
     case 7001 : {
                    count = Serial.readBytes((char *)ledIdBuf, 1);
                    if (count != 1) {break;}
                    if (count == 1 ) {bval = atoi((char *)ledIdBuf);} // first val needs to be int, others unsigned char
                    cdim = brighten8_raw(bval);
                    break;}
     case 8000 : {
                    count = Serial.readBytes((char *)ledIdBuf, 4);
                    if (count != 4) {break;}
                    if (count == 4) {LEDiD = atoi((char *)ledIdBuf);} // first val needs to be int, others unsigned char
                    leds[LEDiD] = CHSV((32*random8(0,8)) + hue+random8(0,NUM_LEDS_PER_STRIP),192,cdim); //Mcolours[2]; //(R,G,B);
                    LEDS.show();
                    break;}
      case 8001 : {
                    edge(0,1000); // illuminates the outer edge only
//                  digitalWrite(led, HIGH);
                    break;}
      case 8002 : {
                    fillSubMatrix (0,random16(1000,8000)); // "<" pattern
                    break;}
      case 8003 : {
                    lDiagonal(0, 2000); // "\" and "/" patterns
                    break;}
      case 8004 : {
                    edgeSubMatrix(0,2000); // illuminate edges of a sub matrix
//                  digitalWrite(led, HIGH);
                    break;}
      case 8005 : {
                    vLine(0, random16(1000, 8000)); // "|" and "—" patterns
//                  digitalWrite(led, HIGH);
                    break;}
      case 8006 : {
                    vhlLine(0, random16(1000, 8000)); // "|" and "—" patterns
//                  digitalWrite(led, HIGH);
                    break;}
      case 8007 : {
                    hLine(0, random16(1000, 8000)); // "|" and "—" patterns
//                  digitalWrite(led, HIGH);
                    break;}
      case 8008 : {
                    rDiagonal(0, 2000); // "\" and "/" patterns
//                  digitalWrite(led, HIGH);
                    break;}
      case 8009 : {
                    xcross ( 0, 2000); // "x" pattern
//                  digitalWrite(led, HIGH);
                    break;}
      case 8010 : {                  // Turn LEDs OFF
                    clearScreen();
                    break;}
      case 8011 : {
                    plus( 0, 2000); // "+" pattern
//                  digitalWrite(led, HIGH);
                    break;}
      case 8012 : {
                    lLine(0, random16(1000, 8000)); // "|" and "—" patterns
//                  digitalWrite(led, HIGH);
                    break;}
      case 8013 : {
                    centreExplode(0,random16(1000,4000)); // illuminate all of a submatrix
//                  digitalWrite(led, HIGH);
                    break;}
      case 8014 : {
                    centreCollapse(0,random16(1000,4000)); // illuminate all of a submatrix
//                  digitalWrite(led, HIGH);
                  break;}
      case 8015 : {
                    clearScreen();
                    for (unsigned int n = 0; n < random8(10); n++) { // generate patterns a few times.
                      for(int i = 0; i< lines; i++){
                        multiRows[i] = (int) random8(0,lines);
                        multiCols[i] = (int) random8(0,ledsPerSubStrip);
                      }

                      afterMondrian(random(200000,5000000) / maxelements, multiRows, multiCols, (int) random8(2,7));
                    }
//                  digitalWrite(led, HIGH);
                  break;}
   } // end  switch

//  delay(250);
//  digitalWrite(led, LOW);
  } // end of serial.available
} // end of loop()


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  dim
 *  Description: sets a brightness level less than max
 * =====================================================================================
 */

int dim(int color, int level){
  int r = color>>16;
  int g = (color>>8)&0xff;

  int b = (color&0xff);
  int i = 0;

#define rgbfade(r) r-=level; if (r<0)r=10; // should FADE_RATE be a variable proportional to the length of the LEDs turned on?

  for(i = 0; i < level; i++) {
    rgbfade(r);
    rgbfade(g);
    rgbfade(b);
  }
  return ((r&0xff)<<16) |  ((g&0xff)<<8) | (b&0xff);

}   /* -----  end of function dim()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  myColor
 *  Description: get a 24 bit color value
*
  probably want to use more definitive colours such as:

  #define RED    0xFF0000
  #define GREEN  0x00FF00
  #define BLUE   0x0000FF
  #define YELLOW 0xFFFF00
  #define PINK   0xFF1088
  #define ORANGE 0xE05800
  #define WHITE  0xFFFFFF

 * =====================================================================================
 */

//int mycolor(uint8_t red, uint8_t green, uint8_t blue) {
int myColor() {
int mainColor[13] = {0xFF0000,0x00FF00,0x0000FF,0xFFFF00,0xFF1088,0xE05800,0xFFFFFF,0xFF0000,0x00FF00,0x0000FF,0xFFFF00,0xFF1088,0xE05800};

  if(random8(100) > 50) {
    return (random8(0,255) << 16 | (random8(0,255) << 8) | random8(0,255));
  }
  else {
    return  (mainColor[random8(0,13)]);
  }
}   /* -----  end of function myColor()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  xcross
 *  Description: Finally works for the OCTO set up.
 * =====================================================================================
 */
void
clearScreen () {
  int max = NUM_STRIPS * NUM_LEDS_PER_STRIP;

    for (int y=0; y < max; y++) {
        leds[y] = 0;
      }
    LEDS.show();

}   /* -----  end of function clearScreen()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  clusterLEDs()
 *  Description: generates LED points that are clustered in a particular location
 * =====================================================================================
 *
void
clusterLEDs() {
int x, y, rx, ry, row, col, range = 0, cycleTime;

// test a random point
  x = random8(0,lines);
  y = random8(0,ledsPerLine);

// need to specify a point around which to cluster.

  while(!range){
      row = random8(lines);
      rx = (random8(7)) + 2;
      if((row+rx) < lines) range = 1;
  }
  range = 0;

  while(!range){
      col = random8(ledsPerLine);
      ry = (random8(11)) + 2;
      if((col+ry) < ledsPerLine) range = 1;
  }

  cycleTime = random16(2000,8000);
  pointLED( cycleTime, x, y);

}   // -----  end of function clusterLED()  ----- //
*/
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void pointLED(int cycleTime, int row, int col)
 *  Description: Activates individual LEDs at a given coordinate
 * =====================================================================================
 */

void
pointLED(int cycleTime, int row, int col) {
int x, y; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;
  if (row > lines) {
      x = random8(lines);
      y = random8(ledsPerLine);
  } else {
    x = row;
    y = col;
  }
//    color = dim(makeColor(random(0,359), random(0,100), random(0,100)), 0);
//  color = dim(myColor(), random(6,15));
//  leds(Lmatrix[x][y], color);
  leds[Lmatrix[x][y]] = color;
  LEDS.show();
  digitalWrite(led, LOW);
//  delayMicroseconds(wait);
//  if(random8(100) > 50) clearScreen();

}   /* -----  end of function pointLED()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  fillSubMatrix()
 *  Description: generates coords and fully illuminates sub matrix set
 * =====================================================================================
 */
void
fillSubMatrix(int phaseShift, int cycleTime) {
int x, y, row, rx, col, ry, range = 0; //, wait; //, color;

//  digitalWrite(led, HIGH);
//  wait = cycleTime * 2000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = (random8(8)) + 1;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = (random8(13)) + 1;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(makeColor(random(0,359), random(0,100), random(0,100)), 0);
//    color = dim(myColor(), max(rx,ry));
//    color = dim(myColor(), random(14,20));

    for (y=col; y < ry+col; y++) {
      for (x=row; x < row+(rx); x++) {
        leds[Lmatrix[x][y]] = color;
      }
    }
    LEDS.show();
    digitalWrite(led, LOW);
//    delayMicroseconds(wait);
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function fillSubMatrix()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  xcross
 *  Description:
 * =====================================================================================
 */
void
xcross ( int phaseShift, int cycleTime) {
int x, y, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = random8(7) + 3;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = random8(11) + 3;
      if((col+ry) < ledsPerLine) range = 1;
    }
    y = col;

//    color = dim(makeColor(random(0,359), random(0,100), random(0,100)), rx+ry);
//    color = dim(myColor(), max(rx,ry));

    for (x=row; x < row+rx; x++) { // diagonal to left
      leds[Lmatrix[x][y++]] = color;
    }
    y = col;
    for (x=row+(rx-1); x >= row; x--) { // diagonal to right
      leds[Lmatrix[x][y++]] = color;
    }

    LEDS.show();
    digitalWrite(led, LOW);
//    delayMicroseconds(wait);
//    if(random8(100) > 50) clearScreen();
//    leds.begin();

}   /* -----  end of function xcross  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  greaterThan
 *  Description:
 * =====================================================================================
 */
void
greaterThan ( int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = random8(7) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = random8(11) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (x=row; x < row+rx; x++) { // diagonal to left
 //     color = random(0,180);
 //     index = (color + row + col*phaseShift/2) % 180;
      leds[Lmatrix[x][col++]] = color;
    }

    for (x=row+rx; x >= row; x--) { // diagonal to right
//      color = random(0,180);
//      index = (color + row + col*phaseShift/2) % 180;
      leds[Lmatrix[x][col++]] = color;
    }

    LEDS.show();
    digitalWrite(led, LOW);
//    delayMicroseconds(wait);
//    if(random8(100) > 50) clearScreen();
//    leds.begin();

}   /* -----  end of function lessThan  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  lessThan
 *  Description:
 * =====================================================================================
 */
void
lessThan ( int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = random8(7) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = random8(11) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (x=row+rx; x > row; x--) { // diagonal to left
      leds[Lmatrix[x][col++]] = color;
    }

    for (x=row; x <= row+rx; x++) { // diagonal to right
      leds[Lmatrix[x][col++]] = color;
    }

    LEDS.show();
    digitalWrite(led, LOW);
//    delayMicroseconds(wait);
//    if(random8(100) > 50) clearScreen();
//    leds.begin();

}   /* -----  end of function greaterThan  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  rDiagonal
 *  Description:
 * =====================================================================================
 */
void
rDiagonal ( int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = random8(9) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = random8(13) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (x=row+rx; x > row; x--) { // diagonal to right
      leds[Lmatrix[x][col++]] = color;
    }

    LEDS.show();
//    digitalWrite(led, LOW);
//    delayMicroseconds(wait);
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function rDiagonal  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  lDiagonal
 *  Description:
 * =====================================================================================
 */
void
lDiagonal ( int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = random8(9) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = random8(13) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (x=row; x < row+rx; x++) { // diagonal to left
      leds[Lmatrix[x][col++]] = color;
    }

    LEDS.show();
//    digitalWrite(led, LOW);
//    delayMicroseconds(wait);
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function lDiagonal  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  plus
 *  Description:
 * =====================================================================================
 */
void
plus ( int phaseShift, int cycleTime) {
int x, y, row, rx, col; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

      rx = rxr[random8(5)];
      row = random8((lines-rx)+1);
      col = random8(rx/2,ledsPerLine-rx);

//    color = dim(myColor(), random(6,15));

    for (x=row; x < row+(rx); x++) { // hoizontal
       leds[Lmatrix[x][col]] = color;
    }
    x = row;
    x = x + (rx/2);

    for (y=col; y < col +rx; y++) { // verticle
        leds[Lmatrix[x][y-(rx/2)]] = color;
    }

    LEDS.show();
//    digitalWrite(led, LOW);
//    delayMicroseconds(wait);

//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function plus  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  hLine()
 *  Description: generates coords for a horizontal vector
 * =====================================================================================
 */
void
hLine(int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = (random8(9)) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = (random8(13)) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (x=row; x < row+rx; x++) {
       leds[Lmatrix[x][col]] = color;
    }

    LEDS.show();
//    digitalWrite(led, LOW);
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function hLine()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  vhLine()
 *  Description: generates coords for a verticle vector
 * =====================================================================================
 */
void
vLine(int phaseShift, int cycleTime) {
int row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = (random8(9)) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = (random8(13)) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (y=col; y < ry+col; y++) {
      leds[Lmatrix[row][y]] = color;
    }

    LEDS.show();
//    digitalWrite(led, LOW);
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function vLine()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  lLine()
 *  Description: generates coords for a verticle or horizontal vector
 * =====================================================================================
 */
void
lLine(int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = (random8(7)) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = (random8(11)) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    for (x=row; x < row+rx; x++) {
      leds[Lmatrix[x][col]] = color;
    }
    if (random(100)> 50)row = row  + rx;
    for (y=col; y < ry+col; y++) {
      leds[Lmatrix[row][y]] = color;
    }

    LEDS.show();
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function lLine()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  vhlLine()
 *  Description: generates coords for a verticle or horizontal vector
 * =====================================================================================
 */
void
vhlLine(int phaseShift, int cycleTime) {
int x, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

    while(!range){
      row = random8(lines);
      rx = (random8(9)) + 2;
      if((row+rx) < lines) range = 1;
    }
    range = 0;

    while(!range){
      col = random8(ledsPerLine);
      ry = (random8(13)) + 2;
      if((col+ry) < ledsPerLine) range = 1;
    }

//    color = dim(myColor(), random(6,15));

    switch(random(3)) {
      case 0 : {
        for (x=row; x < row+rx; x++) {
          leds[Lmatrix[x][col]] = color;
        }
        break;
      }
      case 1 : {
        for (y=col; y < ry+col; y++) {
          leds[Lmatrix[row][y]] = color;
        }
        break;
      }
      case 2 : {
        for (x=row; x < row+rx; x++) {
          leds[Lmatrix[x][col]] = color;
        }
        if (random8(100)> 50)row = row  + rx;
        for (y=col; y < ry+col; y++) {
          leds[Lmatrix[row][y]] = color;
        }
        break;
      }
    }

    LEDS.show();
//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function vhlLine()  ----- */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  edgeSubMatrix()
 *  Description: generates coords for matrix set
 * =====================================================================================
 */

void
edgeSubMatrix(int phaseShift, int cycleTime) {
int x, y, row, rx, col, ry, range = 0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

  while(!range){
    row = (random8(lines));
    rx = (random8(7) + 2);
    if((row+rx) < lines) range = 1;
  }
  range = 0;

  while(!range){
    col = random8(ledsPerLine);
    ry = (random8(11)) + 2;
    if((col+ry) < ledsPerLine) range = 1;
  }

//  color = dim(myColor(), random(6,15));

  for (y=col; y < ry+col; y++) {
    leds[Lmatrix[row][y]] = color;
  }

  for (x=row+1; x < row+(rx); x++) {
    leds[Lmatrix[x][col+(ry-1)]] = color;
  }

  for (y=col+(ry-1); y > col; y--) {
    leds[Lmatrix[row+(rx-1)][y-1]] = color;
  }

  for (x=row+(rx-1); x > row+1; x--) {
    leds[Lmatrix[x-1][col]] = color;
  }

  LEDS.show();

//    if(random8(100) > 50) clearScreen();

}   /* -----  end of function edgeSubMatrix()  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void centreExplode(int phaseShift, int cycleTime)
 *  Description: Illuminates successive contentric squares from the outside in
 * =====================================================================================
 */

void
centreCollapse(int phaseShift, int cycleTime) {
int x=0, y=0, i, z, j = 0, wait; //, color;

  digitalWrite(led, HIGH);
  wait = cycleTime * 2000 / ledsPerStrip;

//  color = dim(myColor(), random(6,15));

  for (i=0, z=0; i<8; i++, z = z + 6) {

// Light LEFT edge looking at large LED screen
    color = dim(myColor(), random8(6,15));
    for (y=0; y < ledsPerLine; y++) {
      j = i;
//      if (i == 7) j = 8;
      leds[Lmatrix[j][y]] = color;
    }

// Light BOTTOM edge
    color = dim(myColor(), random8(6,15));
    for (x=1; x < lines; x++) {
      if (i == 7) z = 44;
      leds[Lmatrix[x][ledsPerLineEnd-z]] = color;
    }

// Light RIGHT edge
    color = dim(myColor(), random8(6,15));
    for (y=ledsPerLineEnd; y >= 0; y--) {
      j = i;
//      if (i == 7) j = 8;
      leds[Lmatrix[15-j][y]] = color;
    }

// Light TOP edge
    color = dim(myColor(), random8(6,15));
    for (x=14; x > 0 ; x--) {
      if (i == 7) z = 43;
      leds[Lmatrix[x][z]] = color;
    }

    LEDS.show();
    digitalWrite(led, LOW);
    delayMicroseconds(wait);


// Light LEFT edge looking at large LED screen
    color = dim(myColor(), random8(6,15));
    for (y=0; y < ledsPerLine; y++) {
      j = i;
//      if (i == 7) j = 8;
      leds[Lmatrix[j][y]] = 0;
    }

// Light BOTTOM edge
    color = dim(myColor(), random8(6,15));
    for (x=1; x < lines; x++) {
      if (i == 7) z = 44;
      leds[Lmatrix[x][ledsPerLineEnd-z]] = 0;
    }

// Light RIGHT edge
    color = dim(myColor(), random8(6,15));
    for (y=ledsPerLineEnd; y >= 0; y--) {
      j = i;
//      if (i == 7) j = 8;
      leds[Lmatrix[15-j][y]] = 0;
    }

// Light TOP edge
    color = dim(myColor(), random8(6,15));
    for (x=14; x > 0 ; x--) {
      if (i == 7) z = 43;
      leds[Lmatrix[x][z]] = 0;
    }

  }
//    if(random8(100) > 50) { clearScreen(); }
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void centreExplode(int phaseShift, int cycleTime)
 *  Description: Illuminates successive contentric squares from the inside out
 * =====================================================================================
 */

void
centreExplode(int phaseShift, int cycleTime) {
int x=0, y=0, i, z=0, q, j = 0, wait; //, color;

  digitalWrite(led, HIGH);
  wait = cycleTime * 2000 / ledsPerStrip;

  color = dim(myColor(), random8(6,15));

// Light Concentric squares
  for (i=8, z=0; i>0; i--, z = z + 6) {

// Light Right edge
    for (y=0; y < ledsPerLine; y++) {
      j = i-1;
      leds[Lmatrix[j][y]] = color;
    }

// Light Top edge
    for (x=0; x < lines; x++) {
      if (i == 8) z = 44;
      if (i == 1) {z = ledsPerLineEnd;}
      leds[Lmatrix[x][ledsPerLineEnd-z]] = color;
    }

// Light Left edge
    for (y=ledsPerLine; y >= 0; y--) {
      j = i-1;
      if (i == 8) j = 7;
     leds[Lmatrix[15-j][y]] = color;
    }

// Light Bottom edge
    for (x=14; x > 0 ; x--) {
      if (i == 8) q = 45;
      else q = z;
      leds[Lmatrix[x][q]] = color;
    }

    LEDS.show();
    digitalWrite(led, LOW);
    delayMicroseconds(wait);

// Light Right edge
    for (y=0; y < ledsPerLine; y++) {
      j = i-1;
      leds[Lmatrix[j][y]] = 0;
    }

// Light Top edge
    for (x=0; x < lines; x++) {
      if (i == 8) z = 44;
      if (i == 1) {z = ledsPerLineEnd;}
      leds[Lmatrix[x][ledsPerLineEnd-z]] = 0;
    }

// Light Left edge
    for (y=ledsPerLine; y >= 0; y--) {
      j = i-1;
      if (i == 8) j = 7;
     leds[Lmatrix[15-j][y]] = 0;
    }

// Light Bottom edge
    for (x=14; x > 0 ; x--) {
      if (i == 8) q = 45;
      else q = z;
      leds[Lmatrix[x][q]] = 0;
    }
//    LEDS.show();
//    delay(50);

//    if(random8(100) > 50) { clearScreen(); }
  }
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void edge(int phaseShift, int cycleTime)
 *  Description: Illuminates the matrix outer edge
 * =====================================================================================
 */

void
edge(int phaseShift, int cycleTime) {
int x=0, y=0; //, color;

  digitalWrite(led, HIGH);
//  wait = cycleTime * 4000 / ledsPerStrip;

  color = dim(myColor(), random8(6,15));

// Light Right edge
  for (y=0; y < ledsPerLine; y++) {
    leds[Lmatrix[0][y]] = color;
  }

// Light Top edge
  for (x=1; x < lines; x++) {
    leds[Lmatrix[x][ledsPerLineEnd]] = color;
  }

// Light Left edge
  for (y=ledsPerLine; y >= 0; y--) {
    leds[Lmatrix[lines-1][y]] = color;
  }

// Light Bottom edge
  for (x=14; x >= 0 ; x--) {
    leds[Lmatrix[x][0]] = color;
  }

  LEDS.show();
  digitalWrite(led, LOW);
}

/*
 * ===  FUNCTION  ===============================================================================
 *         Name:  void afterMondrian(int dur, int x[], int y[], int form)
 *  Description: Generates Mondrian like light patterns but background is black instead of white
 *               This might need to run multiple times to be effective
 * ==============================================================================================
 */

void afterMondrian(int dur, int x[], int y[], int form) { // Simultaneous Lines
int b, c, m1, m2;

  for (int q = 0; q < form; q++){
//    if (col_change > 1) {
      m1 = random8(0,16)/4;
      m2 = random8(0,16)/4;

// Row Y ON
    b = random8(8,ledsPerSubStrip);
    c = random8(0,b);

    for (int j = c; j<b; j++) {
//     color2 = leds.getPixel(i + (204 * y));
//      coverage=PCOVERAGE(j*j*j);
//      color=scaleColor( Mcolours[m1], coverage);
      leds[Lmatrix[x[q]][j]] = Mcolours[m1];
    }

// Row X ON
    for (int j = random8(0,4); j<lines; j++) {
//      coverage=PCOVERAGE(j*j*j);
//      color=scaleColor( Mcolours[m2], coverage);
      leds[Lmatrix[j][y[q]]] = Mcolours[m2];
//      leds.setPixel(ledsMatrix[j][y[q]], R2,G2,B2);
    }
  }
  LEDS.show();
}

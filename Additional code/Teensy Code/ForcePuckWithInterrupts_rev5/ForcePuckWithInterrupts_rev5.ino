#define USE_OCTOWS2811
#include<OctoWS2811.h>
#include <FastLED.h>
#include "animations.h"
#include "definitions.h"

// function prototypes
extern void roundWinnerAnimation(int , CRGB *, CRGB *, CRGB * );
extern void idleAnimation(int *, CRGB *, CRGB *, CRGB *, CRGB *);
void initialiseGame(void);
void fadeStrips(int);
void fadeRings(void);
void drawBall(void);
int makeColor(unsigned int, unsigned int, unsigned int);
unsigned int h2rgb(unsigned int, unsigned int, unsigned int);
void flashButton (int);
void illuminateZone(int);

// Game states
enum state{
  idling,
  playing,
  won
} gameState;

// Strips
CRGB leds1[STRIP_LEN];
CRGB leds2[STRIP_LEN];

// Rings
CRGB ledRing1[RING_LEN];
CRGB ledRing2[RING_LEN];
CRGB led_off_colour = 0x000000;
CRGB zoneColour = 0xFFFFFF;

// Ball variables
volatile int ball_dir = -1; //1 or -1
volatile int ball_pos = STRIP_LEN/2;
volatile boolean ball_running = false;

int ball_speed = LEVEL1;
int zoneOn = 0;
int animation = 0;
CRGB ball_colour = 0x00FF00; // red
int shift = 0;

int rainbowColors[180]; //array of colours for idle animation

int playerScore[2] = {0,0}; //array to keep score

/*****************************************
                    setup
******************************************/

void setup() {

  Serial.begin(9600); // USB is always 12 Mbit/sec

// set onboard teensy LED to output for status update if required

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);   // turn the setup LED on

// set up button interrupts

  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  attachInterrupt(BUTTON_1, button_1_ISR, RISING);
  attachInterrupt(BUTTON_2, button_2_ISR, RISING);

// generate rainbow colours for idle animation

  for (int i=0; i<180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;

// pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }

  //initialise LED strips

  FastLED.addLeds<CHIPSET, LED_PIN1, COLOR_ORDER>(leds1, STRIP_LEN).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, LED_PIN2, COLOR_ORDER>(leds2, STRIP_LEN).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, LED_RING_PIN1, COLOR_ORDER>(ledRing1, RING_LEN).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, LED_RING_PIN2, COLOR_ORDER>(ledRing2, RING_LEN).setCorrection( TypicalLEDStrip );

  FastLED.show(); // Initialize all pixels to 'off'
  FastLED.setBrightness( BRIGHTNESS );

  //set game state to idling

  gameState = idling;
  digitalWrite(LED, LOW);   // turn the setup LED off
}

/*****************************************
                    loop
******************************************/

void loop() {

// Initialise game variables

  initialiseGame();
  int hit_cnt = 0;

// Idle state. Remain in idle state while no button has been pressed

  button1_pressed = 0;
  button2_pressed = 0;

  while(gameState == idling) {
    idleAnimation(rainbowColors, leds1, leds2, ledRing1, ledRing2);
  }

// Begin game

  while(playerScore[0] < 3 && playerScore[1] < 3) { //keep playing rounds while neither player has scored more than 3
    while(ball_running) {
      playGame();
      if (hit_cnt++ == 5) {ball_speed = LEVEL2;} // volley has reached time for faster puck speed
    }

// Need to add code for acknowledging the overall winner

    delay(1000); //wait for 2 seconds after round is over before re-serving...might be too long?;
    ball_running = true; //re-serve ball
    ball_speed = LEVEL1; // start game at entry level speed.
    hit_cnt = 0;
  }
}

/**************************
//core game functionality
**************************/
void playGame(void) {

//if ball is running
  if(ball_running) {

    if (button1_pressed) {
      Serial.write(1);
      Serial.send_now();
      button1_pressed = 0;
    }
   if (button2_pressed) {
      Serial.write(2);
      Serial.send_now();
      button2_pressed = 0;
    }

    // progress ball position according to direction
    illuminateZone(ball_dir);
    if(ball_dir > 0) {
//     if (!zoneOn) {illuminateZone(ball_dir); zoneOn = 1;}
     ball_colour = 0xFF0000;
      ball_pos += (ball_speed - shift);
      if (ball_pos > STRIP_LEN*2/3) {shift = 1;}
    } else {
//    if (!zoneOn) {illuminateZone(ball_dir); zoneOn = 1;}
      ball_colour = 0x0000FF;
      ball_pos -= (ball_speed - shift);
      if (ball_pos < STRIP_LEN/3) {shift = 1;}
    }

//If the ball runs off the end of the strip without getting hit, increment score, and play round won animation
    if(ball_pos >= STRIP_LEN) {
      Serial.write(3);
      Serial.send_now();
      ball_running = false; //set ball running flag to false, so that buttons don't do anything
      playerScore[1]++; // Other players score incremented
      ball_pos = 0; //set ball to serve from winner's end
      ball_dir = 1;
      shift = 0;
      roundWinnerAnimation(0, leds1, leds2, ledRing1); //play round winner animation
      fadeStrips(ball_dir);
      fadeRings();
    } else if( ball_pos <= 0) {
      Serial.write(4);
      Serial.send_now();
      ball_running = false; //set ball running flag to false, so that buttons don't do anything
      playerScore[0]++;  // Other players score incremented
      ball_pos = STRIP_LEN; //set ball to serve from winner's end
      ball_dir = -1;
      roundWinnerAnimation(1, leds1, leds2, ledRing2); //play round winner animation
      shift = 0;
      fadeStrips(ball_dir);
      fadeRings();
    } else {
      //otherwise if ball is still in play, draw the ball
      fadeStrips(ball_dir);
      fadeRings();
      drawBall();
      FastLED.show(BRIGHTNESS);
    }
  } // end if (ball_running)
} // end loop

/*******************************************************************
//fade function. scales all LED's in strips in both strips by n/255
********************************************************************/
inline void fadeStrips(int end) {
    if  (end == 1) {
    for(int i = 0; i < STRIP_LEN - ZONE_LEN; i++) {
      leds2[i] = leds1[i].nscale8(200);
    }
    } else {
    for(int i = ZONE_LEN; i < STRIP_LEN; i++) {
      leds2[i] = leds1[i].nscale8(200);
    }
  }
}

inline void fadeRings(void) {
  for(int i = 0; i < RING_LEN; i++) {
    ledRing1[i].nscale8(100);
    ledRing2[i].nscale8(100);
    }
}

void illuminateZone(int end) {

      for(int i = STRIP_LEN - ZONE_LEN ; i < STRIP_LEN; i++) { leds2[i] = leds1[i] = zoneColour; }
      for(int i = 0; i < ZONE_LEN; i++) { leds2[i] = leds1[i] = zoneColour; }
 }

/**********************************
//draw the ball @ current position
**********************************/
inline void drawBall(void) {
    leds2[ball_pos] = leds1[ball_pos] = ball_colour;
    if(ball_dir > 0) {
      for (int i = 1; i < ball_speed; i++) {leds2[ball_pos-i] = leds1[ball_pos-i] = ball_colour; }
    } else {
      for (int i = 1; i < ball_speed; i++) {leds2[ball_pos+i] = leds1[ball_pos+i] = ball_colour; }
    }
}

/***************************
//Button interrupt routines
***************************/

void button_1_ISR() {

// if ball is within the zone and ball is running, it's a hit so change direction.

  if((ball_pos < ZONE_LEN) && (ball_dir<0) && ball_running) {
      ball_dir = -ball_dir;
  } else if (gameState == idling) {
      ball_pos = 0;
      ball_dir = 1;
      ball_running = true;
      gameState = playing;
  } else if (ball_running) {
      for(int i = 0; i < RING_LEN; i++) { ledRing1[i] = 0xFF0000; }
  }
  button1_pressed = 1;
}

void button_2_ISR() {

  if((ball_pos >= (STRIP_LEN - ZONE_LEN)) && (ball_pos < STRIP_LEN) && (ball_dir>0) && ball_running) {
      ball_dir = -ball_dir;
  } else if (gameState == idling) {
      ball_pos = STRIP_LEN;
      ball_dir = -1;
      ball_running = true;
      gameState = playing;
  } else if (ball_running) {
      for(int i = 0; i < RING_LEN; i++) { ledRing2[i] = 0x0000FF; }
  }
  button2_pressed = 1;
}

/**********************************************************************
Generate fastLED colour from supplied hue, sat, and lightness values
used to generate rainbow colours for idle animation.
**********************************************************************/
int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness)
{
  unsigned int red, green, blue;
  unsigned int var1, var2;

  if (hue > 359) hue = hue % 360;
  if (saturation > 100) saturation = 100;
  if (lightness > 100) lightness = 100;


  if (saturation == 0) {
    red = green = blue = lightness * 255 / 100;
  } else {
    if (lightness < 50) {
      var2 = lightness * (100 + saturation);
    } else {
      var2 = ((lightness + saturation) * 100) - (saturation * lightness);
    }
    var1 = lightness * 200 - var2;
    red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
    green = h2rgb(var1, var2, hue) * 255 / 600000;
    blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
  }
  return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
  if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
  if (hue < 180) return v2 * 60;
  if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
  return v1 * 60;
}

/*****************************
//clear scores and LED strips
*****************************/
void initialiseGame(void) {
  gameState = idling; //set gamestate to idling
  playerScore[0] = playerScore[1] = 0; //reset player scores
  FastLED.clear(); //clear LED's
  FastLED.show();
}

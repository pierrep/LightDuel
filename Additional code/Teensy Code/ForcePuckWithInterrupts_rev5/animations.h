#include "definitions.h"


volatile int button1_pressed = 0;
volatile int button2_pressed = 0;

/**************************************
//play an animation when a round is won
**************************************/

void roundWinnerAnimation(int playerNumber, CRGB *leds1, CRGB *leds2, CRGB *ring) {

  FastLED.clear(); //clear strip
  FastLED.show();
  bool flash = 1; //bit to indicate whether to turn LED's on or off. inverted below.
  CRGB colour = 0xFF0000; //initial animation colour
  int numFlashes = 10;

  //if player 0 wins, incrementally light up led's from player 0 end to end of strip, then pulse LED's three times
  switch(playerNumber) {
    case 0: {
            Serial.write(5); // This player wins
            Serial.send_now();
            // pulse all the LED's numFlashes times
            for(int k=0; k<numFlashes; k++) {
              delay(150); //200ms delay between flashes
              flash ^= 1;
              if(flash == true) { colour = 0xFF0000; } else { colour = 0x000000;}
              for(int j=0; j<STRIP_LEN/2; j++) { leds1[j] = leds2[j] = ring[j % RING_LEN] = colour;} //set led to off
              FastLED.show();
            }
            delay(150);
            FastLED.clear(); //clear strip
            FastLED.show();
            break;
            }
    //if player 1 wins, incrementally light up led's from player 1 end to end of strip, then pulse LED's three times
    case 1: {
            Serial.write(6); // This player wins
            Serial.send_now();
             // pulse half the strip numFlashes times
            for(int k=0; k<numFlashes; k++) {
              delay(150); //200ms delay between flashes
              flash ^= 1;
              if(flash == true) { colour = 0x0000FF;} else { colour = 0x000000;}
              for(int j=STRIP_LEN - 1; j>=STRIP_LEN/2; j--) { leds1[j] = leds2[j] = ring[j % RING_LEN] = colour; } //change LED colour
              FastLED.show();
            }
            delay(150);
            FastLED.clear(); //clear strip
            FastLED.show();
            break;
            }
    break;
  }
}

/*****************************************
//play an animation before the game starts
*****************************************/

void idleAnimation(int *rainbowArray, CRGB *strip1, CRGB *strip2, CRGB *ring1, CRGB *ring2) {
 int color, x, y, wait;
 int phaseShift = 10;
 int cycleTime = 500; //time to cycle through all the colours

  wait = cycleTime * 1000 / STRIP_LEN;
  for (color=0; (color < 180) && (button1_pressed == 0) && (button2_pressed == 0); color++) { // want to break out of this as soon as a button is pressed
    for (x=0; (x < STRIP_LEN)  && (button1_pressed == 0) && (button2_pressed == 0); x++) {
      for (y=0; y < 8; y++) {
        int index = (color + x + y * phaseShift/2) % 180;
        //leds.setPixel(x + y*ledsPerStrip, rainbowColors[index]);
        strip1[x] = strip2[x] = ring1[x % RING_LEN] = ring2[x % RING_LEN] = rainbowArray[index];
      }
    }
    FastLED.show();
    delayMicroseconds(wait);
  }
}

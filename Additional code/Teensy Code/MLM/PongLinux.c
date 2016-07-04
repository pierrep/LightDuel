/*
 * Title: PongLinux
 *
 * Author: Alistair Riddell
 *
 * DoC: Mon May 23 16:27:25 AEST 2016
 *
 * DLM: 
 *
 * SETUP
 *
 * USB installation (should be automatic, see below)
 *
 * COMMENTS:
 *
 * 		NOTE: When using two Teensys with this code, the chip serial numbers have to be hard coded into the program.
 *
 *		Start importing game code from Teensy only version
 *		All USB I/O is only with the two Teensys
 *		Finish with a BIG code clean up!
 *
 * 	Tue May 24 17:31:37 AEST 2016
 * 		Data to be sent to the Teensys with behaviour IDs. That is, for game behaviour and puck movement. This should minimise USB data transfers.
 * 		Button data to be read by each Thread but only if the puck direction and position are correct at the time. Otherwise, button data to be ignored.
 * 		Puck data to be sent as start led position and colour. The Teensy will then draw the whole puck display with fading...hopefully.
 *
*/

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <usb.h>
#include <libudev.h>
#include <locale.h>
#include <inttypes.h>
#include "liblo-0.28/lo/lo.h"

#define DEVICE_CNT 1 //2

char *udevnames[]  = {"ttyACM0", "ttyACM1"};
//const char *serial_nums[]  = {"828800", "1369580"}; // Serial numbers of other Teensys
//const char *serial_nums[]  = {"462150", "532430"}; // Serial numbers of Teensys of other Teensys
//const char *serial_nums[]  = {"461880", "532430"}; // Serial numbers of Teensys used with light canvas

//const char *serial_nums[]  = {"828800"}; //, "532430"}; // Serial numbers of Teensys of other Teensy config
const char *serial_nums[]  = {"1368340"}; //, "532430"}; // Serial numbers of Teensys of other Teensy config

const char *serial_num;
const char *dev_node;

#define OFFSET 5
#define FALSE 0
#define TRUE 1
#define IDLETIME 1
//#define BAUDRATE B38400
#define BAUDRATE B115200
#define STRIP_LEN 113
//#define ledsPerLine 90
//#define ledsPerLineEnd ledsPerLine-1
//#define lines  32
//#define maxLeds ledsPerLine*lines

//const int Mcolours[4] = {0xFF0000,0x0000FF,0xFFFF00, 0xFFFFFF};
//const int rows = 8; // number of actual strips

void signal_callback_handler(int);

void delay (int milliseconds);
//int urandom();
int udev_assignment ( void );
int randn(int min_num, int max_num);
inline int map(int, int, int, int, int);
//void init_comms(void);
int init_comms1(const char *);
int init_comms2(const char *);
//int init_joystick1(void);
//int init_joystick2(void);
void init_threads(void);
void build_data(void);
//void LedsUp1b(int, int, unsigned char *);
//void dimSend (int, uint8_t);
//void LedsUp2b(int, int, unsigned char *);
void LedPuck (int, int, int, unsigned char *);
void LedsOff1(int);
void LedsUp2(int);
void init_matrix(void);
void msleep(int);
void autoRun(void);
int udev_check (void);

// Thread Functions
void * g1_thread( void * arg );
void * g2_thread( void * arg );
//void * minute_thread1(void * arg);
void * serialRead_thread(void * arg);
//void * minute_thread2(void * arg);

//const int totalLeds = ledsPerLine*ines*4;
volatile int STOP=FALSE;
struct termios oldtio,oldtio2, newtio, newtio2;
//struct termios settings;

//char devpath[14];
//char devpath2[14];
//char jdevpath[12] = "/dev/hidraw0";
//char jdevpath2[12] = "/dev/hidraw1";
//char *usbdevname;
//char *usbdevname2;
//char *udevname;

//int Lmatrix[lines][ledsPerLine];
//int drawingMemory[ledsPerStrip*6];

// Threads
pthread_t g1thread;
pthread_t g2thread;
//pthread_t min1thread;
//pthread_t min2thread;

int g1, g2, mt1, mt2;

int gameC1, gameC2; // only need two for the Teensys, gfd, gfd2; // Devices
int b, c, x, y, j, q, r;
int m1, m2, jsize, jsize2, tick1 = 0, tick2 = 0, tick3 = 0;
//int autorun = FALSE; // Auto run state.
int multiRows[6];
int multiCols[6];
int finish = FALSE;
int button = 0;

char jchar[1]; // for button number on Teensy 1
char jchar2[1];

unsigned char colour;
unsigned char buf[3];
unsigned char buf2[3];
unsigned char puck[10]; // puck player colour
unsigned char ledsOffBuf[5];
unsigned char formbuf1[5];
unsigned char ledIdbuf1[5];
unsigned char formbuf2[6];
unsigned char ledIdbuf2[6];
uint8_t dimBuf[1];

lo_address t;

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description: Doesn't do much after the init stage
 * =====================================================================================
 */
int
main ( int argc, char *argv[] )
{
time_t tt;

/*  Intializes random number generator */
	srand((unsigned) time(&tt));

//srand(time(NULL));
//	printf("%s %s \n", argv[0], argv[1]);
//

//	this sorts out teensy connection order.
//  set up all communications to Teensy 3.1s from this call too
//	 udev_assignment();
	if(udev_assignment() != 0 ) {
		printf("***************************\n");
		printf("Not all Teensys found.\n"
		"Check that Teensys are powered up\n"
		"and have the correct Serial Numbers defined.\n"
		"Exited\n");
		printf("***************************\n");
		exit(-1);
	 }

/*  an address to send messages to. sometimes it is better to let the server
    pick a port number for you by passing NULL as the last argument.
	Args:
		host 	An IP address or number, or NULL for the local machine.
		port 	a decimal port number or service name.
	*/
//    lo_address t = lo_address_new_from_url( "osc.unix://localhost/tmp/mysocket" );
//    t = lo_address_new("192.168.0.53", "5010"); // for PD
//    t = lo_address_new("192.168.0.54", "5010");
//    t = lo_address_new(NULL, "7770");
//	if (t == NULL) {printf("OSC address error.\n"); exit(-1);}
//	else {printf("\nOSC send address: %s\n", t);}
/* 
	if (argc <= 1) {
//		printf("Not enough args.\n\tUsage: LEDdataSend DeviceName\n\tEx. %s ttyACM0\n", argv[0]);
//		printf("No args. So using default /dev/ttyACM0\n");

//		First Controller
		memset(devpath, '0', 14);
		strcpy(devpath, "/dev/ttyACM0");
		printf("First Controller: %s\n", devpath);

//		Second Controller
		memset(devpath2, '0', 14);
		strcpy(devpath2, "/dev/ttyACM1");
		printf("Second Controller: %s\n", devpath2);
	} else {
		if (argc <= 2) { // One specified controller only.
			printf("Args are: %s %s \n", argv[0], argv[1]);
			usbdevname = argv[1];
//		devpath = (char *) malloc(14);
			strcpy (devpath, "/tty/");
			strcat(devpath, usbdevname);
			printf("%s %s \n", argv[0], devpath);
		}
		else {
			printf("Args are: %s %s \n", argv[0], argv[1]);
			usbdevname = argv[1];
//		devpath = (char *) malloc(14);
			strcpy (devpath, "/tty/");
			strcat(devpath, usbdevname);

			usbdevname2 = argv[2];
//		devpath2= (char *) malloc(14);
			strcpy (devpath2, "/tty/");
			strcat(devpath2, usbdevname2);
		}
	}
*/

// Handle Ctl-C 
	signal(SIGINT, signal_callback_handler); // so we can Ctrl C out of this cleanly

// Init matrix for LED id assignment
//	init_matrix();

//   set up joysticks
//	init_joystick1();
//	init_joystick2();

//	memset (buf, 0, sizeof(buf));	// zero the buf

//	jsize = (&jchar)[1] - jchar;
//	jsize2 = (&jchar2)[1] - jchar2;

// Initiate the threads 

	init_threads();

/***************************
		While - Lets's GO!
 ***************************/

 /*  send a message to /a/b/c/d with a mixtrure of float and string arguments */
//	lo_send(t, "/a/b/c/d", "sfsff", "one", 0.12345678f, "three", -0.00000023001f, 1.0);
							   

	while (!STOP) {

/* loop until we have a terminating condition */

	} // end of while

/*************
 * 	FINISHED
 * ***********/

/*  restore the old port settings */
	tcsetattr(gameC1,TCSANOW,&oldtio);
//	tcsetattr(gameC2,TCSANOW,&oldtio2);
	close( gameC1 );
//	close( gameC2 );
//	close( gfd );
//	close( gfd2 );
	printf("Goodbye!\n");
	return 0;

}	// ---------- End of Main -------------------

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_threads
 *  Description:  
 * =====================================================================================
 */
		void
init_threads ( )
{
	if ((g1 = pthread_create(&g1thread, NULL, g1_thread, (void *) (NULL)))) {
	   fprintf(stderr, "error creating a new Game 1 thread.\n");
   	   exit(0);
   	 } else { printf("Game 1 Thread created: %d ... ", g1);}

 	pthread_detach(g1thread);
/* 
	if (( g2 = pthread_create(&g2thread, NULL, g2_thread, (void * ) (NULL)))) {
	   fprintf(stderr, "error creating a new Game 2 thread.\n");
   	   exit(0);
   	 } else { printf("Game 2 Thread created: %d ... ", g2);}

 	pthread_detach(g2thread);
*/
/*
	if ((mt1 = pthread_create(&min1thread, NULL, minute_thread1, (void * )(NULL)))) {
		fprintf(stderr, "error creating a new Minute 1 thread.\n");
		exit(0);
	} else { printf("Minute 1 Thread created: %d ... ", mt1);}

	pthread_detach(min1thread);

	if ((mt2 = pthread_create(&min2thread, NULL, minute_thread2, (void * )(NULL)))) {
		fprintf(stderr, "error creating a new Minute 2 thread.\n");
		exit(0);
	} else { printf("Minute 2 Thread created: %d ... ", mt2);}

	pthread_detach(min2thread);
*/
}		// -----  end of function init_threads  ----- 

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  init_matrix
 *  Description:
 * =====================================================================================
 *
		void
init_matrix (void )
{
int i = 0, x = 0, y = 0;

//	memset (Lmatrix, 0, sizeof(Lmatrix));	// zero the matrix

	printf("Matrix max LEDs are: %d \n",maxLeds);
	while (i < maxLeds) {
      for (y=ledsPerLineEnd; y >= 0; y-- ) {
		Lmatrix[x][y] = i++;
//		printf(" %d ",Lmatrix[x][y]);
     }
//	 printf("\n");
     x++;
     for (y=0; y < ledsPerLine; y++ ) {
		Lmatrix[x][y] = i++;
//		printf(" %d ",Lmatrix[x][y]);
     }
//	 printf("\n");
    x++;
   }
}		// -----  end of function init_matrix  ----- 
*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  delay
 *  Description:
 * =====================================================================================
*/ 
void delay(int milliseconds) {
long pause;
clock_t now,then;

	pause = milliseconds*(CLOCKS_PER_SEC/1000);
	now = then = clock();
	while( (now-then) < pause ) now = clock();

}	//  -----  end of function delay  ----- 


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  g1_thread
 *  Description:  Thread for Game 1
 * =====================================================================================
 */
void *
g1_thread( void * arg) {

/*  STRIP */
//int strip_len = STRIP_LEN;
//int offset1 = 0;
//int offset2 = 0;
int zone_len = 10;

/*  BALL */
int ball_dir = -1; //1 or -1
int ball_pos = STRIP_LEN/2;
//int ball_speed = 4;
int ball_len = 4; //num pixels that form the puck
//int ball_colour = 0x00FF00; // red

int ball_running = 1;
int ball_hit = 0;


printf("Game 1 Thread Running\n");
    while (!finish) {

// Button number is dependent on ball direction in respect to the players
// And this depends on the game initialisation state, that is, who starts first.

//	if ((state = read(gameC1, jchar, 1)) != -1) { // This will read button data from the Teensy assigned to Game 1 and that it is read only once.
//      button = atoi(jchar);
//	}

		if (button == 1) {
 //     printf("button1 hit! ");
//      printf("\t\tball_pos = %d", ball_pos);
			if((ball_pos < zone_len + ball_len ) && (ball_pos > ball_len) && (!ball_hit)) {
      printf(" change dir! %d", ball_dir);
				ball_dir = -ball_dir;
				ball_hit = !ball_hit;
//				offset1 += OFFSET;
//				ball_colour = 0xFF0000; // colour of puck is set for this player's hit
			}
      printf("\n");
	  button = 0;
      //sendOSC();
	}

		if (button == 2) {
//      printf("button2 hit! ");
//      printf("\t\tball_pos = %d", ball_pos);
			if(ball_pos > ((STRIP_LEN - zone_len) - ball_len) && (ball_pos < STRIP_LEN - ball_len) && (ball_hit)) {
      printf(" change dir! %d", ball_dir);
				ball_dir = -ball_dir;
				ball_hit = !ball_hit;
//				offset2 += OFFSET;
//				ball_colour = 0x0000FF; // colour of puck is set for this player's hit
			}
      printf("\n");
			button = 0;
    //sendOSC();
		}

		if(ball_running) {
      printf("Ball Running %d \n", ball_pos);
			if(ball_dir > 0)
				ball_pos += 1;
			else
				ball_pos -= 1;

			if(ball_pos > STRIP_LEN - ball_len) {
//				if(offset2 == 0) {
					ball_running = 0;
//	   		   	}
//				offset2 -= OFFSET;
				ball_pos = STRIP_LEN - ball_len;
				ball_dir = -ball_dir;
				ball_hit = !ball_hit;
			}

			if(ball_pos < ball_len) {
//				if(offset1 == 0) {
					ball_running = 0;
//				}
//				offset1 -= OFFSET;
				ball_pos = 0;
				ball_dir = -ball_dir;
				ball_hit = !ball_hit;
			}

      printf("Ball Running %d\n", ball_running);
			if(!ball_running) {
      printf("Ball NOT Running %d\n", ball_pos);
//      lo_send(t, "/js1/ts1_2", "i", (int)jchar[4]);
//				clearStrip();
//				buttonState1 = buttonState2 = 0;
//				offset1 = offset2 = 0;
				ball_dir = -ball_dir;
				ball_hit = !ball_hit;
				ball_pos = STRIP_LEN/2;
				ball_running = 1;
			} else {
// Draw the puck here
//      printf("Draw Ball ->\n");

				LedPuck(8000, ball_pos, ball_dir, buf); // send LED puck data. The controller should construct the rest of the puck

//				clearStrip();
//				drawOffSets();
//				drawBall();
//				lo_send(t, "/js1/ts1_2", "i", (int)jchar[4]);
//				LedsUp1b(8000, Lmatrix[x][y], buf2); // send LED data
//      FastLED.show(); // display this frame
			}
		}
    } // finish loop
//printf(" Game 1 Exited \n");
	return NULL;

}		// -----  end of function g1_thread  ----- 


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  g2_thread
 *  Description:  Thread for Game 2
 * =====================================================================================
 */
void *
g2_thread( void * arg) {

//char prevJS = 128;
/*
char prev4 = 128;
char prev5 = 128;
char JSRprev = 'c';
char JSLprev = 'c';
char JSDprev = 'c';
char JSUprev = 'c';
char JSCCWprev = 'c';
char JSCWprev = 'c';
char TLprev = 'c';
char JSTOn = 128;
int x = 16, y = 45;
int jres;
int screen = 0;
int prev_screen = 0;
*/
int button = 0;

printf("Game 2 Thread Running\n");
    while (!finish) {

// 6 params for the Mad Catz Joystick are from left: first JS buttons and thumb stick;  Last field: Base Lever.
// jchar array	   0    1   2   3	4	5
// passive state = 127 127 127 = read(jfd2, jchar2, jsize2);
//
 		button = read(gameC2, jchar2, jsize2);
//printf(" Mad Catz 2 jres = %d.  %d %d %d %d %d %d\n", jres, (int)jchar2[0], (int)jchar2[1], (int)jchar2[2], (int)jchar2[3], (int)jchar2[4], (int)jchar2[5]);
/*
		if(jres == 6) {
  			if (jchar2[4] != prev4) { // 0 is the default position
				switch (jchar2[4]) { // Thumb Stick
					case 0: 	{ JSTOn = 0; tick2= 0; break; } //printf("\t\t\t\tTS-2 OFF: %d\n",(int)jchar2[4]); break;}
					case 1: 	{ if(screen == 0) { 
									lo_send(t, "/js2/ts2_1", "i", (int)jchar2[4]);
									LedsUp1( 8001 );} else { LedsUp2( 8001 );} lo_send(t, "/js2/ts2_1", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-1: %d\n",(int)jchar2[4]); break; }
					case 2: 	{ if(screen == 0) { LedsUp1( 8002 );} else { LedsUp2( 8002 );} lo_send(t, "/js2/ts2_2", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-2: %d\n",(int)jchar2[4]); break; }
					case 3: 	{ if(screen == 0) { LedsUp1( 8003 );} else { LedsUp2( 8003 );} lo_send(t, "/js2/ts2_3", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-3: %d\n",(int)jchar2[4]); break; }
					case 4: 	{ if(screen == 0) { LedsUp1( 8004 );} else { LedsUp2( 8004 );} lo_send(t, "/js2/ts2_4", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-4: %d\n",(int)jchar2[4]); break; }
					case 5: 	{ if(screen == 0) { LedsUp1( 8005 );} else { LedsUp2( 8005 );} lo_send(t, "/js2/ts2_5", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-5: %d\n",(int)jchar2[4]); break; }
					case 6: 	{ if(screen == 0) { LedsUp1( 8006 );} else { LedsUp2( 8006 );} lo_send(t, "/js2/ts2_6", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-6: %d\n",(int)jchar2[4]); break; }
					case 7: 	{ if(screen == 0) { LedsUp1( 8007 );} else { LedsUp2( 8007 );} lo_send(t, "/js2/ts2_7", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-7: %d\n",(int)jchar2[4]); break; }
					case 8: 	{ if(screen == 0) { LedsUp1( 8008 );} else { LedsUp2( 8008 );} lo_send(t, "/js2/ts2_8", "i", (int)jchar2[4]); break; } //printf("\t\t\tTS2-8: %d\n",(int)jchar2[4]); break; }

					case 16: 	{ tick2 = 0; JSTOn = 129; lo_send(t, "/js2/trig", "i", (int)jchar[4]) ;break;} //  printf("B1-2 %d\n",jchar[6]); break;}
//					case 32: 	{ if(screen == 0) { LedsUp1( 8010 );} else { LedsUp2( 8010 );} lo_send(t, "/js2/js2cb", "i", (int)jchar2[4]); break; } //printf("\t\t\tCTB: %d\n",(int)jchar2[4]); break;}
					case 32: 	{ lo_send(t, "/js2/cb", "i", (int)jchar[4]); //printf("\t\t\tCTB: %d\n",(int)jchar[4]); break;}
								  if( randn (0,100) > 70) {
									LedsUp1( 8010 ); LedsUp2( 8010 );
								  } else {
									if(screen == 0) { LedsUp1( 8010 );} else { LedsUp2( 8010 );} 
								  }
								  break;
								}
					case 64: 	{ if(screen == 0) { LedsUp1( 8012 );} else { LedsUp2( 8012 );} lo_send(t, "/js2/js2lub", "i", (int)jchar2[4]); break; } //printf("\t\t\tRLB: %d\n",(int)jchar2[4]); break;}
		 			case 128: 	{ if(screen == 0) { LedsUp1( 8013 );} else { LedsUp2( 8013 );} lo_send(t, "/js2/js2rub", "i", (int)jchar2[4]); break; } //printf("\t\t\tRUB: %d\n",(int)jchar2[4]); break;}
				}
				prev4 = jchar2[4];
			}

  			if (jchar2[5] != prev5) { // 240 is the default position
				switch (jchar2[5]) {
					case 240: 	{ lo_send(t, "/js2/lboff", "i", (int)jchar2[5]); tick2 = 0; break; } //printf("\t\t\tButtons OFF: %d\n",(int)jchar2[5]); break; }
					case 241: 	{ if(screen == 0) { LedsUp1( 8015 );} else { LedsUp2( 8015 );} lo_send(t, "/js2/llb", "i", (int)jchar2[5]); break; } //printf("\t\t\tLLB: %d\n",(int)jchar2[5]); break; }
					case 242: 	{ if(screen == 0) { LedsUp1( 8014 );} else { LedsUp2( 8014 );} lo_send(t, "/js2/rlb", "i", (int)jchar2[5]); break; } //printf("\t\t\tRLB: %d\n",(int)jchar2[5]); break; }
//					case 244: 	{ if(screen == 0) { LedsUp1( 8011 );} else { LedsUp2( 8011 );} lo_send(t, "/js2/lb", "i", (int)jchar[5]); break; } //printf("\t\t\tJSLB: %d\n",(int)jchar[5]); break; }
					case 244: 	{ if(JSTOn == 0) { 
								  	lo_send(t, "/js2/lb", "i", (int)jchar2[5]); 
									if (screen == 0) {
										if (randn(0,100) > 50) { LedsUp1( 8011 ); } else { LedsUp1( 8009 ); }
									} else { 
										if (randn(0,100) > 50) { LedsUp2( 8011 ); } else { LedsUp2( 8009 ); }
									} 
								  } //printf("\t\t\tJSLB: %d\n",(int)jchar2[5]); break; }
								  break;
								}
				}
				prev5 = jchar2[5];
			}

  			if ((jchar2[0] != 0) && (jchar2[0] > 127) && (JSRprev != jchar2[0]) && (JSTOn == 129)) {
				tick2 = 0; JSRprev = jchar2[0]; x = map((int)jchar2[0], 255, 128, 31, 16); LedsUp1b(8000, Lmatrix[x][y], buf2); lo_send(t, "/js2/r", "i", (int)jchar2[0]);} // printf("\t\t\tJS2-Right: %d\n", (int)jchar2[0]);}
 			if ((jchar2[0] != 0) && (jchar2[0] < 128) && (JSLprev != jchar2[0]) && (JSTOn == 129)) {
				tick2 = 0; JSLprev = jchar2[0]; x = map((int)jchar2[0], 1, 127, 0, 16); LedsUp1b(8000, Lmatrix[x][y], buf2);lo_send(t, "/js2/l", "i", (int)jchar2[0]); } //printf("\t\t\t\tJS2-Left: %d\n", x);}

			if ((jchar2[1] != 0) && (jchar2[1] < 128) && (JSUprev != jchar2[1]) && (JSTOn == 129)) {
				tick2 = 0; JSUprev = jchar2[1]; y = map((int)jchar2[1], 127, 1, 45, 89); LedsUp1b(8000, Lmatrix[x][y], buf2); lo_send(t, "/js2/u", "i", (int)jchar2[1]); } // printf("\t\t\t\tJS2-Down: %d\n", y);}
			if ((jchar2[1] != 0) && (jchar2[1] > 127) && (JSDprev != jchar2[1]) && (JSTOn == 129)) {
				tick2 = 0; JSDprev = jchar2[1]; y = map((int)jchar2[1], 128, 255, 44, 0); LedsUp1b(8000, Lmatrix[x][y], buf2); lo_send(t, "/js2/d", "i", (int)jchar2[1]); } // printf("\t\t\t\tJS2-Up: %d\n", y );}
 
			if ((jchar2[2] < 128) && (JSCCWprev != jchar2[2])) {
				if (jchar2[2] < 100) {
					screen = 0;
					if (screen != prev_screen) {
						tick2 = 0; lo_send(t, "/js2/ccw", "i", (int)screen); prev_screen = screen;}
				}
				JSCCWprev = jchar2[2]; // printf("\t\t\t\t\tJS2-CCW: %d\n", (int)jchar2[2]);
			}
			if ((jchar2[2] > 127) && (JSCWprev != jchar2[2])) {
				if (jchar2[2] > 156) {
					screen = 1;
					if (screen != prev_screen) {
						tick2 = 0; lo_send(t, "/js2/cw", "i", (int)screen); prev_screen = screen;}
				}
				JSCWprev = jchar2[2]; // printf("\t\t\t\t\tJS2-CW: %d\n", (int)jchar2[2]);
			}
//
//	Audio volume control
//
  			if (jchar2[3] != TLprev ) {
//				 if (jchar2[3] > 127) { screen = 0; } else { screen = 1; } TLprev = jchar2[3]; } 
//				dimSend(screen, jchar2[3]); TLprev = jchar2[3]; lo_send(t, "/js2tl", "i", (int)jchar2[3]); } // printf("Throttle Lever:  %d\n", (int)jchar2[3]);}
				TLprev = jchar2[3]; lo_send(t, "/js2/tl", "i", (int)jchar2[3]); } // printf("Throttle Lever:  %d\n", (int)jchar2[3]);}

		} // end of jres if
*/
    }
//printf(" Game 2 Exited \n");
	return NULL;

}		// -----  end of function g2_thread  ----- 


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  LedPUck
 *  Description:  This should send puck start info to Teensy.
 *
 * =====================================================================================
 */
	void
LedPuck (int formId, int ledId, int direction, unsigned char *buf)
{
int	i = 0;
int res;

//	printf("LedPuck 1 formId %d ledId %d. \n", formId, ledId);

	if (direction < 0) {
		while (i++ < 3) { // Blue
			buf[i] = 0;
			buf[i++] = 255;
			buf[i] = 0;
		}
	} else {
		while (i++ < 3) { // Red
			buf[i] = 0; // (unsigned char)rand() % 255;
			buf[i++] = 0; // (unsigned char)rand() % 255;
			buf[i] = 255;
		}
	}

// Id for Puck control
		snprintf((char *)formbuf1, 5, "%d", formId);
		if ((res = write(gameC1, formbuf1, 4)) != 4) {
			printf("\nLedPuck formId Failed to write data. ");}

// Led Position
		snprintf((char *)ledIdbuf1, 5, "%d", ledId);
		if ((res = write(gameC1, ledIdbuf1, 4)) != 4) {
			printf("\nLedPuck ledId Failed to write data.\n");}

// Colour of puck based on direction
	if ((res = write(gameC1, buf, 3)) != 3) {
		printf("\nLedPuck buf Failed to write data.\n");}

}		// -----  end of LedPuck


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  LedsUp1b
 *  Description:  This should now write to either ports depending on LED number being over 1439
 *  			  Currently no longer sending colour data, just LED id.
 * =====================================================================================
 *
	void
LedsUp1b (int formId, int ledId, unsigned char *buf)
{
//int	i = 0;
int res;

//	printf("Screen 1 formId %d ledId %d. ", formId, ledId);

	if (ledId < 1440) {
		snprintf((char *)formbuf1, 5, "%d", formId);
		if ((res = write(gameC1, formbuf1, 4)) != 4) {
			printf("\nLedsUp1b formId Failed to write data. ");}
//	else {
//	printf("Screen 1 formId %d  ", formId);
//		printf("Form Data written. %d - ", res);
//	}
		snprintf((char *)ledIdbuf1, 5, "%d", ledId);
		if ((res = write(gameC1, ledIdbuf1, 4)) != 4) {
			printf("\nLedsUp1b ledId Failed to write data.\n");}
/ 
		while (i++ < 3) {
			buf[i] = (unsigned char)rand() % 255;
//			buf[i++] = (unsigned char)rand() % 255;
//			buf[i] = (unsigned char)rand() % 255;
		}

		if ((res = write(gameC1, buf, 3)) != 3) {
			printf("\nLedsUp1b buf Failed to write data.\n");}
/
	} else { // write to port 2

		snprintf((char *)formbuf1, 5, "%d", formId);
		if ((res = write(gameC2, formbuf1, 4)) != 4) {
			printf("\nLedsUp1b formId Failed to write data. ");}
//	else {
//	printf("Screen 1 formId %d  ", formId);
//		printf("Form Data written. %d - ", res);
//	}
		snprintf((char *)ledIdbuf1, 5, "%d", ledId - 1440);
		if ((res = write(gameC2, ledIdbuf1, 4)) != 4) {
		printf("\nLedsUp1b ledId Failed to write data.\n");}
//	else {
//	printf("ledId %d. \n", ledId);
//		printf("LED iD Data written. %d - ", res);
//	}
/ 
	while (i++ < 3) {
		buf[i] = (unsigned char)rand() % 255;
//		buf[i++] = (unsigned char)rand() % 255;
//		buf[i] = (unsigned char)rand() % 255;
	}

	if ((res = write(port2, buf, 3)) != 3) {
		printf("\nLedsUp1b buf Failed to write data.\n");}
//	else {
//		printf("LedsUp1b buf Data written. %d\n", res);
	}
/
	}
}		// -----  end of LedsUp1b
*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  dimBuf
 *  Description:  This writes a dim value. But to which screen? 
 *  			  
 * =====================================================================================
 /
	void
dimSend (int screen, uint8_t dimVal)
{
int res;
int formId  = 7000;

//	printf("Screen 1 formId %d ledId %d. ", formId, ledId);

	if (screen == 0 ) {
		snprintf((char *)formbuf1, 5, "%d", formId);
		if ((res = write(port, formbuf1, 4)) != 4) { printf("\nLedsUp1b formId Failed to write data. ");}

		snprintf((char *)dimBuf, 2, "%d", dimVal);
		if ((res = write(port, dimBuf, 1)) != 1) {
			printf("\nLedsUp1b ledId Failed to write data.\n");}

	} else { // write to screen 2

		snprintf((char *)formbuf1, 5, "%d", formId);
		if ((res = write(port2, formbuf1, 4)) != 4) { printf("\nLedsUp1b formId Failed to write data. ");}

		snprintf((char *)dimBuf, 2, "%d", dimVal);
		if ((res = write(port2, dimBuf, 1)) != 1) { printf("\nLedsUp1b ledId Failed to write data.\n");}
	}
}		// -----  end of dimSend
*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  LedsUp1
 *  Description:  
 * =====================================================================================
 */

void
LedsOff1 (int ledId) {
int res;

	printf("LedsOff: ledId %d led1buf size %d\n", ledId, sizeof(ledsOffBuf));
	snprintf((char *)ledsOffBuf, 5, "%d", ledId);

	if ((res = write(gameC1, ledsOffBuf, 4)) != 4) {
		printf("\nFailed to write data to gameC1.\n"); }
//	} else {
//		printf("screen 1 LEDs ON Data written. %d\n", res);
//	}

}		// -----  end of function LedsUp1  ----- 


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  LedsUp2
 *  Description:  
 * =====================================================================================
 */
void
LedsUp2 (int ledId2) {
int res;

//	printf("ledId2 %d led2buf size %d\n", ledId2, sizeof(led2buf));
	snprintf((char *)ledsOffBuf, 5, "%d", ledId2);

	if ((res = write(gameC2, ledsOffBuf, 4)) != 4) {
		printf("\nFailed to write data to LEDs.\n"); }
//	else {
//		printf("screen 2 LEDs ON Data written. %d %s \n", res, led2buf);
//	}

}		// -----  end of function LedsUp2  ----- 


//=======================================================
//			init_comms ALL comms to Teesnsy 3.1s
//=======================================================
/*
void
init_comms()
{
	init_comms1("/dev/ttyACM0");
	init_comms2("/dev/ttyACM1");

}
*/


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_comms1
 *  Description:  
 * =====================================================================================
 *
int
init_comms1(const char *devpath)
{
	/
	Open device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
	/
	printf("Opening First Teensy 3.1 Light Controller...\n");

	gameC1 = open(devpath, O_RDWR | O_NOCTTY );
	printf("Device Status %d ... ", gameC1);
	if (gameC1 < 0) {perror(devpath); exit(-1); } else {printf("Teensy 3.1 Device 1: %d is open.\n", gameC1);}

// Configure the port
	tcgetattr(gameC1, &settings);
	cfmakeraw(&settings);
	tcsetattr(gameC1, TCSANOW, &settings);
	long flag = fcntl(gameC1, F_GETFL, 0 );
	fcntl(gameC1,F_SETFL,flag);


	return 0;

}		// -----  end of function init_comms1  ----- 
*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_comms1
 *  Description:  
 * =====================================================================================
 */
int
init_comms1 (const char * devpath)
{

//	Open device for reading and writing and not as controlling tty
//	because we don't want to get killed if linenoise sends CTRL-C.

	printf("Opening First Teensy 3.1 Light Controller...%s\n", devpath);

	gameC1 = open(devpath, O_RDWR | O_NOCTTY );
	printf("Device Status %d ... ", gameC1);
	if (gameC1 < 0) {perror(devpath); exit(-1); } else {printf("Teensy 3.1 Device 1: %d is open.\n", gameC1);}

	tcgetattr(gameC1,&oldtio); // save current serial port settings /
	bzero(&newtio, sizeof(newtio)); // clear struct for new port settings /


//	BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
//	CRTSCTS : output hardware flow control (only used if the cable has
//	all necessary lines. See sect. 7 of Serial-HOWTO)
//	CS8     : 8n1 (8bit,no parity,1 stopbit)
//	CLOCAL  : local connection, no modem contol
//	CREAD   : enable receiving characters

	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

//	IGNPAR  : ignore bytes with parity errors
//	ICRNL   : map CR to NL (otherwise a CR input on the other computer
//                   will not terminate input)
//evice raw (no other input processing)

	newtio.c_iflag = IGNPAR | ICRNL;

//	Raw output.
//	newtio.c_oflag = 0;
	

//	ICANON  : enable canonical input
//	disable all echo functionality, and don't send signals to calling program

	newtio.c_lflag = ICANON;

//	initialize all control characters
//	default values can be found in /usr/include/termios.h, and are given
//	in the comments, but we don't need them here

	newtio.c_cc[VINTR]    = 0;     // Ctrl-c /
	newtio.c_cc[VQUIT]    = 0;     // Ctrl-\ /
	newtio.c_cc[VERASE]   = 0;     // del /
	newtio.c_cc[VKILL]    = 0;     // @ /
	newtio.c_cc[VEOF]     = 4;     // Ctrl-d /
	newtio.c_cc[VTIME]    = 0;     // inter-character timer unused /
	newtio.c_cc[VMIN]     = 0;     // non blocking /
	newtio.c_cc[VSWTC]    = 0;     // '\0' /
	newtio.c_cc[VSTART]   = 0;     // Ctrl-q /
	newtio.c_cc[VSTOP]    = 0;     // Ctrl-s /
	newtio.c_cc[VSUSP]    = 0;     // Ctrl-z /
	newtio.c_cc[VEOL]     = 0;     // '\0' /
	newtio.c_cc[VREPRINT] = 0;     // Ctrl-r /
	newtio.c_cc[VDISCARD] = 0;     // Ctrl-u /
	newtio.c_cc[VWERASE]  = 0;     // Ctrl-w /
	newtio.c_cc[VLNEXT]   = 0;     // Ctrl-v /
	newtio.c_cc[VEOL2]    = 0;     // '\0' /


//	now clean the modem line and activate the settings for the port

	tcflush(gameC1, TCIFLUSH);
	tcsetattr(gameC1,TCSANOW, &newtio);
	return 0;

}		// -----  end of function init_comms2  ----- 

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_comms2
 *  Description:  
 * =====================================================================================
 *
int
init_comms2 (const char * devpath2)
{

//	Open device for reading and writing and not as controlling tty
//	because we don't want to get killed if linenoise sends CTRL-C.

	printf("Opening Second Teensy 3.1 Light Controller...%s\n", devpath2);

	gameC2 = open(devpath2, O_RDWR | O_NOCTTY );
	printf("Device Status %d ... ", gameC2);
	if (gameC2 < 0) {perror(devpath2); exit(-1); } else {printf("Teensy 3.1 Device 2: %d is open.\n", gameC2);}

	tcgetattr(gameC2,&oldtio2); // save current serial port settings /
	bzero(&newtio2, sizeof(newtio2)); // clear struct for new port settings /


//	BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
//	CRTSCTS : output hardware flow control (only used if the cable has
//	all necessary lines. See sect. 7 of Serial-HOWTO)
//	CS8     : 8n1 (8bit,no parity,1 stopbit)
//	CLOCAL  : local connection, no modem contol
//	CREAD   : enable receiving characters

	newtio2.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

//	IGNPAR  : ignore bytes with parity errors
//	ICRNL   : map CR to NL (otherwise a CR input on the other computer
//                   will not terminate input)
//evice raw (no other input processing)

	newtio2.c_iflag = IGNPAR | ICRNL;

//	Raw output.
//	newtio.c_oflag = 0;
	

//	ICANON  : enable canonical input
//	disable all echo functionality, and don't send signals to calling program

	newtio2.c_lflag = ICANON;

//	initialize all control characters
//	default values can be found in /usr/include/termios.h, and are given
//	in the comments, but we don't need them here

	newtio2.c_cc[VINTR]    = 0;     // Ctrl-c /
	newtio2.c_cc[VQUIT]    = 0;     // Ctrl-\ /
	newtio2.c_cc[VERASE]   = 0;     // del /
	newtio2.c_cc[VKILL]    = 0;     // @ /
	newtio2.c_cc[VEOF]     = 4;     // Ctrl-d /
	newtio2.c_cc[VTIME]    = 0;     // inter-character timer unused /
	newtio2.c_cc[VMIN]     = 0;     // non blocking /
	newtio2.c_cc[VSWTC]    = 0;     // '\0' /
	newtio2.c_cc[VSTART]   = 0;     // Ctrl-q /
	newtio2.c_cc[VSTOP]    = 0;     // Ctrl-s /
	newtio.c_cc[VSUSP]    = 0;     // Ctrl-z /
	newtio2.c_cc[VEOL]     = 0;     // '\0' /
	newtio2.c_cc[VREPRINT] = 0;     // Ctrl-r /
	newtio2.c_cc[VDISCARD] = 0;     // Ctrl-u /
	newtio2.c_cc[VWERASE]  = 0;     // Ctrl-w /
	newtio2.c_cc[VLNEXT]   = 0;     // Ctrl-v /
	newtio2.c_cc[VEOL2]    = 0;     // '\0' /


//	now clean the modem line and activate the settings for the port

	tcflush(gameC2, TCIFLUSH);
	tcsetattr(gameC2,TCSANOW, &newtio2);
	return 0;

}		// -----  end of function init_comms2  ----- 

*/
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  msleep
 *  Description:  
 * =====================================================================================
 */
void msleep(int ms) {
	struct timespec time;
	time.tv_sec = ms /1000;
	time.tv_nsec = (ms % 1000) * (1000 * 1000);
	nanosleep(&time, NULL);

}		// -----  end of function msleep  ----- 


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  minute_thread1
 *  Description:  
 * =====================================================================================
 *
void * 
minute_thread1(void * arg)
{
	printf("Minute Thread 1 running...\n");

	while (!STOP) {
		msleep(60000);
		tick1++;
		tick2++;
		tick3 = 0;
		if ((tick1 >= IDLETIME) && (tick2 >= IDLETIME) && (autorun == FALSE)) {autorun = TRUE; } //printf("Thread 1 Auto run TRUE: T1 %d T2 %d T3 %d\n", tick1, tick2, tick3);}
	}
	return NULL;

}		// -----  end of function minute_thread1  ----- 
*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  minute_thread2
 *  Description:  
 * =====================================================================================
 *
void * 
minute_thread2(void * arg)
{
	printf("Minute Thread 2 running...\n");

	while (!STOP) {
		if ((tick1 >= IDLETIME) && (tick2 >= IDLETIME) && (autorun == TRUE )) { tick3++; msleep(15000); } //printf("Thread 2 Auto run TRUE: T1 %d T2 %d T3 %d\n", tick1, tick2, tick3);}
		else { tick3 = 0; autorun = FALSE; msleep(15000);} //  printf("Thread 2 Auto run FALSE: T1 %d T2 %d T3 %d\n", tick1, tick2, tick3);}
		if ((tick1 >= IDLETIME) && (tick2 >= IDLETIME) && (tick3 >= IDLETIME) && (autorun == TRUE)) { autoRun(); tick1 = 0; tick2 = 0; tick3 = 0; autorun = FALSE;}
	}
	return NULL;

}		// -----  end of function minute_thread2  ----- 
*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  autoRun
 *  Description:  
 * =====================================================================================
 *
		void
autoRun (void)
{
	printf("\nGoing into Auto mode...\n\n");  

//    delay(100);

	LedsUp1( 8010);
	LedsUp2( 8010);

	while ((tick1 >= IDLETIME) && (tick2 >= IDLETIME) && (autorun == TRUE)) {
		switch (randn(0,20)) {
			case 0: 	{ LedsUp1( 8001 ); break; }
			case 1: 	{ LedsUp1( 8002 ); break; }
			case 2: 	{ LedsUp1( 8003 ); break; }
			case 3: 	{ LedsUp1( 8004 ); break; }
			case 4: 	{ LedsUp1( 8005 ); break; }
			case 5: 	{ LedsUp1( 8006 ); break; }
			case 6: 	{ LedsUp1( 8007 ); break; }
			case 7: 	{ LedsUp1( 8008 ); break; }
			case 8: 	{ LedsUp1( 8009 ); break; }
			case 9: 	{ LedsUp1( 8010 ); break; } // clear screen
			case 10: 	{ LedsUp1( 8011 ); break; }
			case 11: 	{ LedsUp1( 8010 ); break; }
			case 12: 	{ LedsUp1( 8012 ); break; }
		 	case 13: 	{ LedsUp1( 8013 ); break; }
		 	case 14: 	{ LedsUp1( 8014 ); break; }
		 	case 15: 	{ LedsUp1( 8015 ); break; } // Mondrian like...
			default: break;
		}
		if (randn(0,101) > 50) {delay(randn(50,1000));};
  		switch (randn(0,15)) {
			case 0: 	{ LedsUp2( 8001 ); break; }
			case 1: 	{ LedsUp2( 8002 ); break; }
			case 2: 	{ LedsUp2( 8003 ); break; }
			case 3: 	{ LedsUp2( 8004 ); break; }
			case 4: 	{ LedsUp2( 8005 ); break; }
			case 5: 	{ LedsUp2( 8006 ); break; }
			case 6: 	{ LedsUp2( 8007 ); break; }
			case 7: 	{ LedsUp2( 8008 ); break; }
			case 8: 	{ LedsUp2( 8009 ); break; }
			case 9: 	{ LedsUp2( 8010 ); break; } // clear screen
			case 10: 	{ LedsUp2( 8011 ); break; }
			case 11: 	{ LedsUp2( 8010 ); break; }
		 	case 12: 	{ LedsUp2( 8012 ); break; }
		 	case 13: 	{ LedsUp2( 8013 ); break; }
		 	case 14: 	{ LedsUp2( 8014 ); break; }
		 	case 15: 	{ LedsUp2( 8015 ); break; } // Mondrian like...
//			default: break;
		}
		delay(randn(50,1000));
	} // end atuo while loop

	LedsUp1( 8010);
	LedsUp2( 8010);
	tick1 = 0;
	tick2 = 0;
	tick3 = 0;
	autorun = FALSE;
	printf("\nLeaving Auto mode...\n\n");  
//		return 0;

}		// -----  end of function autoRun  ----- 
*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  udev_assignment
 *  Description:  
 * =====================================================================================
 */
	int
udev_assignment ( void )
{
struct udev *udev;
struct udev_enumerate *enumerate;
struct udev_list_entry *devices, *dev_list_entry;
struct udev_device *dev;
int dev_state;
int dev_cnt = 0;
int Teensys_matched = 1;

 while (dev_cnt < DEVICE_CNT) { // what to loop through potential devices and find there serial number which should be unique for the Teensy 3.1

	/* Create the udev object */
	udev = udev_new();

	if (!udev) {
		printf("Can't create udev: %s\n", udevnames[dev_cnt]);
		exit(1);
	}

	/* Create a list of the devices in the 'hidraw' subsystem. */
	enumerate = udev_enumerate_new(udev);
//	if((dev_state = udev_enumerate_add_match_parent(enumerate, dev)) >= 0) {printf("found device struct. %d\n", dev_state);}
	if((dev_state = udev_enumerate_add_match_sysname(enumerate, udevnames[dev_cnt])) >= 0) {printf("Found device struct. %d\n", dev_state);}
//	if((dev_state = udev_enumerate_add_match_subsystem(enumerate, udevnames)) >= 0) {printf("found device struct. %d\n", dev_state);}
	if((dev_state = udev_enumerate_scan_devices(enumerate)) >= 0) {printf("Scanned device. %d\n", dev_state);}
//	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	/* For each item enumerated, print out its information.
	   udev_list_entry_foreach is a macro which expands to
	   a loop. The loop will be executed for each member in
	   devices, setting dev_list_entry to a list entry
	   which contains the device's path in /sys. */
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;

		/* Get the filename of the /sys entry for the device
		   and create a udev_device object (dev) representing it */
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		/* usb_device_get_devnode() returns the path to the device node
		   itself in /dev. */
		printf("\tDevice Node Path: %s\n", dev_node = udev_device_get_devnode(dev));

		/* The device pointed to by dev contains information about
		   the hidraw device. In order to get information about the
		   USB device, get the parent device with the
		   subsystem/devtype pair of "usb"/"usb_device". This will
		   be several levels up the tree, but the function will find
		   it.*/
		dev = udev_device_get_parent_with_subsystem_devtype(
		       dev,
		       "usb",
		       "usb_device");
		if (!dev) {
			printf("Unable to find parent usb device.\n");
			exit(1);
		}

		/* From here, we can call get_sysattr_value() for each file
		   in the device's /sys entry. The strings passed into these
		   functions (idProduct, idVendor, serial, etc.) correspond
		   directly to the files in the directory which represents
		   the USB device. Note that USB strings are Unicode, UCS2
		   encoded, but the strings returned from
		   udev_device_get_sysattr_value() are UTF-8 encoded. */

		printf("\tVID/PID: %s %s\n",
		        udev_device_get_sysattr_value(dev,"idVendor"),
		        udev_device_get_sysattr_value(dev, "idProduct"));
		printf("\tManufacturer:  %s\n\tProduct: %s\n",
		        udev_device_get_sysattr_value(dev,"manufacturer"),
		        udev_device_get_sysattr_value(dev,"product"));
//		printf("  serial: %s\n", serial_name = udev_device_get_sysattr_value(dev, "serial"));
		serial_num = udev_device_get_sysattr_value(dev, "serial");
		if(strcmp(serial_num, serial_nums[0]) == 0) {
			printf("serial number %s matched expected number %s.\n\n", serial_num, serial_nums[0]);
			printf("Assigning to gameC1 to %s.\n\n", dev_node);
			init_comms1(dev_node);
			Teensys_matched--;
		}
/*
		if(strcmp(serial_num, serial_nums[1]) == 0) {
			printf("serial number %s matched expected number %s.\n\n", serial_num, serial_nums[1]);
			printf("Assigning to gameC1 to %s.\n\n", dev_node);
			init_comms2(dev_node);
			Teensys_matched--;
		}
*/
		udev_device_unref(dev);
	}
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);

	udev_unref(udev);
	dev_cnt++; // Increment to the next devie

	} // End of Device while loop

	return Teensys_matched;
}
		/* -----  end of function udev_assignment  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  serial_read_thread
 *  Description:  
 * =====================================================================================
 */
void * 
serialRead_thread(void * arg)
{
int state = 0;

	printf("Serial Read Thread running...\n");

	while (!STOP) {
		if ((state = read(gameC1, jchar, 1)) != -1) { // This will read button data from the Teensy assigned to Game 1 and that it is read only once.
       		button = atoi(jchar);
	}
	}
	return NULL;

}		// -----  end of Serial Read Thread  ----- 


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  map
 *  Description:
 * =====================================================================================
 */
inline int map(int x, int in_min, int in_max, int out_min, int out_max)
{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

}	//  -----  end of function map  ----- 


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  randn
 *  Description:
 * =====================================================================================
 */
int randn(int min_num, int max_num) {
	int result=0,low_num=0,hi_num=0;

	if(min_num<max_num) {
		low_num=min_num;
		hi_num=max_num+1; // this is done to include max_num in output.
	}else{
		low_num=max_num+1;// this is done to include max_num in output.
		hi_num=min_num;
	}
   result = (random()%(hi_num-low_num))+low_num;
   return result;

}	//  -----  end of function randn  ----- 


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: signal_callback_handler
 *  Description:  
 * =====================================================================================
 */
void signal_callback_handler(int signum)  {

	printf("Caught signal %d. Reseting and Exiting...\n", signum);

	LedsOff1(8001);
//	LedsUp2(8010);

//	tick1 = 0;
//	tick2 = 0;
//	autorun = FALSE;
	finish = TRUE;
	STOP = TRUE;

}		// -----  end of function signal_callback_handler  ----- 

/********************************************************************
 * 
 *				OLD CODE below
 *
 *
 ********************************************************************/
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  LedsUp2b
 *  Description:
 * =====================================================================================
 */
/*  	void
LedsUp2b (int formId, int ledId, unsigned char *buf)
{
int	i = 0;
int res;

//	printf("Screen 2 formId %d ledId %d. ", formId, ledId );
	snprintf((char *)formbuf2, 5, "%d", formId);
	if ((res = write(port2, formbuf2, 4)) != 4) {
		printf("\nLedsUp2b formId Failed to write data.\n");}
	else {
	printf("Screen 2 formId %d,  ", formId );
//		printf("LedsUp2b FormId Data written. %d - ", res);
	}
	snprintf((char *)ledIdbuf2, 6, "%d", ledId);
	if ((res = write(port2, ledIdbuf2, 5)) != 5) {
		printf("\nLedsUp1b ledId Failed to write data.\n");}
	else {
	printf(" ledId %d.\n ", ledId );
//		printf("LedsUp2b Id Data written %d - ", res);
	}

	while (i < 3) {
		buf[i++] = (unsigned char)rand() % 255;
		buf[i++] = (unsigned char)rand() % 255;
		buf[i] = (unsigned char)rand() % 255;
	}

	if ((res = write(port2, buf, 3)) != 3) {
		printf("\nLedsUp2b buf Failed to write data.\n");}
//	else {
//		printf("LedsUp2b buf Data written. %d\n", res);
//	}

}		// -----  end of function LedsUp  ----- *
*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  urandom
 *  Description:
 * =====================================================================================
 *
int urandom(){
   FILE *rand = fopen("/dev/urandom","r");
   unsigned char c1 = getc(rand);
   unsigned char c2 = getc(rand);
   fclose(rand);
   return (int)c1*c2;
} */

/*
//=======================================================
//			js2_thread - ThrustMaster
//=======================================================
void *
js2_thread( void * arg) {

char prev2 = 128;
char JSRprev = 'c';
char JSLprev = 'c';
char JSDprev = 'c';
char JSUprev = 'c';
char JSTOn = 128;
int x = 16, y = 45;
int jres;

printf("Joystick 2 Thread Running\n");
    while (!finish) {

// 4 params for the Thrustmaster Joystick are from left: first JS buttons and thumb stick;  Last field: Base Lever.
// jchar array	   0    1   2   3
// passive state = 127 127 127 = read(jfd2, jchar2, jsize2);
// `
 		jres = read(jfd2, jchar2, jsize2);
		if(jres == 4) {
//printf(" Thrustmaster jres2 = %d.  %d %d %d %d \n", jres2, jchar2[0], jchar2[1], jchar2[2], jchar2[3]);
  			if (jchar2[0] != prev2) { // 8 is the default position
				tick2 = 0;
				switch (jchar2[0]) {
					case 0: 	{ LedsUp2( 8001); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-0: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 16: 	{ LedsUp2( 8002); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-1: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 32: 	{ LedsUp2( 8003); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-2: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 48: 	{ LedsUp2( 8004); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-3: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 64: 	{ LedsUp2( 8005); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-4: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 80: 	{ LedsUp2( 8006); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-5: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 96: 	{ LedsUp2( 8007); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-6: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 112: 	{ LedsUp2( 8008); prev2 = jchar2[0]; break;} //printf("\t\t\tTS2-7: %d\n",jchar2[0]); prev2 = jchar2[0]; break; }
					case 128: 	{ prev2 = jchar2[0]; JSTOn = 0; break; } //printf("\t\t\t\tOFF: %d\n",jchar2[0]); break;}
					case 129: 	{ prev2 = jchar2[0]; JSTOn = 129; break; } //printf("\t\t\tJST: %d\n",jchar2[0]); break;}
					case 130: 	{ LedsUp2( 8011); prev2 = jchar2[0]; break; } //printf("\t\t\tJSBC2: %d\n",jchar2[0]); break;}
					case 132: 	{ if( randn(0,101) > 50 ) {LedsUp2(8010);} else { LedsUp1(8010);} prev2 = jchar2[0]; break; } //printf("\t\t\tJSBR2-2: %d\n",jchar2[0]); break;}
		 			case 136: 	{ LedsUp2( 8012); prev2 = jchar2[0]; break; } //printf("\t\t\tJSBR2: %d\n",jchar2[0]); break;}
				}
			}

  			if ((jchar2[1] != 0) && (jchar2[1] > 127) && (JSRprev != jchar2[1]) && (JSTOn == 129)) {
				tick2 = 0; JSRprev = jchar2[1]; x = map((int)jchar2[1], 255, 128, 16, 0); LedsUp1b(8000, Lmatrix[x][y], buf2);} //  printf("\t\t\tJS2-Right: %d\n", x);}
 			if ((jchar2[1] != 0) && (jchar2[1] < 128) && (JSLprev != jchar2[1]) && (JSTOn == 129)) {
				tick2 = 0; JSLprev = jchar2[1]; x = map((int)jchar2[1], 0, 127, 17, 31); LedsUp1b(8000, Lmatrix[x][y], buf2); } //printf("\t\t\t\tJS2-Left: %d\n", x);}
			if ((jchar2[2] != 0) && (jchar2[2] < 128) && (JSDprev != jchar2[2]) && (JSTOn == 129)) {
				tick2 = 0; JSDprev = jchar2[2]; y= map((int)jchar2[2], 0, 127, 44, 0); LedsUp1b(8000, Lmatrix[x][y], buf2);} // printf("\t\t\t\tJS2-Down: %d\n", y);}
			if ((jchar2[2] != 0) && (jchar2[2] > 127) && (JSUprev != jchar2[2]) && (JSTOn == 129)) {
				tick2 = 0; JSUprev = jchar2[2]; y = map((int)jchar2[2], 255, 128, 45, 89); LedsUp1b(8000, Lmatrix[x][y], buf2);} // printf("\t\t\t\tJS2-Up: %d\n", y );}

		} // end of jres2 if
    }
//printf(" THRUSTMASTER Exited \n");
	return NULL;
}
*/


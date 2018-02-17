#pragma once

#include "ofxTeensyOcto.h"
#include "ofMain.h"
#include "button.h"
#include "game.h"
#include "ofxOsc.h"

#ifdef TARGET_RASPBERRY_PI
#include "wiringPi.h"
#define USE_TEENSY
#endif

#define USE_TEENSY

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        void updateFbo();
        void updateTeensy();
        void setupMedia();
        void enableVideo();
        void disableVideo();
        void drawPong();
        void drawRawPixels();
        void drawGamePixels();
        void showFPS();
        void setupButtons();
        void updateButtons();

		//----------------------------OSC
		// admin osc events
		void ListenForOSC();
		void SendGameFinished();	// sends when game is over and we return to idle
		void SendButtonPress(int laneIndex, int nearFar, int hitMiss);
		void SendRoundWon(int playerIndex, int laneIndex, int rallyLength);
		void SendGameWon(int playerIndex, int rallyLength);
		void SendPuckPositions(Puck p0, Puck p1); // sends the normalized puck positions
		void SendReadyToServe(float end); // float end to serve

    
    // LED - Teensy stuff
    //-----------------------------
    ofxTeensyOcto teensy;
    int stripWidth;
    int stripHeight;
    int stripsPerPort;
    int numPorts;
    int brightness;
    
    // FBO stuff
    //-----------------------------
    ofFbo fbo;
    int fboRotate;
    bool fboFlip;
    ofTexture tex;
    ofPixels guiPixels;
    
    // Gui
    //-----------------------------
    void drawPanels();
    ofTrueTypeFont font;
    
    // Graphic functions
    //-----------------------------
    void drawDemos();
    void drawVideos();
    void drawImages();
    int drawModes;
    int demoModes;
    float ballpos;
    int dir;

    // Video player
    //-----------------------------
    ofDirectory dirVid;
    vector<ofVideoPlayer> vid;
    int currentVideo;
    bool videoOn;
    
    // Image player
    //-----------------------------
    ofDirectory dirImg;
    vector<ofImage> img;
    int currentImage;
    
    // Timer
    float prevTime;
    float curTime;

    //Buttons
    Button buttons[4];


	// OSC
	//-----------------------------
	// admin
	static string _GetSettingsOSCAdd;
	static string _SetSettingsOSCAdd;
	static string _NewGameOSCAdd;
	static string _ResetGameOSCAdd;
	static string _GameFinishedOSCAdd;

	// events
	static string _ButtonPressOSCAdd;
	static string _RoundWonOSCAdd;
	static string _GameWonOSCAdd;
	static string _Lane0PuckOSCAdd;
	static string _Lane1PuckOSCAdd;
	static string _RPiReadyToServeOSCAdd;

	
    // OSC
	ofxOscSender oscSender;
	ofxOscReceiver oscReceiver;

    Game game;	
};


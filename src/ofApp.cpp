#include "ofApp.h"
#include <signal.h>

#define BUTTON_1 0
#define BUTTON_2 1
#define BUTTON_3 6
#define BUTTON_4 4

volatile int do_exit = 0;

void sigint(int sig)
{
    do_exit = 1;
}

volatile int button1 = 0;
volatile int button2 = 0;
volatile int button3 = 0;
volatile int button4 = 0;


void buttonHit1()
{
    //cout << "Button 1 Interrupt!" << endl;
    button1 = 1;
}

void buttonHit2() 
{
    //cout << "Button 2 Interrupt!" << endl;
    button2 = 1;
}

void buttonHit3()
{
    //cout << "Button 3 Interrupt!" << endl;
    button3 = 1;
}

void buttonHit4() 
{
    //cout << "Button 4 Interrupt!" << endl;
    button4 = 1;
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofBackground(0, 0, 0);                      // default background to black / LEDs off
    ofDisableAntiAliasing();                    // we need our graphics sharp for the LEDs
    ofSetVerticalSync(false);
    ofSetFrameRate(90);
    ofSetWindowTitle("Light Duel");

    signal(SIGINT, sigint);
    setupButtons();

    // SYSTEM SETTINGS
    //--------------------------------------
    stripWidth = 300;   // pixel width - keep this the logical length of the strip, not the actual pixels!!
    stripHeight = 1;    // pixel height of strip
    stripsPerPort = 8;                          // total number of strips per port
    numPorts = 1;                               // total number of teensy ports?
    brightness = 200;                             // LED brightness

    drawModes = 0;                              // default is demo mode
    demoModes = 0;                              // default is draw white

    dir = 1;

    #ifdef USE_TEENSY
    teensy.setup(stripWidth, stripHeight, stripsPerPort, numPorts);
    /* Configure our teensy boards (portName, xOffset, yOffset, width%, height%, direction) */
    teensy.serialConfigure("ttyACM0", 0, 0, 100, 100, 0);
    #endif

    // allocate our pixels, fbo, and texture
    fbo.allocate(stripWidth, stripHeight*stripsPerPort*numPorts*20, GL_RGB);

    setupMedia();

    curTime = ofGetElapsedTimeMillis();
    prevTime = curTime;

    game.Setup(buttons, this);
}

void ofApp::exit()
{

    /* turn all leds to black */
    fbo.begin();
    ofClear(0,0,0);
    fbo.end();

    updateTeensy();

    teensy.close();
    ofLogNotice() << "Exiting....";
}

//--------------------------------------------------------------
void ofApp::update()
{
    showFPS();
    if(do_exit == 1) {exit();std::exit(1);}

    updateButtons();

	

    curTime = ofGetElapsedTimeMillis();
    game.Update((curTime - prevTime) * .001f , buttons);
    prevTime = curTime;

    /*if(digitalRead(BUTTON_1) == 0) ofLog c() << "Button 1: Event counter: " << event_counter;
    if(digitalRead(BUTTON_2) == 0) ofLogNotice() << "Button 2: Event counter: " << event_counter;
    if(digitalRead(BUTTON_3) == 0) ofLogNotice() << "Button 3: Event counter: " << event_counter;
    if(digitalRead(BUTTON_4) == 0) ofLogNotice() << "Button 4: Event counter: " << event_counter;*/

    //ofLogNotice() << "Event counter: " << event_counter;
    //ofSetWindowTitle("TeensyOctoExample - "+ofToString(ofGetFrameRate()));
    ballpos+=dir*1.0f;

    if (dirVid.size() > 0)
    {
        if (videoOn) vid[currentVideo].update();                  // update video when enabled
    }

    updateFbo();                                // update our Fbo functions
    updateTeensy();

	// Listen for osc
	
}

//--------------------------------------------------------------
void ofApp::updateFbo()
{
    fbo.begin();                                // begins the fbo
    ofClear(0,0,0);							// refreshes fbo, removes artifacts

    switch (drawModes)
    {
        case 0:
            game.draw(fbo.getWidth(),fbo.getHeight());
            break;
        case 1:
            drawVideos();
            break;
        case 2:
            drawImages();
            break;
        case 3:
            drawDemos();
            break;
        case 4:
            drawPong();
            break;
        default:
            ofClear(0,0,0);
            break;
    }

    fbo.end();                                  // closes the fbo

}

//--------------------------------------------------------------
void ofApp::updateTeensy()
{
    #ifdef USE_TEENSY
    fbo.readToPixels(teensy.pixels1);           // send fbo pixels to teensy
    teensy.update();                            // update our serial to teensy stuff
    #endif
}

//--------------------------------------------------------------
void ofApp::draw()
{
    #ifdef TARGET_RASPBERRY_PI
    return;
    #endif

    bool bShowGamePixels =true;

    if(bShowGamePixels) {
        drawGamePixels();
    } else {
        drawRawPixels();
    }

}

void ofApp::drawGamePixels()
{
    ofDrawBitmapString("Use keys 1,2 & 3,4 to trigger lane buttons ",20,100);
    ofColor currentColour;
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(20,200);
    ofSetRectMode(OF_RECTMODE_CENTER);

    int pixelWidth = 3;

    ofSetColor(255);
    ofDrawBitmapString("LANE 0",0,-15);
    for (int y = 0; y < 2; y++)
    {
        for (int x = 0; x < stripWidth; x++)
        {
            ofPushMatrix();
            currentColour = teensy.pixels1.getColor(x, y);
            ofSetColor(currentColour);
            ofTranslate(x*pixelWidth, y*pixelWidth*2 );
            ofDrawRectangle(x, y, pixelWidth, pixelWidth*2);
            ofPopMatrix();
        }
    }

    ofTranslate(0,50);

    ofSetColor(255);
    ofDrawBitmapString("RING 0",0,5);
    for (int y = 2; y < 4; y++)
    {
        for (int x = 0; x < 30; x++)
        {
            ofPushMatrix();
            currentColour = teensy.pixels1.getColor(x, y);
            ofSetColor(currentColour);
            ofTranslate(x*pixelWidth, y*pixelWidth*2 );
            ofDrawRectangle(x, y, pixelWidth, pixelWidth*2);
            ofPopMatrix();
        }
    }

    ofTranslate(0,100);

    ofSetColor(255);
    ofDrawBitmapString("LANE 1",stripWidth*pixelWidth+250,15);
    for (int y = 4; y < 6; y++)
    {
        //for (int x = stripWidth-1; x >= 0; x--)
        for (int x = 0; x < stripWidth; x++)
        {
            ofPushMatrix();
            currentColour = teensy.pixels1.getColor(stripWidth -1 - x, y);
            ofSetColor(currentColour);
            ofTranslate(x*pixelWidth, y*pixelWidth*2 );
            ofDrawRectangle(x, y, pixelWidth, pixelWidth*2);
            ofPopMatrix();
        }
    }

    ofTranslate(0,50);

    ofSetColor(255);
    ofDrawBitmapString("RING 1",stripWidth*pixelWidth+250,30);
    for (int y = 6; y < 8; y++)
    {
        for (int x = 270; x < stripWidth; x++)
        {
            ofPushMatrix();
            currentColour = teensy.pixels1.getColor(x, y);
            ofSetColor(currentColour);
            ofTranslate(x*pixelWidth, y*pixelWidth*2 );
            ofDrawRectangle(x, y, pixelWidth, pixelWidth*2);
            ofPopMatrix();
        }
    }
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopStyle();
    ofPopMatrix();
}

void ofApp::drawRawPixels()
{
    ofColor currentColour;
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(20,200);
    ofSetRectMode(OF_RECTMODE_CENTER);

    int pixelWidth = 3;

    for (int y = 0; y < stripHeight*stripsPerPort; y++)
    {
        for (int x = 0; x < stripWidth; x++)
        {
            ofPushMatrix();
            currentColour = teensy.pixels1.getColor(x, y);
            ofSetColor(currentColour);
            ofTranslate(x*pixelWidth, y*pixelWidth*2 );
            ofDrawRectangle(x, y, pixelWidth, pixelWidth*2);
            ofPopMatrix();
        }
    }

    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopStyle();
    ofPopMatrix();
}

void ofApp::drawPong()
{
    if(ballpos > 290) {
        ballpos = 290;
        dir = -1;
    }
    else if(ballpos < 0) {
        ballpos = 0;
        dir = 1;
    }
    ofDrawRectangle(ballpos,0,10,stripHeight*stripsPerPort*numPorts);

}

void ofApp::drawDemos()
{
    switch (demoModes) {
        case 0:
            teensy.drawTestPattern();
            break;
        case 1:
            teensy.drawWhite();
            break;
        case 2:
            teensy.drawRainbowH();
            break;
        case 3:
            teensy.drawRainbowV();
            break;
        case 4:
            teensy.drawWaves();
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::enableVideo()
{
    if (dirVid.size() > 0) {
        if (!videoOn) videoOn = true;           // enables video
        if (vid[currentVideo].isLoaded() == false) {
            vid[currentVideo].load(dirVid.getPath(currentVideo));
            vid[currentVideo].play();           // plays the video
        }
        else {
            if (vid[currentVideo].isPlaying()) {
                vid[currentVideo].stop();       // stops/pauses the video
            }
            else {
                vid[currentVideo].play();       // plays the video
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::disableVideo()
{
    if (dirVid.size() > 0) {
        videoOn = false;                        // disables video
        if (vid[currentVideo].isPlaying()) vid[currentVideo].stop();  // stops/pauses the video
    }
}

//--------------------------------------------------------------
void ofApp::drawVideos()
{
    //Play videos
    if (dirVid.size() > 0){
        ofSetColor(brightness);
        vid[currentVideo].setSpeed(5.0f);
        vid[currentVideo].draw(0, 0, stripWidth, stripHeight*stripsPerPort*numPorts);
    }
}

//--------------------------------------------------------------
void ofApp::drawImages()
{
    if (dirImg.size() > 0) {
        ofSetColor(brightness);
        img[currentImage].draw(0, 0, stripWidth, stripHeight*stripsPerPort*numPorts);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    switch (key)
    {
        case '1':
            buttons[2].setState(1);
            break;
        case '2':
            buttons[3].setState(1);
            break;
        case '3':
            buttons[1].setState(1);
        break;
        case '4':
            buttons[0].setState(1);
        break;

        case OF_KEY_UP:
            brightness += 2;
            if (brightness > 255) brightness = 255;
            teensy.setBrightness(brightness);
            break;

        case OF_KEY_DOWN:
            brightness -= 2;
            if (brightness < 0) brightness = 0;
            teensy.setBrightness(brightness);
            break;

        case 'v':
            drawModes = 1;                          // video mode
            enableVideo();
            break;

        case 'i':
            drawModes = 2;                          // image mode
            disableVideo();

            img[currentImage].load(dirImg.getPath(currentImage));
            break;


        case '=':
            if (drawModes == 1) {
                vid[currentVideo].stop();
            }

            if (drawModes == 2) {
                currentImage++;
                if (currentImage > (int) dirImg.size()-1) currentImage = 0;
                img[currentImage].load(dirImg.getPath(currentImage));
            }
            break;

        case '-':
            if (drawModes == 1) {
                vid[currentVideo].stop();
            }

            if (drawModes == 2) {
                currentImage--;
                if (currentImage < 0) currentImage = (int) dirImg.size()-1;
                img[currentImage].load(dirImg.getPath(currentImage));
            }
            break;

        case 'd':
            disableVideo();

            demoModes++;
            if (drawModes != 3) drawModes = 3;      // switch the draw mode to display demo mode.
            if (demoModes > 4) demoModes = 0;       // tap through the demo modes on each press.
            break;

        case 't':
            disableVideo();

            if (drawModes != 0) drawModes = 0;      // switch the draw mode
            break;
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
    switch (key) {
        case '=':
            if (drawModes == 1)
            {
                currentVideo++;
                if (currentVideo > (int) dirVid.size()-1) currentVideo = 0;
                vid[currentVideo].load(dirVid.getPath(currentVideo));
                vid[currentVideo].play();
            }                       // restart video at first frame
            break;

        case '-':
            if (drawModes == 1) {
                currentVideo--;
                if (currentVideo < 0) currentVideo = (int) dirVid.size()-1;
                vid[currentVideo].load(dirVid.getPath(currentVideo));
                vid[currentVideo].play();
            }
            break;
        case '1':
            buttons[2].setState(0);
            break;
        case '2':
            buttons[3].setState(0);
            break;
        case '3':
            buttons[1].setState(0);
        break;
        case '4':
            buttons[0].setState(0);
        break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::setupMedia()
{
    ofLogNotice() << "Setting up media..." << endl;
    dirVid.listDir("videos/");
    dirVid.sort();
    //allocate the vector to have as many ofVidePlayer as files
    if( dirVid.size() ){
        vid.assign(dirVid.size(), ofVideoPlayer());
    }
    videoOn = false;
    currentVideo = 0;

    dirImg.listDir("images/");
    dirImg.sort();
    //allocate the vector to have as many ofImages as files
    if( dirImg.size() ){
        img.assign(dirImg.size(), ofImage());
    }
    currentImage = 0;
    ofLogNotice() << "Finished setting up media..." << endl;
}

//--------------------------------------------------------------
void ofApp::showFPS()
{
    if(ofGetFrameNum()%300 == 0 ) {
        ofLogNotice() << "FPS: " << ofToString(ofGetFrameRate());
    }
}

//--------------------------------------------------------------
void ofApp::setupButtons()
{
    for(int i = 0;i < 4;i++)
    {
        buttons[i].setId(i+1);
    }

#ifdef TARGET_RASPBERRY_PI

    if(wiringPiSetup() < 0) {
        ofLogError() << "Failed to init WiringPi lib";
    }

    pinMode(BUTTON_1,INPUT); // pin 17
    pinMode(BUTTON_2,INPUT); // pin 18
    pinMode(BUTTON_3,INPUT); // pin 23
    pinMode(BUTTON_4,INPUT); // pin 24
    pullUpDnControl(BUTTON_1,PUD_UP);
    pullUpDnControl(BUTTON_2,PUD_UP);
    pullUpDnControl(BUTTON_3,PUD_UP);
    pullUpDnControl(BUTTON_4,PUD_UP);
//    if(wiringPiISR( BUTTON_1, INT_EDGE_FALLING, &buttonHit1) < 0) {
//        ofLogError() << "Failed to set up interrupt 1" << endl;
//    }
//    if(wiringPiISR( BUTTON_2, INT_EDGE_FALLING, &buttonHit2) < 0) {
//        ofLogError() << "Failed to set up interrupt 2" << endl;
//    }
//    if(wiringPiISR( BUTTON_3, INT_EDGE_FALLING, &buttonHit3) < 0) {
//        ofLogError() << "Failed to set up interrupt 3" << endl;
//    }
//    if(wiringPiISR( BUTTON_4, INT_EDGE_FALLING, &buttonHit4) < 0) {
//        ofLogError() << "Failed to set up interrupt 4" << endl;
//    }
#endif
}

//--------------------------------------------------------------
void ofApp::updateButtons()
{
#ifdef TARGET_RASPBERRY_PI
    buttons[0].setState(digitalRead(BUTTON_1));
    buttons[1].setState(digitalRead(BUTTON_2));
    buttons[2].setState(digitalRead(BUTTON_3));
    buttons[3].setState(digitalRead(BUTTON_4));
#endif

    for(int i = 0;i < 4;i++)
    {
        buttons[i].update();
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}


// ----------------OSC--------------------------------
// Admin
string ofApp::_GetSettingsOSCAdd = "/lightduel/getsettings";
string ofApp::_SetSettingsOSCAdd = "/lightduel/setsettings";
string ofApp::_NewGameOSCAdd = "/lightduel/newgame";
string ofApp::_ResetGameOSCAdd = "/lightduel/resetgame";
string ofApp::_GameFinishedOSCAdd = "/lightduel/gamefinished";// Sent when game is over so we know we are ready for new users

// Events and states
string ofApp::_ButtonPressOSCAdd = "/lightduel/button";		//[int - laneIndex] [int - near/far 0/1]  [int - 0 for miss 1 for hit]
string ofApp::_RoundWonOSCAdd = "/lightduel/roundwon";		//[int - player index] [int - lane index] [int - rally length]
string ofApp::_GameWonOSCAdd = "/lightduel/gamewon";		//[int - player index]  [int - total rally length]
string ofApp::_Lane0PuckOSCAdd = "/lightduel/puck0"; //[float - normalized pos]
string ofApp::_Lane1PuckOSCAdd = "/lightduel/puck1"; //[float - normalized pos]

void ofApp::ListenForOSC()  // Gets  the games settings
{
	while (oscReceiver.hasWaitingMessages())
	{
		// get the next message
		ofxOscMessage m;
        oscReceiver.getNextMessage(m);

		if (m.getAddress() == _GetSettingsOSCAdd)
		{
			ofLogNotice() << "OSC ----- Get settings recieved:";

			ofxOscMessage m;
			m.setAddress(_SetSettingsOSCAdd);
			m.addFloatArg(game._LeftLane.m_Puck.m_Acceleration);
			m.addInt32Arg(game._RoundsPerGame);

			m.addFloatArg(game._NearPlayerCol.r);
			m.addFloatArg(game._NearPlayerCol.g);
			m.addFloatArg(game._NearPlayerCol.b);

			m.addFloatArg(game._FarPlayerCol.r);
			m.addFloatArg(game._FarPlayerCol.g);
			m.addFloatArg(game._FarPlayerCol.b);

			oscSender.sendMessage(m);
		}
		else if (m.getAddress() == _SetSettingsOSCAdd)
		{
			ofLogNotice() << "OSC ----- Set settings recieved:";

			game._LeftLane.m_Puck.m_Acceleration = m.getArgAsFloat(0);
			game._RightLane.m_Puck.m_Acceleration = m.getArgAsFloat(0);

			game._RoundsPerGame = m.getArgAsInt32(1);

			game._NearPlayerCol.r = m.getArgAsInt32(2);
			game._NearPlayerCol.g = m.getArgAsInt32(3);
			game._NearPlayerCol.b = m.getArgAsInt32(4);

			game._FarPlayerCol.r = m.getArgAsInt32(5);
			game._FarPlayerCol.g = m.getArgAsInt32(6);
			game._FarPlayerCol.b = m.getArgAsInt32(7);
		}
		else if (m.getAddress() == _NewGameOSCAdd)
		{
			ofLogNotice() << "OSC ----- New game recieved:";
			game.ResetGame();
		}
		else if (m.getAddress() == _ResetGameOSCAdd)
		{
			ofLogNotice() << "OSC ----- Reset game recieved:";
			game.ResetGame();
		}		
	}
}

void ofApp::SendGameFinished()	// sends when game is over and we return to idle
{
	ofxOscMessage m;
	m.setAddress(_GameFinishedOSCAdd);
	oscSender.sendMessage(m);
}

void ofApp::SendButtonPress(int laneIndex, int nearFar, int hitMiss)
{
	ofxOscMessage m;
	m.setAddress(_ButtonPressOSCAdd);
	m.addInt32Arg(laneIndex);
	m.addInt32Arg(nearFar);
	m.addInt32Arg(hitMiss);
	oscSender.sendMessage(m);
}

void ofApp::SendRoundWon(int playerIndex, int laneIndex, int rallyLength)
{
	ofxOscMessage m;
	m.setAddress(_RoundWonOSCAdd);
	m.addInt32Arg(playerIndex);
	m.addInt32Arg(laneIndex);
	m.addInt32Arg(rallyLength);
	oscSender.sendMessage(m);
}

void ofApp::SendGameWon(int playerIndex, int rallyLength)
{
	ofxOscMessage m;
	m.setAddress(_GameWonOSCAdd);
	m.addInt32Arg(playerIndex);
	m.addInt32Arg(rallyLength);
	oscSender.sendMessage(m);
}

void ofApp::SendPuckPositions(Puck p0, Puck p1) // sends the normalized puck positions
{
	ofxOscMessage m;
	m.setAddress(_Lane0PuckOSCAdd);
	m.addFloatArg(p0.m_NormalizedPosition);
	oscSender.sendMessage(m);

	ofxOscMessage m2;
	m2.setAddress(_Lane1PuckOSCAdd);
	m2.addFloatArg(p1.m_NormalizedPosition);
	oscSender.sendMessage(m2);
	
}

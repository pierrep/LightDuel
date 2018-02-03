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

void buttonHit1() {
    //cout << "Button 1 Interrupt!" << endl;
    button1 = 1;
}

void buttonHit2() {
    //cout << "Button 2 Interrupt!" << endl;
    button2 = 1;
}

void buttonHit3() {
    //cout << "Button 3 Interrupt!" << endl;
    button3 = 1;
}

void buttonHit4() {
    //cout << "Button 4 Interrupt!" << endl;
    button4 = 1;
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofBackground(0, 0, 0);                      // default background to black / LEDs off
    ofDisableAntiAliasing();                    // we need our graphics sharp for the LEDs
    ofSetVerticalSync(false);
    ofSetFrameRate(90);
    
    signal(SIGINT, sigint);
    setupButtons();
    
    // SYSTEM SETTINGS
    //--------------------------------------
    stripWidth = 276;                            // pixel width of strip
    stripHeight = 1;                            // pixel height of strip
    stripsPerPort = 8;                          // total number of strips per port
    numPorts = 1;                               // total number of teensy ports?
    brightness = 200;                             // LED brightness

    drawModes = 0;                              // default is demo mode
    demoModes = 0;                              // default is draw white

    dir = 1;
    
    #ifdef OF_TARGET_RASPI
    teensy.setup(stripWidth, stripHeight, stripsPerPort, numPorts);
    /* Configure our teensy boards (portName, xOffset, yOffset, width%, height%, direction) */
    teensy.serialConfigure("ttyACM0", 0, 0, 100, 100, 0);
    #endif
    
    // allocate our pixels, fbo, and texture
    fbo.allocate(stripWidth, stripHeight*stripsPerPort*numPorts, GL_RGB);
    
    setupMedia();
    
    curTime = ofGetElapsedTimeMillis();
    prevTime = curTime;

    game.Setup();
}

void ofApp::exit()
{
    /* turn all leds to black */
    fbo.begin();
    ofClear(0,0,0);
    fbo.end();

    updateTeensy();
}

//--------------------------------------------------------------
void ofApp::update()
{
    //showFPS();
    if(do_exit == 1) {exit();std::exit(1);}

    updateButtons();

    if(buttons[0].isPressedThisFrame()) {
         cout << "Button 0" << endl;
    } else if(buttons[1].isPressedThisFrame()) {
        cout << "Button 1" << endl;
    } else if(buttons[2].isPressedThisFrame()) {
        cout << "Button 2" << endl;
    } else if(buttons[3].isPressedThisFrame()) {
        cout << "Button 3" << endl;
    }

    curTime = ofGetElapsedTimeMillis();
    game.Update((curTime - prevTime) * .001f, buttons );
    prevTime = curTime;
	
    /*if(digitalRead(BUTTON_1) == 0) ofLogNotice() << "Button 1: Event counter: " << event_counter;
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

}

//--------------------------------------------------------------
void ofApp::updateFbo()
{    
    fbo.begin();                                // begins the fbo
    ofClear(0,0,0);                             // refreshes fbo, removes artifacts
    
    ofPushStyle();
    switch (drawModes)
    {
        case 0:            
            //drawPong();
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
        default:
            break;
    }
    ofPopStyle();
    
    fbo.end();                                  // closes the fbo

}

//--------------------------------------------------------------
void ofApp::updateTeensy()
{
    #ifdef OF_TARGET_RASPI
    fbo.readToPixels(teensy.pixels1);           // send fbo pixels to teensy
    teensy.update();                            // update our serial to teensy stuff
    #endif
}

//--------------------------------------------------------------
void ofApp::draw()
{
    #ifdef OF_TARGET_RASPI
    return;
    #endif

    /*
    teensy.draw(20,300);

    ofSetColor(255);
    ofDrawBitmapString("// Controls //", ofGetWidth()-250, 20);
    ofDrawBitmapString("Brightness (up/down) == " + ofToString(brightness), ofGetWidth()-250, 80);
    ofDrawBitmapString("Videos # == " + ofToString(dirVid.size()), ofGetWidth()-250, 120);
    ofDrawBitmapString("Images # == " + ofToString(dirImg.size()), ofGetWidth()-250, 140);
    */

    fbo.readToPixels(guiPixels);

    ofColor colors;
    ofPushMatrix();
    ofTranslate(0,200);
    ofSetRectMode(OF_RECTMODE_CENTER);

    for (int y = 0; y < stripHeight*stripsPerPort*numPorts; y++)
    {
        for (int x = 0; x < stripWidth; x++)
        {
            ofPushMatrix();
            colors = guiPixels.getColor(x, y);
            ofSetColor(colors);
            ofTranslate(x*2, y*2 + (y/16*4)); //sections in groups
            ofDrawRectangle(x, y, 2, 2);
            ofPopMatrix();
        }
    }
    ofSetRectMode(OF_RECTMODE_CORNER);
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
            buttons[0].setState(1);
            break;
        case '2':
            buttons[1].setState(1);
            break;
        case '3':
            buttons[2].setState(1);
        break;
        case '4':
            buttons[3].setState(1);
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
            buttons[0].setState(0);
            break;
        case '2':
            buttons[1].setState(0);
            break;
        case '3':
            buttons[2].setState(0);
        break;
        case '4':
            buttons[3].setState(0);
        break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::setupMedia()
{
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
}

//--------------------------------------------------------------
void ofApp::showFPS()
{

	curTime = ofGetElapsedTimeMillis();
	if(curTime - prevTime > 2000) {
        //ofLogNotice() << "FPS: " << ofToString(ofGetFrameRate());
		prevTime = curTime;

	}	

}

//--------------------------------------------------------------
void ofApp::setupButtons()
{
    for(int i = 0;i < 4;i++)
    {
        buttons[i].setId(i+1);
    }

#ifdef OF_TARGET_RASPI

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
#ifdef OF_TARGET_RASPI
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

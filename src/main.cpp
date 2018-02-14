#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	
	#ifdef TARGET_RASPBERRY_PI
    ofSetupOpenGL(200,200,OF_WINDOW);			
    #else
    ofSetupOpenGL(1280,800,OF_WINDOW);
    #endif

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}

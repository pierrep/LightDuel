#pragma once
#include "lane.h"
#include "button.h"
#include "ofxOsc.h"

class Game
{
    public:
        Game();

        enum state { idle, waitingToServe, inPlay, roundWon, gameWon };
		enum player { p1, p2 };

		// Game state
        state m_State;

		// Methods
        void Setup();
        void Update( float deltaTime, Button buttons[] );
        void ResetGame();
        void DrawRings(int gameWidth);

		void ButtonPressed(int btnIndex, bool hitPuck);
		//bool PuckHit(Lane lane); TODO add this ot clean up the player input section

        void SetState( state state );

        void p1KeyPressDebug();
        void p2KeyPressDebug();

        void draw(int width, int height);

		// Lane definitions
        Lane lane1;
        Lane lane2;

		// Colours
        ofColor m_WinningColor;
        ofColor m_P1Color;
        ofColor m_P2Color;
		ofColor m_IdleColor;

		// Player scores per game
        int m_P1Score;
        int m_P2Score;
        int m_RoundPerGame;

		// Durations and timers for states 
        float m_RoundWinDuration;
        float m_GameWinDuration;
        float m_ServeDuration;
        float m_StateTimer;		
		float m_StateNormTime;	// normalized time passed during state. Used to control animation effects


		// TODO change serving to an enum so its easier to read
        bool m_P1Serve = true;
		player _Serving;
     
		// OSC sender
        ofxOscSender oscSender;

		// TODO: Osc reciever
		ofxOscReceiver oscReciever;
};


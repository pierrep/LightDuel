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
        void Setup(Button buttons[]);
        void Update(float deltaTime, Button buttons[]);
        void ResetGame();
        void DrawRings(int gameWidth);

		void ButtonPressed(int btnIndex, bool hitPuck);		

        void SetState( state state );

        void draw(int width, int height);

		// Lane definitions
        Lane _LeftLane;
        Lane _RightLane;

		// Colours
        ofColor m_WinningColor;
        ofColor _NearPlayerCol;
        ofColor _FarPlayerCol;
		ofColor m_IdleColor;

		// Player scores per game
        int _NearPlayerScore;
        int _FarPlayerScore;
        int _RoundsPerGame;

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
		ofxOscReceiver oscReceiver;
};


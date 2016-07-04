#pragma once
#include "lane.h"
#include "button.h"
#include "ofxOsc.h"

class Game
{
    public:
        Game();

        enum state { idle, waitingToServe, inPlay, roundWon, gameWon };

        state m_State;

        void setup();
        void update( float deltaTime, Button buttons[] );
        void ResetGame();
        void DrawRings(int gameWidth);

        void SetState( state state );

        void p1KeyPressDebug();
        void p2KeyPressDebug();

        void draw(int width, int height);
        Lane lane1;
        Lane lane2;

        ofColor m_WinningColor;
        ofColor m_P1Color;
        ofColor m_P2Color;

        int m_P1Score;
        int m_P2Score;
        int m_TargetScore;

        float m_RoundWinDuration;
        float m_GameWinDuration;
        float m_ServeDuration;
        float m_StateTimer;

        bool m_P1Serve = true;


        // normalized time passed during state. Used to control animation effects
        float m_StateNormTime;

        ofxOscSender oscSender;


};


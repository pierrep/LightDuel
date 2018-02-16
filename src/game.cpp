#include "game.h"
#include "button.h"
#include "ofApp.h"

Game::Game()
{
    // debuggin installation with no button presses
    m_State = idle;

    _RoundsPerGame = 3;
    _NearPlayerScore = 0;
    _FarPlayerScore = 0;

     m_RoundWinDuration = 4;
     m_GameWinDuration = 6;
     m_ServeDuration = 2;
     m_StateTimer = 0;

	 _LeftLane._Index = 0;
	 _RightLane._Index = 1;

     _NearPlayerCol = ofColor::blue;
     _FarPlayerCol = ofColor::yellow;

#ifdef TARGET_RASPBERRY_PI
     _RightLane.m_Strip1YIndex = 0;
     _RightLane.m_Strip2YIndex = 1;
     _LeftLane.m_Strip1YIndex = 4;
     _LeftLane.m_Strip2YIndex = 5;
#else
     _RightLane.m_Strip1YIndex = 1;
     _RightLane.m_Strip2YIndex = 2;
     _LeftLane.m_Strip1YIndex = 5;
     _LeftLane.m_Strip2YIndex = 6;
#endif

	 _RightLane._LaneDrawFlipped = true;
}

void Game::Setup(Button buttons[], ofApp* ofApp)
{
    _RightLane.Init(&buttons[0], &buttons[3], _NearPlayerCol,_FarPlayerCol, this);
	_LeftLane.Init(&buttons[1], &buttons[2], _NearPlayerCol, _FarPlayerCol, this);

	_ofApp = ofApp;
}

void Game::Update(float frameTime, Button buttons[])
{
    if( m_State == inPlay )
    {		
		// Update lanes        
        _RightLane.update(frameTime);
        _LeftLane.update(frameTime);

		// OSC for pucks
        _ofApp->SendPuckPositions(_LeftLane.m_Puck, _RightLane.m_Puck);

		// Check wins
        if(_LeftLane._NearScoredThisFrame || _RightLane._NearScoredThisFrame)
        {
            m_WinningColor = _NearPlayerCol;
            _NearPlayerScore++;

			if (_NearPlayerScore >= _RoundsPerGame)
			{
                _ofApp->SendGameWon(0, _RallyLength);

				SetState(gameWon);
			}
			else
			{
				if (_LeftLane._NearScoredThisFrame)
                    _ofApp->SendRoundWon(0, 0, _RallyLength);
				else
                    _ofApp->SendRoundWon(0, 1, _RallyLength);

				_NearServe = true;

				SetState(roundWon);
			}
        }
        else if(_LeftLane._FarScoredThisFrame || _RightLane._FarScoredThisFrame)
        {
             m_WinningColor = _FarPlayerCol;
            _FarPlayerScore++;

			if (_FarPlayerScore >= _RoundsPerGame)
			{
                _ofApp->SendGameWon(1, _RallyLength);


				SetState(gameWon);
			}
			else
			{
				if (_LeftLane._NearScoredThisFrame)
                    _ofApp->SendRoundWon(1, 0, _RallyLength);
				else
                    _ofApp->SendRoundWon(1, 1, _RallyLength);

				_NearServe = false;

				SetState(roundWon);
			}
        }       
    }
    else if( m_State == waitingToServe )
    {
        m_StateTimer += frameTime;

        m_StateNormTime = m_StateTimer/ m_ServeDuration;

        if( m_StateTimer > m_ServeDuration)
            SetState( inPlay );
    }
    else if( m_State == roundWon )
    {
        m_StateTimer += frameTime;

        m_StateNormTime = m_StateTimer/ m_RoundWinDuration;

        if( m_StateTimer > m_RoundWinDuration)
            SetState( waitingToServe );
    }
    else if( m_State == gameWon )
    {
        m_StateTimer += frameTime;

        m_StateNormTime = m_StateTimer/ m_GameWinDuration;

        if( m_StateTimer > m_GameWinDuration)
        {
            SetState( idle );
        }
    }
    else if( m_State == idle )
    {
        if(ofGetFrameNum()%180 == 0) {
            _ofApp->SendGameFinished();
        }

        // Check if enough buttons are held down to transition out of idle state
        int buttonsDown = 0;

        for( int i = 0; i < 4; i++ ) // just put in 4 because buttons.Length doesnt work was quicker than googling
        {
            if( buttons[i].m_IsDown )
                buttonsDown++;
        }

        if( buttonsDown >= 2 )
        {
            ResetGame();
            SetState( waitingToServe );
        }
    }
}

void Game::ButtonPressed(int laneIndex, int nearFar, int hitMiss)
{
    _ofApp->SendButtonPress(laneIndex, nearFar, hitMiss);
	
}

void Game::SetState( state state )
{
    m_State = state;

    if( m_State == idle )
    {
        // Wait for button presses
        ResetGame();
    }
    else if( m_State == waitingToServe )
    {
        m_StateTimer = 0;

        _LeftLane.Reset();
        _RightLane.Reset();

        if( _NearServe )
        {
            _LeftLane.m_Puck.ResetToStart();
			_RightLane.m_Puck.ResetToStart();        
        }
        else
        {
            _LeftLane.m_Puck.ResetToEnd();
            _RightLane.m_Puck.ResetToEnd();          
        }
    }
    else if( m_State == inPlay )
    {

    }
    else if( m_State == roundWon )
    {
        m_StateTimer = 0;
    }
    else if( m_State == gameWon )
    {
        m_StateTimer = 0;
		
    }

}

void Game::ResetGame()
{
    _NearPlayerScore = 0;
    _FarPlayerScore = 0;

    _LeftLane._NearScoredThisFrame = false;
    _LeftLane._FarScoredThisFrame = false;

    _RightLane._NearScoredThisFrame = false;
    _RightLane._FarScoredThisFrame = false;


    _LeftLane.m_Puck.ResetToStart();
    _RightLane.m_Puck.ResetToStart();

     _NearServe = true;
}

void Game::DrawRings(int gameWidth)
{
#ifdef TARGET_RASPBERRY_PI
    // Draw rings
    ofSetColor(_NearPlayerCol);
    ofDrawLine(0,6,gameWidth,6);
    ofDrawLine(0,7,gameWidth,7);

    // Draw rings
    ofSetColor(_FarPlayerCol);
    ofDrawLine(0,2,gameWidth,2);
    ofDrawLine(0,3,gameWidth,3);

#else
    // Draw rings
    ofSetColor(_NearPlayerCol);
    ofDrawLine(0,7,gameWidth,7);
    ofDrawLine(0,8,gameWidth,8);

    // Draw rings
    ofSetColor(_FarPlayerCol);
    ofDrawLine(0,3,gameWidth,3);
    ofDrawLine(0,4,gameWidth,4);
#endif
}


void Game::draw(int gameWidth, int gameHeight)
{

    if( m_State == idle )
    {
        ofSetColor(ofColor::black);
		ofDrawRectangle(0,0,gameWidth,gameHeight);
    }
    else if( m_State == waitingToServe )
    {
        ofSetColor(ofColor::purple * (1- m_StateNormTime));
        ofDrawRectangle(0,0,gameWidth,gameHeight);
    }
    else if( m_State == inPlay )
    {
        _LeftLane.draw(gameWidth, gameHeight);
        _RightLane.draw(gameWidth, gameHeight);
    }
    else if( m_State == roundWon )
    {
        ofSetColor(m_WinningColor * sin( m_StateNormTime * 20 ));
        ofDrawRectangle(0,0,gameWidth,gameHeight);
    }
    else if( m_State == gameWon )
    {
        ofSetColor(m_WinningColor * sin( m_StateNormTime * 30 ));
        ofDrawRectangle(0,0,gameWidth,gameHeight);
    }
    
    DrawRings(gameWidth);

}


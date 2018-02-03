#include "game.h"
#include "button.h"


#define HOST "127.0.0.1"
#define PORT 10001

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

     _RightLane.m_Strip1YIndex = 0;
     _RightLane.m_Strip2YIndex = 1;

	 // Pass the lanes a ref to the game
	// _LeftLane._Game = this;
	// _RightLane._Game = this;
	 
     _NearPlayerCol = ofColor::blue;
     _FarPlayerCol = ofColor::yellow;
}

void Game::Setup(Button buttons[])
{
	// Setup OSC
    oscSender.setup(HOST,PORT);
	oscReciever.setup(PORT);
}

void Game::Update(float frameTime, Button buttons[])
{
    if( m_State == inPlay )
    {
		// Update lanes
        _LeftLane.update(frameTime, buttons[0], buttons[3]);
        _RightLane.update(frameTime, buttons[1], buttons[2]);

		// Check wins
        if(_LeftLane._NearScoredThisFrame || _RightLane._NearScoredThisFrame)
        {
            m_WinningColor = _NearPlayerCol;
            _NearPlayerScore++;

            if( _NearPlayerScore >= _RoundsPerGame)	SetState( gameWon );
            else									SetState( roundWon );
        }
        else if(_LeftLane._FarScoredThisFrame || _RightLane._FarScoredThisFrame)
        {
             m_WinningColor = _FarPlayerCol;
            _FarPlayerScore++;

            if( _FarPlayerScore >= _RoundsPerGame)	SetState(gameWon);              
            else									SetState(roundWon);                
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

void Game::ButtonPressed(int btnIndex, bool hitPuck)
{
	// concat address string using button index
	string address = "/button" + btnIndex;

	ofxOscMessage m;
	m.setAddress(address);

	if (hitPuck)
	{
		m.addIntArg(1);
	}
	else if(!hitPuck)
	{
		m.addIntArg(0);
	}
	
	oscSender.sendMessage(m);
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

       // if( m_P1Serve )
      //  {
            _LeftLane.m_Puck.ResetToStart();
            _RightLane.m_Puck.ResetToEnd();
            m_P1Serve = false;
      //  }
      //  else
      //  {
       //     lane1.m_Puck.ResetToEnd();
       //     lane2.m_Puck.ResetToStart();
       //     m_P1Serve = true;
       // }
    }
    else if( m_State == inPlay )
    {

    }
    else if( m_State == roundWon )
    {
        m_StateTimer = 0;\
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
    _RightLane.m_Puck.ResetToEnd();
     m_P1Serve = false;
}

void Game::DrawRings( int gameWidth)
{
    // Draw rings
    ofSetColor(_NearPlayerCol);
    ofDrawLine(0,7,gameWidth,7);
    ofDrawLine(0,6,gameWidth,6);

    // Draw rings
    ofSetColor(_FarPlayerCol);
    ofDrawLine(0,2,gameWidth,2);
    ofDrawLine(0,3,gameWidth,3);
}

void Game::draw(int gameWidth, int gameHeight)
{
    if( m_State == idle )
    {
        for( int i = 0; i < 8; i++ )
        {
            ofSetColor(ofColor::white);
            ofDrawLine(0,i,gameWidth,i);
        }

       DrawRings(gameWidth);
    }
    else if( m_State == waitingToServe )
    {
        for( int i = 0; i < 8; i++ )
        {
            ofSetColor(ofColor::purple * (1- m_StateNormTime));
            ofDrawLine(0,i,gameWidth,i);
        }

        DrawRings(gameWidth);
    }
    else if( m_State == inPlay )
    {
        _LeftLane.draw(gameWidth, gameHeight);
        _RightLane.draw(gameWidth, gameHeight);


        DrawRings(gameWidth);

    }
    else if( m_State == roundWon )
    {

        for( int i = 0; i < 8; i++ )
        {
             // TODO: change to player who wons color
            ofSetColor(m_WinningColor * sin( m_StateNormTime * 20 ));
            ofDrawLine(0,i,gameWidth,i);
        }

       DrawRings(gameWidth);
    }
    else if( m_State == gameWon )
    {
        for( int i = 0; i < 8; i++ )
        {
            // TODO: change to player who wons color
            ofSetColor(m_WinningColor * sin( m_StateNormTime * 30 ));
            ofDrawLine(0,i,gameWidth,i);
        }

       DrawRings(gameWidth);
    }
}

#include "game.h"
#include "button.h"

Game::Game()
{
   // m_State =idle;

    // debuggin installation with no button presses
    m_State = idle;

    m_TargetScore = 3;
    m_P1Score = 0;
    m_P2Score = 0;

     m_RoundWinDuration = 4;
     m_GameWinDuration = 6;
     m_ServeDuration = 2;
     m_StateTimer = 0;

     lane2.m_Strip1YIndex = 0;
     lane2.m_Strip2YIndex = 1;
     lane1.m_FlipRreturnZoneColouring = true;

     m_P1Color = ofColor::blue;
     m_P2Color = ofColor::yellow;
}


void Game::update(float frameTime, Button buttons[] )
{

    if( m_State == inPlay )
    {

        // Update player 1 inputs
        if( buttons[0].isPressedThisFrame() )
        {
            if( lane2.m_Puck.m_NormalizedPosition + (lane2.m_Puck.m_NormalizedWidth/2.0f) > 1 - lane2.m_ReturnZoneNormalized )
            {
                if( lane2.m_Puck.m_Direction == 1 )
                    lane2.m_Puck.ReturnPuck();

            }
        }
        if( buttons[1].isPressedThisFrame() )
        {
            if( lane1.m_Puck.m_NormalizedPosition - (lane1.m_Puck.m_NormalizedWidth/2.0f) < lane1.m_ReturnZoneNormalized )
            {
                if( lane1.m_Puck.m_Direction == -1 )
                    lane1.m_Puck.ReturnPuck();

            }
        }

        if( buttons[2].isPressedThisFrame() )
        {
            if( lane1.m_Puck.m_NormalizedPosition + (lane1.m_Puck.m_NormalizedWidth/2.0f) > 1 - lane1.m_ReturnZoneNormalized )
            {
                if( lane1.m_Puck.m_Direction == 1 )
                    lane1.m_Puck.ReturnPuck();

            }
        }
        if( buttons[3].isPressedThisFrame() )
        {
            if( lane2.m_Puck.m_NormalizedPosition - (lane2.m_Puck.m_NormalizedWidth/2.0f) <  lane2.m_ReturnZoneNormalized )
            {
                if( lane2.m_Puck.m_Direction == -1 )
                    lane2.m_Puck.ReturnPuck();

            }
        }


        lane1.update(frameTime);
        lane2.update(frameTime);

        if( lane1.m_P1Win )
        {
            // reset win state
            m_WinningColor = m_P1Color;

            m_P1Score++;

            if( m_P1Score >= m_TargetScore)
                SetState( gameWon );
            else
                SetState( roundWon );

        }
        else if( lane1.m_P2Win )
        {
             m_WinningColor = m_P2Color;

            m_P2Score++;

            if( m_P2Score >= m_TargetScore)
                SetState( gameWon );
            else
                SetState( roundWon );
        }
        else if( lane2.m_P1Win )
        {
             m_WinningColor = m_P1Color;

            m_P1Score++;

            if( m_P1Score >= m_TargetScore)
                SetState( gameWon );
            else
                SetState( roundWon );

        }
        else if( lane2.m_P2Win )
        {
             m_WinningColor = m_P2Color;

            m_P2Score++;

            if( m_P2Score >= m_TargetScore)
                SetState( gameWon );
            else
                SetState( roundWon );
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

        lane1.Reset();
        lane2.Reset();

       // if( m_P1Serve )
      //  {
            lane1.m_Puck.ResetToStart();
            lane2.m_Puck.ResetToEnd();
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
    m_P1Score = 0;
    m_P2Score = 0;

    lane1.m_P1Win = false;
    lane1.m_P2Win = false;

    lane2.m_P1Win = false;
    lane2.m_P2Win = false;


    lane1.m_Puck.ResetToStart();
    lane2.m_Puck.ResetToEnd();
     m_P1Serve = false;
}

void Game::p1KeyPressDebug()
{
    lane1.P1ButtonPress();
    lane2.P1ButtonPress();

    if( m_State == idle )
    {
        SetState( waitingToServe );
    }
    else if( m_State == waitingToServe )
    {
        SetState( inPlay );
    }
}

void Game::p2KeyPressDebug()
{
    lane1.P2ButtonPress();
    lane2.P2ButtonPress();

    if( m_State == idle )
    {
        SetState( waitingToServe );
    }
    else if( m_State == waitingToServe )
    {
        SetState( inPlay );
    }
}

void Game::DrawRings( int gameWidth)
{
    // Draw rings
    ofSetColor(m_P1Color);
    ofDrawLine(0,7,gameWidth,7);
    ofDrawLine(0,6,gameWidth,6);

    // Draw rings
    ofSetColor(m_P2Color);
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
        lane1.draw(gameWidth, gameHeight);
        lane2.draw(gameWidth, gameHeight);


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

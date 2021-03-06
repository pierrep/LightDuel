#include "lane.h"
#include "game.h"

Lane::Lane()
{
    m_PixelLength = 280;
    m_ReturnZoneNormalized = 0.2f;

    _NearScoredThisFrame = false;
    _FarScoredThisFrame = false;
}

/*
void Lane::init( ofColor returnzone1col, ofColor returnzone2col, int strip1Y, int strip2Y, bool flipReturnCol, Button nearBtn, Button farBtn )
{
    _NearReturnZoneBaseCol = returnzone1col;
    _FarReturnZoneBaseCol = returnzone2col;

    m_Strip1YIndex = strip1Y;
    m_Strip2YIndex = strip2Y;

    m_ReturnZonePixelLength = m_PixelLength * m_ReturnZoneNormalized;

    m_LaneFlipped = flipReturnCol;

	_NearButton = nearBtn;
	_FarButton = farBtn;
}
*/

void Lane::Init( Button* nearBtn, Button* farBtn,ofColor NearPlayerCol, ofColor FarPlayerCol, Game* game)
{
	_NearButton = nearBtn;
	_FarButton = farBtn;
    _NearPlayerCol = NearPlayerCol;
    _FarPlayerCol = FarPlayerCol;
	_Game = game;
}

void Lane::Reset( )
{
    _NearScoredThisFrame = false;
    _FarScoredThisFrame = false;
    m_ReturnZoneNormalized = .2f;
}

void Lane::update(float frameTime)
{
	// Updates the puck position
    m_Puck.update(frameTime);
		
	// Check to see if the puck has moved outside lane == WIN
    if(m_Puck.m_NormalizedPosition < 0 )
    {
		_FarScoredThisFrame = true;
        ofLogNotice() << "Far scored this frame";
    }
    else if( m_Puck.m_NormalizedPosition > 1)
    {
		_NearScoredThisFrame = true;
        ofLogNotice() << "Near scored this frame";
    }	
    else
    {
		// Check to see if buttons are pressed within the normalized zones
		// Player 1 is on the 0 end of the normalized strip 
        if(_NearButton->isPressedThisFrame())
        {
			if (m_Puck.m_NormalizedPosition - (m_Puck.m_NormalizedWidth / 2.0f) < m_ReturnZoneNormalized && m_Puck.m_Direction < 0)
			{
				ofLogNotice() << "Near return on lane:" << _Index;

				m_Puck.ReturnPuck();

                _Game->ButtonPressed(_Index, 0, 1);
			}
			else
			{
                _Game->ButtonPressed(_Index, 0, 0);
			}
        }

		// Player 2 is on the 1 side of the normalized strip
        if(_FarButton->isPressedThisFrame())
        {			
			if (m_Puck.m_NormalizedPosition + (m_Puck.m_NormalizedWidth / 2.0f) > 1 - m_ReturnZoneNormalized && m_Puck.m_Direction > 0)
			{
				ofLogNotice() << "Far return on lane:" << _Index;
				m_Puck.ReturnPuck();

                _Game->ButtonPressed(_Index, 1, 1);
			}
			else
			{
                _Game->ButtonPressed(_Index, 1, 0);
			}				
        }
    }
}

void Lane::draw(int w, int h)
{
    if(_LaneDrawFlipped)
    {
        // Draw strip 1
        ofSetColor(_FarPlayerCol * .7f);
        ofDrawLine(0,m_Strip1YIndex,m_ReturnZoneNormalized * w,m_Strip1YIndex);

        ofSetColor(_NearPlayerCol * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip1YIndex,1.0f * w,m_Strip1YIndex);

        // draw strip 2
        ofSetColor(_FarPlayerCol * .7f);
        ofDrawLine(0,m_Strip2YIndex,m_ReturnZoneNormalized * w,m_Strip2YIndex);

        ofSetColor(_NearPlayerCol * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip2YIndex,1.0f * w,m_Strip2YIndex);
    }
    else
    {
        // Draw strip 1
        ofSetColor(_NearPlayerCol * .7f);
        ofDrawLine(0,m_Strip1YIndex,m_ReturnZoneNormalized * w,m_Strip1YIndex);

        ofSetColor(_FarPlayerCol * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip1YIndex,1.0f * w,m_Strip1YIndex);

        // draw strip 2
        ofSetColor(_NearPlayerCol * .7f);
        ofDrawLine(0,m_Strip2YIndex,m_ReturnZoneNormalized * w,m_Strip2YIndex);

        ofSetColor(_FarPlayerCol * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip2YIndex,1.0f * w,m_Strip2YIndex);
    }

	// Draw pucks
    m_Puck.draw(w,h, m_Strip1YIndex, _LaneDrawFlipped);
    m_Puck.draw(w,h, m_Strip2YIndex, _LaneDrawFlipped);
}

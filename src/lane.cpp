#include "lane.h"

Lane::Lane()
{
    m_PixelLength = 280;
    m_ReturnZoneNormalized = 0.2f;
    m_ReturnZonePlayer1BaseColor = ofColor::blue;
    m_ReturnZonePlayer2BaseColor = ofColor::yellow;
    m_Strip1YIndex = 4;
    m_Strip2YIndex = 5;

    m_P1Win = false;
    m_P2Win = false;

}

void Lane::init(  ofColor returnzone1col, ofColor returnzone2col, int strip1Y, int strip2Y, bool flipReturnCol  )
{
    m_ReturnZonePlayer1BaseColor = returnzone1col;
    m_ReturnZonePlayer2BaseColor = returnzone2col;

    m_Strip1YIndex = strip1Y;
    m_Strip2YIndex = strip2Y;

    m_ReturnZonePixelLength = m_PixelLength * m_ReturnZoneNormalized;

    _LaneFlipped = flipReturnCol;
}

void Lane::P1ButtonPress()
{
    if( (m_Puck.m_NormalizedPosition < m_ReturnZoneNormalized ))
        m_Puck.ReturnPuck();
}

void Lane::P2ButtonPress()
{
    if( (m_Puck.m_NormalizedPosition > 1 - m_ReturnZoneNormalized ))
        m_Puck.ReturnPuck();
}

void Lane::Reset( )
{
    m_P1Win = false;
    m_P2Win = false;
    m_ReturnZoneNormalized = .2f;
}

void Lane::update(float frameTime )
{
    m_Puck.update(frameTime);
    m_P1Button.update();
    m_P2Button.update();

    if(m_Puck.m_NormalizedPosition < 0 )
    {
		if (_LaneFlipped)	m_P1Win = true;
		else 				m_P2Win = true;
    }
    else if( m_Puck.m_NormalizedPosition > 1)
    {
		if (_LaneFlipped)	m_P2Win = true;
		else 				m_P1Win = true;
    }
    else
    {
        if(m_P1Button.isPressedThisFrame() )
        {
            P1ButtonPress();
        }
        else if(m_P2Button.isPressedThisFrame() )
        {
            P2ButtonPress();
        }
    }
}

void Lane::draw(int w, int h)
{
    if( _LaneFlipped)
    {
        // Draw strip 1
        ofSetColor(m_ReturnZonePlayer1BaseColor * .7f);
        ofDrawLine(0,m_Strip1YIndex,m_ReturnZoneNormalized * w,m_Strip1YIndex);

        ofSetColor(m_ReturnZonePlayer2BaseColor * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip1YIndex,1.0f * w,m_Strip1YIndex);

        // draw strip 2
        ofSetColor(m_ReturnZonePlayer1BaseColor * .7f);
        ofDrawLine(0,m_Strip2YIndex,m_ReturnZoneNormalized * w,m_Strip2YIndex);

        ofSetColor(m_ReturnZonePlayer2BaseColor * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip2YIndex,1.0f * w,m_Strip2YIndex);
    }
    else
    {
        // Draw strip 1
        ofSetColor(m_ReturnZonePlayer2BaseColor * .7f);
        ofDrawLine(0,m_Strip1YIndex,m_ReturnZoneNormalized * w,m_Strip1YIndex);

        ofSetColor(m_ReturnZonePlayer1BaseColor * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip1YIndex,1.0f * w,m_Strip1YIndex);

        // draw strip 2
        ofSetColor(m_ReturnZonePlayer2BaseColor * .7f);
        ofDrawLine(0,m_Strip2YIndex,m_ReturnZoneNormalized * w,m_Strip2YIndex);

        ofSetColor(m_ReturnZonePlayer1BaseColor * .7f);
        ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,m_Strip2YIndex,1.0f * w,m_Strip2YIndex);
    }


    m_Puck.draw(w,h, m_Strip1YIndex);
    m_Puck.draw(w,h, m_Strip2YIndex);

/*
    ofSetColor(m_ReturnZonePlayer1BaseColor);
    ofDrawLine(0,1,m_ReturnZoneNormalized * w,1);

    ofSetColor(m_ReturnZonePlayer2BaseColor);
    ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,1,1 * w,1);



//2
    ofSetColor(m_ReturnZonePlayer1BaseColor);
    ofDrawLine(0,2,m_ReturnZoneNormalized * w,2);

    ofSetColor(m_ReturnZonePlayer2BaseColor);
    ofDrawLine((1.0f - m_ReturnZoneNormalized) * w,2,1 * w,2);





/* Test Y indexes
    ofSetColor(ofColor::blue);
    ofDrawLine(0,1,w,1);

    ofSetColor(ofColor::green);
    ofDrawLine(0,2,w,2);

    ofSetColor(ofColor::yellow);
    ofDrawLine(0,3,w,3);

    ofSetColor(ofColor::red);
    ofDrawLine(0,4,w,4);

    ofSetColor(ofColor::blue);
    ofDrawLine(0,5,w,5);

    ofSetColor(ofColor::yellow);
    ofDrawLine(0,6,w,6);

    ofSetColor(ofColor::green);
    ofDrawLine(0,7,w,7);

    ofSetColor(ofColor::red);
    ofDrawLine(0,8,w,8);
    */

}

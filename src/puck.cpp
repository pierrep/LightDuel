#include "puck.h"

Puck::Puck()
{
    m_Direction = 1;
    m_StartSpeed = .2f;
    m_CurrentSpeed = .2f;
    m_Acceleration = .05f;
    m_NormalizedPosition = .1f;
    m_NormalizedWidth = 0.05f;
    m_IsDead = false;

}

void Puck::setup( ofColor col)
{
    m_Color = col;
}

void Puck::ReturnPuck()
{
    if( m_Direction == 1)
        m_Direction = -1;
    else
        m_Direction = 1;

    // Increase speed
    m_CurrentSpeed += m_Acceleration;
}


void Puck::update(float frameTime)
{
    // If the puck is still in play then update it
    if( !m_IsDead)
    {
        m_NormalizedPosition += m_CurrentSpeed * m_Direction * frameTime;
    }
}

void Puck::ResetToStart()
{
    m_NormalizedPosition = .05f;
    m_Direction = 1;
    m_IsDead = false;
    m_CurrentSpeed = m_StartSpeed;

}

void Puck::ResetToEnd()
{
    m_NormalizedPosition = .95f;
    m_Direction = -1;
    m_IsDead = false;
    m_CurrentSpeed = m_StartSpeed;
}

void Puck::draw(int w, int h, int yRow, bool flipped )
{
    ofSetColor(ofColor::white);

	float norm = m_NormalizedPosition;

	if (flipped) norm = 1 - m_NormalizedPosition;

    ofDrawLine( (norm - (m_NormalizedWidth/2.0f)) * w,yRow,(norm + (m_NormalizedWidth/2.0f)) * w,yRow);
    //ofDrawLine( .4f * w,yRow, .6f * w,yRow);
}



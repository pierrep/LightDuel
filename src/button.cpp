#include "button.h"


Button::Button()
{
    m_CanPressDown = true;
    m_IsDown = false;
    m_CoolDownDuration = 500;
    m_CoolDownTimer = 0;
    curTime = ofGetElapsedTimeMillis();
    prevTime = curTime;
    polledState = 1;
    m_PressedDownThisFrame = false;
    id = 0;
}

void Button::update()
{
    m_PressedDownThisFrame = false;

    curTime = ofGetElapsedTimeMillis();
    float deltaTime = curTime - prevTime;
    prevTime = curTime;

    m_CoolDownTimer += deltaTime; // accumulate time since last frame/update

    // Check for button up
    if( m_IsDown && (polledState == 1) ) {
        m_IsDown = false;
    }

    if( !m_CanPressDown)
    {
        cout << "m_CoolDownTimer: " << m_CoolDownTimer << endl;
        // Test for conditions so button has been lifted
         if( (polledState == 1) && (m_CoolDownTimer > m_CoolDownDuration) )
        {
            m_CanPressDown = true;
            m_IsDown = false;
        }
    }


    if( (polledState == 0)  && m_CanPressDown )
    {
        ButtonPressed();
    }
}

void Button::ButtonPressed()
{
    m_PressedDownThisFrame = true;
    m_CanPressDown = false;
    m_CoolDownTimer = 0;
    m_IsDown = true;
    ofLogNotice() << "Button Pressed:" << id;
    polledState = 1;
}

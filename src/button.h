#pragma once
#include "ofMain.h"

class Button
{
    public:
        Button();
        void update();
        void ButtonPressed();
        void setState(int poll) { polledState = poll;}
        void setId(int _id) {id = _id;}
        bool isDown() {return m_IsDown;}
        bool isPressedThisFrame() {return m_PressedDownThisFrame;}

        bool m_CanPressDown;
        bool m_IsDown;
        float m_CoolDownDuration;
        float m_CoolDownTimer;
        bool m_PressedDownThisFrame;

        float curTime;
        float prevTime;
        int polledState;
        int id;
};


#pragma once
#include "ofMain.h"

class Puck
{
    public:
        Puck();

        float m_Direction = 1;
        float m_StartSpeed = .1f;    // Speed that it starts at at the beginning of each round
        float m_CurrentSpeed = .1f;
        float m_AccelerationPerHit = .05f;

        bool m_IsDead = false;

        float m_NormalizedPosition = 0;
        float m_PixelWidth = 6;

        ofColor m_Color;
};


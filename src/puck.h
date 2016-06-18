#pragma once
#include "ofMain.h"

class Puck
{
    public:
        Puck();
        void setup( ofColor col);
        void ReturnPuck();
        void update( float frameTime);
        void ResetToStart();
        void ResetToEnd();
        void draw(int width, int height, int yRow);

        float m_Direction;
        float m_StartSpeed;    // Speed that it starts at at the beginning of each round
        float m_CurrentSpeed;
        float m_Acceleration;

        bool m_IsDead;

        float m_NormalizedPosition;
        float m_NormalizedWidth;

        ofColor m_Color;


};


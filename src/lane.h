#pragma once
#include "button.h"
#include "puck.h"

class Lane
{
    public:
        Lane();
        void init(  ofColor returnzone1col, ofColor returnzone2col, int strip1Y, int strip2Y , bool flipColReturnZone);
        void draw(int width, int height);
        void update(float frameTime);

        void Reset();

        void P1ButtonPress();
        void P2ButtonPress();

        bool m_P1Win;
        bool m_P2Win;

        int m_Strip1YIndex;
        int m_Strip2YIndex;

        int m_PixelLength;

        bool m_FlipRreturnZoneColouring = false;

        int m_ReturnZonePixelLength;
        float m_ReturnZoneNormalized;

        ofColor m_ReturnZonePlayer1BaseColor;
        ofColor m_ReturnZonePlayer1CurrentColor;

        ofColor m_ReturnZonePlayer2BaseColor;
        ofColor m_ReturnZonePlayer2CurrentColor;

        Button m_P1Button;
        Button m_P2Button;

        Puck m_Puck;

};


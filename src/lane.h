#pragma once
#include "button.h"
#include "puck.h"

class Lane
{
    public:
        Lane();

        int m_PixelLength;

        int m_ReturnZonePixelLength;
        float m_ReturnZoneNormalized;

        ofColor m_ReturnZonePlayer1BaseColor;
        ofColor m_ReturnZonePlayer1CurrentColor;

        ofColor m_ReturnZonePlayer2BaseColor;
        ofColor m_ReturnZonePlayer2CurrentColor;

        Button m_P1Button;
        Button m_P2Button;

        //Public Color[] m_Pixels; // All the pixels in the lane. Does not include rings
        Puck m_Puck;

        void init(  ofColor returnzone1col, ofColor returnzone2col );
};


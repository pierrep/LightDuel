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

        bool _NearScoredThisFrame;
        bool _FarScoredThisFrame;

		bool _NearReturnThisFrame = false;
		bool _FarReturnThisFrame = false;

        int m_Strip1YIndex;
        int m_Strip2YIndex;

        int m_PixelLength;

        bool _LaneFlipped = false;

        int m_ReturnZonePixelLength;
        float m_ReturnZoneNormalized;

        ofColor _NearReturnZoneBaseCol;
        ofColor _NearReturnZoneCurrentCol;

        ofColor _FarReturnZoneBaseCol;
        ofColor _FarReturnZoneCurrentCol;

        Button _NearButton;
        Button _FarButton;

        Puck m_Puck;

};


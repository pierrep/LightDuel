#pragma once
#include "button.h"
#include "puck.h"

class Lane
{
    public:
        Lane();
		
		void Init(Button nearBtn, Button farBtn);
       // void init(  ofColor returnzone1col, ofColor returnzone2col, int strip1Y, int strip2Y , bool flipColReturnZone, Button nearBtn, Button farBtn);
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

        bool m_LaneFlipped = false;

        int m_ReturnZonePixelLength;
        float m_ReturnZoneNormalized;

		int _Index;

		Button _NearButton;
		Button _FarButton;

        ofColor _NearReturnZoneBaseCol;
        ofColor _NearReturnZoneCurrentCol;

        ofColor _FarReturnZoneBaseCol;
        ofColor _FarReturnZoneCurrentCol;
		
        Puck m_Puck;

};


#pragma once
#include "button.h"
#include "puck.h"

class Game;

class Lane
{
    public:
        Lane();

        Game* _Game;
		
        void Init(Button* nearBtn, Button* farBtn, ofColor _NearPlayerCol, ofColor _FarPlayerCol, Game* _Game);
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

        bool _LaneDrawFlipped = false;

        int m_ReturnZonePixelLength;
        float m_ReturnZoneNormalized;

		int _Index;

        Button* _NearButton;
        Button* _FarButton;

        ofColor _NearPlayerCol;
        ofColor _FarPlayerCol;
		
        Puck m_Puck;

};


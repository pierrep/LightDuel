#include "lane.h"

Lane::Lane()
{
    m_PixelLength = 300;
    m_ReturnZoneNormalized = 0.1f;
}

void Lane::init(  ofColor returnzone1col, ofColor returnzone2col )
{
    m_ReturnZonePlayer1BaseColor = returnzone1col;
    m_ReturnZonePlayer2BaseColor = returnzone2col;

    m_ReturnZonePixelLength = m_PixelLength * m_ReturnZoneNormalized;
}

#include "hudElement.hpp"
#include "../util/macros.hpp"

/*Recalculate absolute data*/
// Recalculate the absolute size and positon given a new screen resolution
// Also saves the resolution for future changes
void HUDElement::recalculateAbsoluteData(Vector2f screenResolution)
{
    // Save resolution
    this->m_screenResolution = screenResolution;

    // Calculate absolute position and size from relative data and screen resolution
    auto prevAbsolutePos { m_final_topleftPosition };
    this->m_final_topleftPosition = {
        this->m_user_position.x * screenResolution.x
        , this->m_user_position.y * screenResolution.y
    };
    auto prevAbsoluteSize { m_final_absoluteSize };
    this->m_final_absoluteSize = {
        this->m_user_size.x * screenResolution.x * m_user_scale
        , this->m_user_size.y * screenResolution.y * m_user_scale
    };

    // Avoid this calculation if size is the same
    if(prevAbsoluteSize == m_final_absoluteSize && prevAbsolutePos == m_final_topleftPosition) return;

    auto fullXSize { m_final_absoluteSize.x };
    auto fullYSize { m_final_absoluteSize.y };
    auto halfXSize { m_final_absoluteSize.x / 2.f };
    auto halfYSize { m_final_absoluteSize.y / 2.f };
    switch(this->m_user_origin)
    {
    case ORIGIN_CENTER:
        this->m_final_topleftPosition.x -= halfXSize;
        this->m_final_topleftPosition.y -= halfYSize;
        break;
    case ORIGIN_TOPLEFT: break;
    case ORIGIN_BOTLEFT:
        this->m_final_topleftPosition.y -= fullYSize;
        break;
    case ORIGIN_TOPRIGHT:
        this->m_final_topleftPosition.x -= fullXSize;
        break;
    case ORIGIN_BOTRIGHT:
        this->m_final_topleftPosition.x -= fullXSize;
        this->m_final_topleftPosition.y -= fullYSize;
        break;
    case ORIGIN_TOPCENTERED:
        this->m_final_topleftPosition.x -= halfXSize;
        break;
    case ORIGIN_BOTCENTERED:
        this->m_final_topleftPosition.x -= halfXSize;
        this->m_final_topleftPosition.y -= fullYSize;
        break;
    case ORIGIN_LEFTCENTERED:
        this->m_final_topleftPosition.y -= halfYSize;
        break;
    case ORIGIN_RIGHTCENTERED:
        this->m_final_topleftPosition.x -= fullXSize;
        this->m_final_topleftPosition.y -= halfYSize;
        break;
    }
}

/*Recalculate absolute data (Text)*/
// Calculates special size and font size for text, then does normal calculations
void HUDElementText::recalculateAbsoluteData(Vector2f screenResolution)
{
    // Base is 1366 x 768
    m_fontSize = m_user_fontSize * (screenResolution.x / 1366);

    // TODO:: Change m_user_size
    HUDElement::recalculateAbsoluteData(screenResolution);
}

/*Recalculate absolute data (Slider)*/
// Calculates special size and font size for text, then does normal calculations
void HUDElementSlider::recalculateAbsoluteData(Vector2f screenResolution)
{
    // Base is 1366 x 768
    m_fontSize = m_user_fontSize * (screenResolution.x / 1366);

    // TODO:: Change m_user_size
    HUDElement::recalculateAbsoluteData(screenResolution);
}

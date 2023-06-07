#include "hudElementDrawing.hpp"
#include "../util/macros.hpp"

void drawText(HUDElement* baseElement, GameContext& gctx) noexcept
{
    auto* elem = static_cast<HUDElementText*>(baseElement);
    gctx.graphicsEngine.draw2DString(
        elem->getText()
        , baseElement->getAbsolutePosition()
        , elem->getColor()
        , (int)elem->getFontSize()
        , elem->getFont()
    );
}

void drawImage(HUDElement* baseElement, GameContext& gctx) noexcept
{
    auto* elem = static_cast<HUDElementImage*>(baseElement);
    gctx.graphicsEngine.draw2DImage(
        elem->getFilepath()
        , baseElement->getAbsolutePosition()
        , baseElement->getAbsoluteSize()
        , elem->getColor()
        , 1.0f
    );
}

void drawFilledRectangle(HUDElement* baseElement, GameContext& gctx) noexcept
{
    auto* elem = static_cast<HUDElementRectangle*>(baseElement);
    gctx.graphicsEngine.draw2Drectangle(
        baseElement->getAbsolutePosition()
        , baseElement->getAbsoluteSize()
        , elem->getColor()
        , elem->getOpacity()
    );
}

void drawButton(HUDElement* baseElement, GameContext& gctx) noexcept
{
    auto* elem = static_cast<HUDElementButton*>(baseElement);
    gctx.graphicsEngine.draw2DButton(
        elem->getDataContainer()
        , baseElement->getName()
        , baseElement->getAbsolutePosition()
        , baseElement->getAbsoluteSize()
        , elem->getText()
        , elem->getBackgroundFilepath()
        , elem->getButtonColor()
    );
}

void drawCheckbox(HUDElement* baseElement, GameContext& gctx) noexcept
{
    auto* elem = static_cast<HUDElementCheckbox*>(baseElement);
    gctx.graphicsEngine.draw2DCheckbox(
        elem->getDataContainer()
        , baseElement->getName()
        , baseElement->getAbsolutePosition()
        , baseElement->getAbsoluteSize()
    );
}

void drawSliderInt(HUDElement* baseElement, GameContext& gctx) noexcept
{
    auto* elem = static_cast<HUDElementSlider*>(baseElement);
    int prevVal { elem->getDataContainerInt() };
    gctx.graphicsEngine.draw2DSliderInt(
        elem->getDataContainerInt()
        , baseElement->getName()
        , (int)elem->getMinValue()
        , (int)elem->getMaxValue()
        , baseElement->getAbsolutePosition()
        , baseElement->getAbsoluteSize()
        , elem->getLabel()
        , elem->getInnerLabel()
        , elem->getFontSize()
        , elem->getFont()
    );
    if(prevVal != elem->getDataContainerInt())
        elem->setFormatIndex(elem->getDataContainerInt());
}

void drawSliderFloat(HUDElement* baseElement, GameContext& gctx) noexcept
{  
    auto* elem = static_cast<HUDElementSlider*>(baseElement);
    gctx.graphicsEngine.draw2DSliderFloat(
        elem->getDataContainerFloat()
        , baseElement->getName()
        , elem->getMinValue()
        , elem->getMaxValue()
        , baseElement->getAbsolutePosition()
        , baseElement->getAbsoluteSize()
        , elem->getLabel()
        , elem->getInnerLabel()
        , elem->getFontSize()
        , elem->getFont()
    );
}

void drawListBase(HUDElement* baseElement, GameContext& gctx) noexcept
{}

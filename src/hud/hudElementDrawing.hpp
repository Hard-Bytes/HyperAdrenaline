#pragma once
#include "../context/gameContext.hpp"
#include "hudElement.hpp"

// Element drawing function
void drawText(HUDElement*, GameContext& gctx) noexcept;
void drawImage(HUDElement*, GameContext& gctx) noexcept;
void drawFilledRectangle(HUDElement*, GameContext& gctx) noexcept;
void drawButton(HUDElement*, GameContext& gctx) noexcept;
void drawCheckbox(HUDElement*, GameContext& gctx) noexcept;
void drawSliderInt(HUDElement*, GameContext& gctx) noexcept;
void drawSliderFloat(HUDElement*, GameContext& gctx) noexcept;
void drawListBase(HUDElement*, GameContext& gctx) noexcept;

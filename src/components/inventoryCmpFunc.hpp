#pragma once

#include "inventoryCmp.hpp"
#include "../context/gameContext.hpp"

// cf = Component Functions
namespace cf
{
    bool hasKey(InventoryComponent& cmp, int keyQuery) noexcept;
    void addKey(InventoryComponent& cmp, int newKey, GameContext& gctx, bool throwPopup=true) noexcept;
    void addCredits(InventoryComponent& cmp, int addition, GameContext& gctx, bool sendOnline = true) noexcept;
}

#pragma once
#include "component.hpp"

struct InventoryComponent : public Component
{
    // Constructor
    explicit InventoryComponent(EntityID entID);
    ~InventoryComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeInventory; }

    int points {0};
    std::vector<int> keys{}; 
};

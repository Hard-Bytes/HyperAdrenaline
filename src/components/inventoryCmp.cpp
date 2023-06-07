#include "inventoryCmp.hpp"

InventoryComponent::InventoryComponent(EntityID entID)
    : Component(entID)
{

}

InventoryComponent::~InventoryComponent()
{
    keys.clear();
}

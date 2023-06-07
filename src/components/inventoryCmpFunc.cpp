#include "inventoryCmpFunc.hpp"

namespace cf
{

/*Inventory: Has Key*/
// Returns true if the key id is in this inventory component
bool hasKey(InventoryComponent& inventory, int keyQuery) noexcept
{
    auto& keys = inventory.keys;
    for(auto key : keys)
        if(keyQuery == key)
            return true;
    return false;
}

/*Inventory: Add Key*/
// Adds the specified key to this inventory
void addKey(InventoryComponent& inventory, int newKey, GameContext& gctx, bool throwPopup) noexcept
{
    auto& keys = inventory.keys;
    auto it = std::find(keys.begin(), keys.end(), newKey);

    if(it == keys.end())
    {
        keys.push_back(newKey);
        gctx.hudManager.forceInteractableInfoRecalculation();
    }

    if(throwPopup && gctx.gameManager.getLocalPlayerID() == (int)inventory.getEntityID())
    {
        int keyColor = ((newKey-1) % 6) + 1;
        gctx.hudManager.addPopup(
            "bottomSlot"
            , "keyAdded"
            , "You got the key number " + std::to_string(newKey)+"!"
            , "popupBackground"
            , "icon_card"+std::to_string((keyColor))
            , 2.f
        );
    }
}

/*Inventory: Add Credits*/
// Adds the specified amount of credits to this inventory
// Use negative values to substract credits
void addCredits(InventoryComponent& inventory, int addition, GameContext& gctx, bool sendOnline) noexcept
{
    // Update points
    inventory.points += addition;
    gctx.hudManager.forceInteractableInfoRecalculation();

    // Update HUD (different if player is local player or online player)
    int playerID { (int)inventory.getEntityID() };
    if(gctx.gameManager.getLocalPlayerID() == playerID)
    {
        gctx.hudManager.setCreditCount(inventory.points);
    }
    else
    {
        auto& players { gctx.gameManager.getPlayerIDs() };
        unsigned int i;
        for(i=1; i<players.size(); ++i)
            if(players[i] == playerID)
                break;
        if(i < players.size())
            gctx.hudManager.setOtherPlayerCredits(i-1, inventory.points);
    }

    // Send online (if needed)
    if(!sendOnline) return;
    auto typeCmp = gctx.entityManager.getComponentByID<TypeComponent>(inventory.getEntityID());
    gctx.networkClient.send(
        "PlayerPointsChanged" SEP
        +std::to_string(typeCmp->idInServer)
        +SEP+std::to_string(addition)
    );
}

} // End of namespace cf

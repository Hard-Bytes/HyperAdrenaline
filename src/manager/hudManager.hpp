#pragma once
#include "../components/entity.hpp"
#include "../hud/hudPopup.hpp"

class HUDSystem;
class GameContext;

struct HUDManager
{
    void resetData();
    
    // HUD actions
    void updateDashCooldownIndicator(float currentCooldown, float maxCooldown);
    void updateCrosshair(float delta);
    void updateWeaponBar(float delta);
    void showWeaponBar(WeaponComponent& wpnCmp, float timeSecs = 3.f);
    void setCrosshairEffect(const std::string& newEffect, const std::string& newColor);
    void setLifebarLengthAndCount(float currentHP, float maxHP);
    void setCreditCount(int currentCredits);
    void setAmmoCount(int currentAmmo);
    void setFPSCount(int fps, bool showFPS);
    void setWeaponUnlockedInBar(int weaponID, bool visible);
    void setEquipedWeaponMarkInBar(int weaponID);
    void setInteractableInfo(EntityID entityID);

    void makeOtherPlayerData(int playerIndex);
    void setOtherPlayerInfoStyle(int playerIndex, PlayerLook style);
    void setOtherPlayerInfoVisible(int playerIndex, bool isVisible);
    void setOtherPlayerLifebar(int playerIndex, float currentHP, float maxHP);
    void setOtherPlayerCredits(int playerIndex, int currentCredits);

    // Popup actions
    void initPopupSlot(const std::string& slotName, const std::string& textHudElement, const std::string& bgHudElement, const std::string& iconHudElement);
    void addPopup(
        const std::string& slot
        , const std::string& category
        , const std::string& msg
        , const std::string& bg
        , const std::string& icon
        , float time
    );
    void setPopupData(HUDPopup* data, const std::string& textHudElement, const std::string& bgHudElement, const std::string& iconHudElement);
    void updatePopupTimes();
    void clearPopupSlot(const std::string& slotName);

    // Modification
    void makeList(const std::string& listElementName, std::vector<std::string>& elements);

    template <class... Args>
    void addPriorityPopup(const std::string& slot, Args&&... args)
        { this->clearPopupSlot(slot); addPopup(slot, std::forward<Args>(args)...); }

    constexpr void forceInteractableInfoRecalculation() noexcept
        { lastInteractableFound = -1; }

    // Basic getters and setters
    constexpr void setActiveHUD(HUDSystem* newhud) noexcept
        { activeHUD = newhud; }
    constexpr bool hasActiveHUD() const noexcept
        { return activeHUD != nullptr; }

    constexpr void setGameContext(GameContext* newGctx) noexcept
        { gctx = newGctx; }
private:
    HUDSystem* activeHUD { nullptr };
    GameContext* gctx { nullptr };

    int lastInteractableFound { -1 };
    float m_crosshairCounter  { 0.f };
    bool m_modifiedCrosshair  { false };
    float m_weaponBarCounter  { 0.f };

    // Helpers
    bool playerHasKey(int keyID);
    int getPlayerCredits();
};

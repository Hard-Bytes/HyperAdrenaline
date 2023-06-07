#include "hudManager.hpp"

#include "../components/inventoryCmpFunc.hpp"
// Forwarded in hpp
#include "../system/hud.hpp"
#include "../context/gameContext.hpp"

void HUDManager::resetData()
{
    lastInteractableFound   = -1;
    m_crosshairCounter      = 0.f;
    m_modifiedCrosshair     = false;
    m_weaponBarCounter      = 0.f;
}

void HUDManager::updateDashCooldownIndicator(float currentCooldown, float maxCooldown)
{
    float maxDashSize { 0.11102f };
    auto perc { 1 - (currentCooldown / maxCooldown) };  // Get percentage [0-1]
    perc = std::min(perc, 1.f);                         // Cap to 1
    auto newSize { perc * maxDashSize };                // Scale to [0 - max]
    std::string colorIdentifier { "dashChargeColor" };  // Choose color
    if(perc == 1.f) colorIdentifier = "dashFullColor";
    Vector3f dashpercentageColor { activeHUD->getColorFromPalette(colorIdentifier) };

    auto recttmp = static_cast<HUDElementRectangle*>(activeHUD->getHUDElement("rect_DashBackground"));
    auto dashSize = recttmp->getRelativeSize();
    dashSize.y = newSize;
    recttmp->setRelativeSize(dashSize);
    recttmp->setColor(dashpercentageColor);
}

void HUDManager::updateCrosshair(float delta)
{
    if(m_modifiedCrosshair)
    {
        m_crosshairCounter += delta;
        if(m_crosshairCounter > 0.5f)
        {
            auto* tmp = static_cast<HUDElementImage*>(activeHUD->getHUDElement("img_Crosshair"));
            if(!tmp) return;
            tmp->setColor(Vector3f{1,1,1});
            tmp->setFilepath(activeHUD->getTextureFromList("crosshairNormal"));
            m_crosshairCounter = 0.f;
            m_modifiedCrosshair = false;
        }
    }
}

void HUDManager::updateWeaponBar(float delta)
{
    if(m_weaponBarCounter > 0)
    {
        m_weaponBarCounter -= delta;
        if(m_weaponBarCounter <= 0.0f)
        {
            auto bar = activeHUD->getHUDElement("img_weaponBar");
            bar->setIsVisible(false);

            this->setEquipedWeaponMarkInBar(0);
            for(int i=1; i<=7; i++)
                this->setWeaponUnlockedInBar(i, false);
        }
    }
}

void HUDManager::showWeaponBar(WeaponComponent& wpnCmp, float timeSecs)
{
    m_weaponBarCounter = timeSecs;
    auto& weaponVec = wpnCmp.weapons;

    auto bar = activeHUD->getHUDElement("img_weaponBar");
    bar->setIsVisible(true);
    this->setEquipedWeaponMarkInBar(weaponVec[wpnCmp.index].id);

    for(int i=1; i<=7; i++)
        this->setWeaponUnlockedInBar(i, false);
    for(auto& wpn : weaponVec)
        this->setWeaponUnlockedInBar(wpn.id, true);
}

void HUDManager::setCrosshairEffect(const std::string& newEffect, const std::string& newColor)
{
    auto* element { static_cast<HUDElementImage*>(activeHUD->getHUDElement("img_Crosshair")) };
    element->setColor(activeHUD->getColorFromPalette(newColor));
    element->setFilepath(activeHUD->getTextureFromList(newEffect));

    m_crosshairCounter = 0.f;
    m_modifiedCrosshair = true;
}

void HUDManager::setLifebarLengthAndCount(float currentHP, float maxHP)
{
    currentHP = std::ceil(currentHP);
    auto texttmp = static_cast<HUDElementText*>(activeHUD->getHUDElement("text_LifebarCount"));
    texttmp->setText(std::to_string((int)currentHP));

    float lifePerc { currentHP / maxHP };
    float maxLifeSize { 0.177f };
    auto recttmp { static_cast<HUDElementRectangle*>(activeHUD->getHUDElement("rect_Lifebar")) };
    auto lifeSize { recttmp->getRelativeSize() };
    lifeSize.x = lifePerc * maxLifeSize;
    recttmp->setRelativeSize(lifeSize);
}

void HUDManager::setCreditCount(int currentCredits)
{
    auto texttmp = static_cast<HUDElementText*>(activeHUD->getHUDElement("text_CreditsCount"));
    texttmp->setText(std::to_string(currentCredits));
}

void HUDManager::setAmmoCount(int currentAmmo)
{
    // Ammo -1 if ammo is infinite, so display INF
    // Ammo -2 means "does not use ammo", so display nothing
    std::string ammoString {""};
    if( currentAmmo >= 0) 
        ammoString = std::to_string(currentAmmo);
    else if(currentAmmo == -1)
        ammoString = "INF";

    auto* texttmp = static_cast<HUDElementText*>(activeHUD->getHUDElement("text_AmmoCount"));
    texttmp->setText(ammoString);
}

void HUDManager::setFPSCount(int fps, bool showFPS)
{
    auto texttmp = static_cast<HUDElementText*>(activeHUD->getHUDElement("text_FPSCount"));
    if(showFPS)
        texttmp->setText("FPS: "+std::to_string(fps));
    else
        texttmp->setText("");
}

void HUDManager::setWeaponUnlockedInBar(int weaponID, bool visible)
{
    if(weaponID <= 0 || weaponID > 7) return;
    auto element = activeHUD->getHUDElement("img_weaponIcon"+std::to_string(weaponID));
    element->setIsVisible(visible);
}

void HUDManager::setEquipedWeaponMarkInBar(int weaponID)
{
    auto mark = activeHUD->getHUDElement("img_weaponBarMarker");
    if(weaponID > 0 && weaponID <= 7)
    {
        auto base = activeHUD->getHUDElement("img_weaponIcon"+std::to_string(weaponID));
        mark->setRelativePosition(base->getRelativePosition());
        mark->setIsVisible(true);
    }
    else
    {
        mark->setIsVisible(false);
    }
}

void HUDManager::setInteractableInfo(EntityID entityID)
{
    // Only update if interactable target changed
    // If it's a new interactable, save its id
    if((int)entityID == lastInteractableFound)
        return;
    lastInteractableFound = (int)entityID;

    std::string result { "" };
    std::string summary { "" };
    Vector3f summaryColor;
    if(entityID > 0)
    {
        // Recover interactable data
        auto* typeCmp = gctx->entityManager.getComponentByID<TypeComponent>(entityID);
        auto* interactableCmp = gctx->entityManager.getComponentByID<InteractableComponent>(entityID);
        auto type = typeCmp->type;
        auto data = interactableCmp->interactionData;
        auto dataStr = std::to_string((int)data);
        std::string interactionKey = "E";

        switch (type)
        {
        case ENT_UNLOCKEDDOOR:
            result = "Press ["+interactionKey+"] to open";
            summary = "[Unlocked]";
            summaryColor = activeHUD->getColorFromPalette("interactSuccess");
            break;
        case ENT_KILLDOOR:
            if(data > 0 && gctx->gameManager.killedMapEnemies < data)
            {
                result = 
                    "Kill " + dataStr + " enemies to open ("
                    +std::to_string((int)(data - gctx->gameManager.killedMapEnemies))
                    +" more)"
                ;
                summary = "[Locked]";
                summaryColor = activeHUD->getColorFromPalette("interactError");
            }
            else
            {
                result = "Press ["+interactionKey+"] to open";
                summary = "[Unlocked]";
                summaryColor = activeHUD->getColorFromPalette("interactSuccess");
            }
            break;
        case ENT_KEYDOOR:
            if(playerHasKey(interactableCmp->sharedKeyId))
            {
                result = "Press ["+interactionKey+"] to open with key number "+std::to_string(interactableCmp->sharedKeyId);
                summary = "[Unlocked]";
                summaryColor = activeHUD->getColorFromPalette("interactSuccess");
            }
            else
            {
                result = "Find the key number "+std::to_string(interactableCmp->sharedKeyId)+" to open";
                summary = "[Locked]";
                summaryColor = activeHUD->getColorFromPalette("interactError");
            }
            break;
        case ENT_HEAL_VENDINGMACHINE:
        case ENT_AMMO_VENDINGMACHINE:
        {
            auto* aiCmp = gctx->entityManager.getComponentByID<AIComponent>(interactableCmp->idRemoteToInteractWith);
            bool hasKitAlready { !(aiCmp->hasKit == 2) };
            result = "Spend " + dataStr + " credits to buy";
            if(hasKitAlready)
            {
                summary = "[Cannot Purchase]";
                result = "Grab kit first";
                summaryColor = activeHUD->getColorFromPalette("interactMiddle");
            }
            else if(getPlayerCredits() < data)
            {
                summary = "[Cannot Purchase]";
                summaryColor = activeHUD->getColorFromPalette("interactError");
            }
            else
            {
                summary = "[Can Purchase]";
                summaryColor = activeHUD->getColorFromPalette("interactSuccess");
            }
            break;
        }
        case ENT_HEALKIT:
        case ENT_AMMOKIT:
        case ENT_PICKUPKEY:
        case ENT_PICKUPWEAPON:
            result = "Press ["+interactionKey+"] to get";
            break;
        case ENT_PLAYER:
            result = "Press ["+interactionKey+"] to revive";
            summary = "[Revive!]";
            summaryColor = activeHUD->getColorFromPalette("interactSuccess");
            break;
        default: break;
        }
    }

    auto* infoText { static_cast<HUDElementText*>(activeHUD->getHUDElement("text_InteractionInfo")) };
    auto* summaryText { static_cast<HUDElementText*>(activeHUD->getHUDElement("text_InteractionStatus")) };
    infoText->setText(result);
    summaryText->setText(summary);
    summaryText->setColor(summaryColor);
}

void HUDManager::makeOtherPlayerData(int playerIndex)
{
    Vector2f resolution { gctx->graphicsEngine.getScreenResolution() };
    std::string ind { std::to_string(playerIndex) };
    Vector2f offset { 0, -playerIndex*0.12f };

    activeHUD->addNewImageElement(
        "img_CharCard_online"+ind
        , resolution
        , Vector2f{0.014, 0.8165} + offset
        , ORIGIN_BOTLEFT // 2
        , Vector2f{0.2, 0.1284}
        , "hudCharacterCardBlue"
        , Vector3f{1,1,1}
    );
    activeHUD->addNewRectangleElement(
        "rect_LifebarBack_online"+ind
        , resolution
        , Vector2f{0.03, 0.7872} + offset
        , ORIGIN_BOTLEFT // 2
        , Vector2f{0.14, 0.03255}
        , Vector3f{1,1,1}
        , 1.f
    );
    activeHUD->addNewRectangleElement(
        "rect_Lifebar_online"+ind
        , resolution
        , Vector2f{0.03, 0.7872} + offset
        , ORIGIN_BOTLEFT // 2
        , Vector2f{0.13986, 0.03255}
        , Vector3f{1,1,1}
        , 1.f
    );
    activeHUD->addNewImageElement(
        "img_LifebarBorder_online"+ind
        , resolution
        , Vector2f{0.022, 0.8} + offset
        , ORIGIN_BOTLEFT // 2
        , Vector2f{0.14916, 0.05368}
        , "hudLifeBarBorderBlue"
        , Vector3f{1,1,1}
    );
    activeHUD->addNewTextElement(
        "text_LifebarCount_online"+ind
        , resolution
        , Vector2f{0.17328, 0.749} + offset
        , ORIGIN_TOPLEFT // 1
        , 32
        , ""
        , "audiowide"
        , Vector3f{1,1,1}
    );
    activeHUD->addNewImageElement(
        "img_CharProfile_online"+ind
        , resolution
        , Vector2f{0.04, 0.74845} + offset
        , ORIGIN_BOTLEFT // 2
        , Vector2f{0.024076, 0.0435}
        , activeHUD->getTextureFromList("creditsIcon")
        , Vector3f{1,1,1}
    );
    activeHUD->addNewImageElement(
        "img_CreditsIcon_online"+ind
        , resolution
        , Vector2f{0.07593, 0.74845} + offset
        , ORIGIN_BOTLEFT // 2
        , Vector2f{0.024076, 0.0428}
        , activeHUD->getTextureFromList("creditsIcon")
        , Vector3f{1,1,1}
    );
    activeHUD->addNewTextElement(
        "text_CreditsCount_online"+ind
        , resolution
        , Vector2f{0.107, 0.706} + offset
        , ORIGIN_TOPLEFT // 1
        , 32
        , ""
        , "audiowide"
        , activeHUD->getColorFromPalette("creditsYellow")
    );
}

void HUDManager::setOtherPlayerInfoStyle(int playerIndex, PlayerLook style)
{
    std::string hpEmptyBarColorName { "hpBarEmptyColorBlue" };
    std::string hpFullBarColorName  { "hpBarFullColorBlue" };
    std::string hpCharCardTexName   { "hudCharacterCardBlue" };
    std::string hpLifebarTexName    { "hudLifeBarBorderBlue" };
    std::string profileTexName      { "hudCharacterProfileBlue" };
    if(style == PL_Captain)
    {
        hpEmptyBarColorName = "hpBarEmptyColorGreen";
        hpFullBarColorName  = "hpBarFullColorGreen";
        hpCharCardTexName   = "hudCharacterCardGreen";
        hpLifebarTexName    = "hudLifeBarBorderGreen";
        profileTexName      = "hudCharacterProfileGreen";
    }
    Vector3f hpEmptyBarColor    { this->activeHUD->getColorFromPalette(hpEmptyBarColorName) };
    Vector3f hpFullBarColor     { this->activeHUD->getColorFromPalette(hpFullBarColorName) };
    std::string hpCharCardTex   { this->activeHUD->getTextureFromList(hpCharCardTexName) };
    std::string hpLifebarTex    { this->activeHUD->getTextureFromList(hpLifebarTexName) };
    std::string profileTex      { this->activeHUD->getTextureFromList(profileTexName) };

    std::string ind { std::to_string(playerIndex) };
    activeHUD->getHUDElementImage("img_CharCard_online"+ind)->setFilepath(hpCharCardTex);
    activeHUD->getHUDElementRectangle("rect_LifebarBack_online"+ind)->setColor(hpEmptyBarColor);
    activeHUD->getHUDElementRectangle("rect_Lifebar_online"+ind)->setColor(hpFullBarColor);
    activeHUD->getHUDElementImage("img_LifebarBorder_online"+ind)->setFilepath(hpLifebarTex);
    activeHUD->getHUDElementText("text_LifebarCount_online"+ind)->setColor(hpFullBarColor);
    activeHUD->getHUDElementImage("img_CharProfile_online"+ind)->setFilepath(profileTex);
}

void HUDManager::setOtherPlayerInfoVisible(int playerIndex, bool isVisible)
{
    if(playerIndex > 2) return; // TODO::
    std::string playerIndexStr { std::to_string(playerIndex) };
    activeHUD->getHUDElement("img_CharCard_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("rect_LifebarBack_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("rect_Lifebar_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("img_LifebarBorder_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("text_LifebarCount_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("img_CreditsIcon_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("text_CreditsCount_online"+playerIndexStr)->setIsVisible(isVisible);
    activeHUD->getHUDElement("img_CharProfile_online"+playerIndexStr)->setIsVisible(isVisible);
}

void HUDManager::setOtherPlayerLifebar(int playerIndex, float currentHP, float maxHP)
{
    if(playerIndex > 2) return; // TODO::
    // TODO:: To helper func
    currentHP = std::ceil(currentHP);
    auto texttmp { activeHUD->getHUDElementText("text_LifebarCount_online"+std::to_string(playerIndex)) };
    texttmp->setText(std::to_string((int)currentHP));

    float lifePerc { currentHP / maxHP };
    float maxLifeSize { 0.13986f }; //{ 0.177f };
    auto recttmp { activeHUD->getHUDElementRectangle("rect_Lifebar_online"+std::to_string(playerIndex)) };
    auto lifeSize { recttmp->getRelativeSize() };
    lifeSize.x = lifePerc * maxLifeSize;
    recttmp->setRelativeSize(lifeSize);
}

void HUDManager::setOtherPlayerCredits(int playerIndex, int currentCredits)
{
    if(playerIndex > 2) return; // TODO::
    auto texttmp { activeHUD->getHUDElementText("text_CreditsCount_online"+std::to_string(playerIndex)) };
    texttmp->setText(std::to_string(currentCredits));
}

void HUDManager::initPopupSlot(const std::string& slotName, const std::string& textHudElement, const std::string& bgHudElement, const std::string& iconHudElement)
{
    activeHUD->initPopupSlot(slotName, textHudElement, bgHudElement, iconHudElement);
}

void HUDManager::addPopup(
    const std::string& slot
    , const std::string& category
    , const std::string& msg
    , const std::string& bg
    , const std::string& icon
    , float time)
{
    auto bgTex = activeHUD->getTextureFromList(bg);
    auto iconTex = activeHUD->getTextureFromList(icon);
    activeHUD->addPopup(slot, category, msg, bgTex, iconTex, time);
}

void HUDManager::setPopupData(HUDPopup* data, const std::string& textHudElement, const std::string& bgHudElement, const std::string& iconHudElement)
{
    auto* textElem { static_cast<HUDElementText*>(activeHUD->getHUDElement(textHudElement)) };
    auto* bgElem { static_cast<HUDElementImage*>(activeHUD->getHUDElement(bgHudElement)) };
    auto* iconElem { static_cast<HUDElementImage*>(activeHUD->getHUDElement(iconHudElement)) };

    if(!data)
    {
        textElem->setIsVisible(false);
        bgElem->setIsVisible(false);
        iconElem->setIsVisible(false);
        return;
    }
    textElem->setText(data->message);
    textElem->setIsVisible(true);

    bgElem->setFilepath(data->bgTextureName);
    bgElem->setIsVisible(data->bgTextureName != ""); // True if path is not empty

    iconElem->setFilepath(data->iconTextureName);
    iconElem->setIsVisible(data->iconTextureName != ""); // True if path is not empty
}

void HUDManager::updatePopupTimes()
{
    activeHUD->updatePopupTimes();
}

void HUDManager::clearPopupSlot(const std::string& slotName)
{
    activeHUD->clearPopupSlot(slotName);
}

void HUDManager::makeList(const std::string& listElementName, std::vector<std::string>& elements)
{
    auto* listBase = static_cast<HUDElementListBase*>(activeHUD->getHUDElement(listElementName));
    auto resolution { gctx->graphicsEngine.getScreenResolution() };
    auto position   { listBase->getRelativePosition() };
    auto spacing    { listBase->getSpacing() };
    auto origin     { listBase->getElementOrigin() };
    auto fontname   { listBase->getFont() };
    auto fontsize   { listBase->getFontSize() };
    auto textcolor  { listBase->getTextColor() };
    int i = 0;
    for(auto& listElem : elements)
    {
        activeHUD->addNewTextElement(
            listElementName+"_"+std::to_string(i)
            , resolution
            , position
            , origin
            , fontsize
            , listElem
            , fontname
            , textcolor
        );
        position = position + spacing;
        i++;
    }
}

bool HUDManager::playerHasKey(int keyID)
{
    auto localPlayer = gctx->gameManager.getLocalPlayerID();
    if(localPlayer <= 0) return 0;
    auto inventory = gctx->entityManager.getComponentByID<InventoryComponent>(localPlayer);
    return cf::hasKey(*inventory, keyID);
}

int HUDManager::getPlayerCredits()
{
    auto localPlayer = gctx->gameManager.getLocalPlayerID();
    if(localPlayer <= 0) return 0;
    auto inventory = gctx->entityManager.getComponentByID<InventoryComponent>(localPlayer);
    return inventory->points;
}

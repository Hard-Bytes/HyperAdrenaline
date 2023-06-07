#include "hud.hpp"
#include "../util/paths.hpp"
#include "../util/jsonReader.hpp"
#include "../hud/hudAliases.hpp"

HUDSystem::HUDSystem(GameContext& gc)
    : gctx(gc)
{}

HUDSystem::~HUDSystem()
{
    m_hudElements.clear();
    m_hudElementsText.clear();
    m_hudElementsImages.clear();
    m_hudElementsRectangles.clear();
    m_hudElementsButtons.clear();
    m_hudElementsListbases.clear();

    m_colorPalette.clear();
    m_textures.clear();

    for(auto& popupSlot : m_popupSlots)
    {
        popupSlot.second.clearQueue();
    }
    m_popupSlots.clear();
}

void HUDSystem::updateAll() noexcept
{
    this->updatePopupTimes();

    auto playerID = gctx.gameManager.getLocalPlayerID();
    const auto* typeCmp = gctx.entityManager.getComponentByID<TypeComponent>(playerID);
    if(!typeCmp || typeCmp->type != ENT_PLAYER )
        return;

    const WeaponComponent* weaponCmp = gctx.entityManager.getComponentByID<WeaponComponent>(playerID);
    const CollisionComponent* collisonCmp = gctx.entityManager.getComponentByID<CollisionComponent>(playerID);
    auto& hudManager = gctx.hudManager;

    // Set values
    hudManager.setAmmoCount(weaponCmp->weapons[weaponCmp->index].ammo);
    hudManager.updateDashCooldownIndicator(collisonCmp->dashCooldown, collisonCmp->dashCooldownTime);
    hudManager.setFPSCount(gctx.graphicsEngine.getFPSCount(), gctx.gameManager.showFPS);
}

// Se llama desde fuera, concretamente desde RenderSystem
void HUDSystem::renderAll(bool updateGameData) const noexcept
{
    for(auto* elem : m_hudElements)
    {
        if(!elem->getIsVisible()) continue;
        // Function pointer with elem as param
        auto type = elem->getElementType();
        m_drawingFunctionMap.at(type)(elem, gctx);
    }

    // Update some specific hud elements
    if(!updateGameData) return;
    auto delta = gctx.graphicsEngine.getTimeDiffInSeconds();
    gctx.hudManager.updateCrosshair(delta);
    gctx.hudManager.updateWeaponBar(delta);
}

void HUDSystem::loadFromFile(const std::string& filepath) noexcept
{
    rapidjson::Document doc;
    openJsonFile(filepath, doc);

    auto resolution = gctx.graphicsEngine.getScreenResolution();

    // Obtain color palette
    if(doc.HasMember("palette"))
    {
        const rapidjson::Value& colorDoc = doc["palette"];
        m_colorPalette.reserve(colorDoc.MemberCount());
        for (auto iter = colorDoc.MemberBegin(); iter != colorDoc.MemberEnd(); ++iter)
        {
            std::string key = iter->name.GetString();
            auto& value = iter->value;
            auto color = GET_VEC3(value);
            m_colorPalette[key.c_str()] = color;
        }
    }

    // Obtain textures
    if(doc.HasMember("textures"))
    {
        const rapidjson::Value& textureDoc = doc["textures"];
        m_textures.reserve(textureDoc.MemberCount());
        for (auto iter = textureDoc.MemberBegin(); iter != textureDoc.MemberEnd(); ++iter)
        {
            std::string key = iter->name.GetString();
            std::string value = iter->value.GetString();
            m_textures[key.c_str()] = value;
        }
    }

    auto& elements = doc["elements"];
    unsigned int numElements = elements.Size();
    for(unsigned int i = 0; i < numElements; i++)
    {
        auto& element = elements[i];
        auto type = element[HUDDATA_TYPE].GetInt();
        switch (type)
        {
        case HUD_TEXT:
            addNewTextElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , element[HUDDATA_TEXT_FONTSIZE].GetFloat()
                , element[HUDDATA_TEXT_TEXT].GetString()
                , element[HUDDATA_TEXT_FONT].GetString()
                , m_colorPalette[element[HUDDATA_TEXT_COLOR].GetString()]
            );
            break;
        case HUD_IMAGE:
            addNewImageElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
                , m_textures[element[HUDDATA_IMG_TEXTURE].GetString()]
                , Vector3f{1,1,1}
            );
            break;
        case HUD_FILLEDRECTANGLE:
            addNewRectangleElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
                , m_colorPalette[element[HUDDATA_RECT_COLOR].GetString()]
                , element[HUDDATA_RECT_OPACITY].GetFloat()
            );
            break;
        case HUD_BUTTON:
            addNewButtonElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
                , m_textures[element[HUDDATA_IMG_TEXTURE].GetString()]
                , m_colorPalette[element[HUDDATA_BUTTON_BGCOLOR].GetString()]
                , element[HUDDATA_TEXT_FONTSIZE].GetFloat()
                , element[HUDDATA_TEXT_TEXT].GetString()
                , element[HUDDATA_TEXT_FONT].GetString()
                , m_colorPalette[element[HUDDATA_BUTTON_TEXTCOLOR].GetString()]
            );
            break;
        case HUD_CHECKBOX:
            addNewCheckboxElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
            );
            break;
        case HUD_SLIDERINT:
            addNewSliderElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
                , true
                , (float)element[HUDDATA_SLIDER_MINVAL].GetInt()
                , (float)element[HUDDATA_SLIDER_MAXVAL].GetInt()
                , element[HUDDATA_SLIDER_LABEL].GetString()
                , element[HUDDATA_SLIDER_FORMAT].GetString()
                , element[HUDDATA_TEXT_FONT].GetString()
                , element[HUDDATA_TEXT_FONTSIZE].GetFloat()
            );
            if(element.HasMember(HUDDATA_SLIDER_FORMATVEC))
            {
                auto& slider { m_hudElementsSliders.back() };
                auto vec { element[HUDDATA_SLIDER_FORMATVEC].GetArray() };
                unsigned int nElems { vec.Size() };
                slider.reserveFormatVector(nElems);
                for(unsigned int i=0; i<nElems; i++)
                    slider.addFormat(vec[i].GetString());
            }
            break;
        case HUD_SLIDERFLOAT:
        {
            addNewSliderElement(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
                , false
                , element[HUDDATA_SLIDER_MINVAL].GetFloat()
                , element[HUDDATA_SLIDER_MAXVAL].GetFloat()
                , element[HUDDATA_SLIDER_LABEL].GetString()
                , element[HUDDATA_SLIDER_FORMAT].GetString()
                , element[HUDDATA_TEXT_FONT].GetString()
                , element[HUDDATA_TEXT_FONTSIZE].GetFloat()
            );
            break;
        }
        case HUD_LISTBASE:
            m_hudElementsListbases.emplace_back(
                element[HUDDATA_NAME].GetString()
                , resolution
                , GET_VEC2(element[HUDDATA_POSITION])
                , (HUDElementOrigin)element[HUDDATA_ORIGIN].GetInt()
                , GET_VEC2(element[HUDDATA_SIZE])
                , GET_VEC2(element[HUDDATA_LISTB_SPACING])
                , element[HUDDATA_TEXT_FONTSIZE].GetFloat()
                , element[HUDDATA_TEXT_FONT].GetString()
                , m_colorPalette[element[HUDDATA_BUTTON_TEXTCOLOR].GetString()]
            );
            m_hudElements.push_back(
                &m_hudElementsListbases.back()
            );
            break;
        }
        if(element.HasMember(HUDDATA_VISIBILITY))
            m_hudElements.back()->setIsVisible(element[HUDDATA_VISIBILITY].GetBool());
    }

    // Intialize popup slots
    if(doc.HasMember("popupSlots"))
    {
        const rapidjson::Value& popupSlotDoc = doc["popupSlots"];
        for (auto iter = popupSlotDoc.MemberBegin(); iter != popupSlotDoc.MemberEnd(); ++iter)
        {
            std::string key = iter->name.GetString();
            auto& value = iter->value;

            this->initPopupSlot(
                key
                ,value[0].GetString()
                ,value[1].GetString()
                ,value[2].GetString()
            );
        }
    }
}

void HUDSystem::updateAbsoluteData(Vector2f newRes) noexcept
{
    for(auto* elem : m_hudElements)
        elem->recalculateAbsoluteData(newRes);
}

void HUDSystem::clearAllElements() noexcept
{
    m_hudElements.clear();
    m_hudElementsText.clear();
    m_hudElementsImages.clear();
    m_hudElementsButtons.clear();
    m_hudElementsRectangles.clear();

    m_colorPalette.clear();
    m_textures.clear();
    m_popupSlots.clear();
}

void HUDSystem::setHUDScale(float newScale) noexcept
{
    m_currentHUDScale = newScale;
    for(auto* elem : m_hudElements)
        elem->setElementScale(newScale);
}

HUDElement* HUDSystem::getHUDElement(const std::string& name) const noexcept
{
    for(auto* element : m_hudElements)
        if(element->getName() == name)
            return element;
    return nullptr;
}
HUDElementText*      HUDSystem::getHUDElementText(const std::string& name) noexcept
{
    for(auto& element : m_hudElementsText) if(element.getName() == name) return &element;
    return nullptr;
}
HUDElementImage*     HUDSystem::getHUDElementImage(const std::string& name) noexcept
{
    for(auto& element : m_hudElementsImages) if(element.getName() == name) return &element;
    return nullptr;
}
HUDElementRectangle* HUDSystem::getHUDElementRectangle(const std::string& name) noexcept
{
    for(auto& element : m_hudElementsRectangles) if(element.getName() == name) return &element;
    return nullptr;
}
HUDElementButton*    HUDSystem::getHUDElementButton(const std::string& name) noexcept
{
    for(auto& element : m_hudElementsButtons) if(element.getName() == name) return &element;
    return nullptr;
}

Vector3f HUDSystem::getColorFromPalette(const std::string& name) const noexcept
{
    for(auto& color : m_colorPalette)
        if(color.first == name)
            return color.second;
    return {0,0,0};
}

std::string HUDSystem::getTextureFromList(const std::string& name) const noexcept
{
    for(auto& texture : m_textures)
        if(texture.first == name)
            return texture.second;
    return {""};
}

void HUDSystem::initPopupSlot(const std::string& slotName, const std::string& textHudElement, const std::string& bgHudElement, const std::string& iconHudElement)
{
    m_popupSlots.insert(HUDSlotPair(slotName, PopupSlot(textHudElement, bgHudElement, iconHudElement)));
}

void HUDSystem::updatePopupTimes()
{
    auto delta = gctx.graphicsEngine.getTimeDiffInSeconds();
    for(auto& slot : m_popupSlots)
    {
        auto& data = slot.second;
        bool changedPopup = data.updateQueue(delta);
        if(changedPopup)
            gctx.hudManager.setPopupData(
                data.getNextElement()
                ,data.getTextElementName()
                ,data.getBackgroundElementName()
                ,data.getIconElementName()
            );
    }
}

void HUDSystem::clearPopupSlot(const std::string& slotName)
{
    auto& data = m_popupSlots.at(slotName);
    data.clearQueue();
    gctx.hudManager.setPopupData(
        nullptr
        ,data.getTextElementName()
        ,data.getBackgroundElementName()
        ,data.getIconElementName()
    );
}

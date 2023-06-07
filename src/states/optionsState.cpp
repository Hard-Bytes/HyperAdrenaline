#include "optionsState.hpp"

OptionsState::OptionsState(GameContext& gc, bool p_ingame) 
: gctx(gc), m_inGame(p_ingame)
{
    // Only update underlying states if ingame
    isTransparent = p_ingame;
}

void
OptionsState::init()
{
    this->hud.loadFromFile(UTIL_HUDOVERLAY_SETTINGS);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));
    m_exitButton            = this->hud.getHUDElementButton("but_escToKit");
    m_applyButton           = this->hud.getHUDElementButton("but_applyChanges");
    m_sectionAudioButton    = this->hud.getHUDElementButton("but_sectionAudio");
    m_sectionGraphicsButton = this->hud.getHUDElementButton("but_sectionGraphics");
    m_sectionUIButton       = this->hud.getHUDElementButton("but_sectionUI");

    if(!m_inGame)
        this->hud.getHUDElementRectangle("rect_background")->setOpacity(1.f);

    this->m_settingSections.reserve(m_nTabs);
    this->m_settingSections.emplace_back(
        gctx
        , "data/overlay/settingsSections/settingsAudio.json"
        , "data/overlay/settingsSections/overlayAudio.json"
    );
    this->m_settingSections.emplace_back(
        gctx
        , "data/overlay/settingsSections/settingsGraphics.json"
        , "data/overlay/settingsSections/overlayGraphics.json"
    );
    this->m_settingSections.emplace_back(
        gctx
        , "data/overlay/settingsSections/settingsUI.json"
        , "data/overlay/settingsSections/overlayUI.json"
    );

    this->resume();
}

void
OptionsState::resume()
{
    if(!m_inGame)
    {
        gctx.gameManager.clearGame();
        gctx.graphicsEngine.clearScene();
    }
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
}

void
OptionsState::update()
{
    auto& ge { gctx.graphicsEngine };
    gctx.audioManager.update();

    ge.beginRender();
    renderSys.update(false);
    m_settingSections[m_currentTab].hud.renderAll(false);
    ge.endRender();

    if(m_exitButton->isPressed() || ge.getKeySinglePress(Key::ESC))
    {
        m_alive = false;
    }
    else if(ge.getKeySinglePress(Key::RIGHT))       { if(m_currentTab < (m_nTabs-1)) ++m_currentTab; }
    else if(ge.getKeySinglePress(Key::LEFT))        { if(m_currentTab > 0) --m_currentTab; }
    else if(m_sectionAudioButton->isPressed())      { m_currentTab = 0; }
    else if(m_sectionGraphicsButton->isPressed())   { m_currentTab = 1; }
    else if(m_sectionUIButton->isPressed())         { m_currentTab = 2; }
    else if(m_applyButton->isPressed())
    {
        m_hasChanges = false;
        auto& setman { gctx.settingsManager };
        for(auto& section : m_settingSections)
            for(auto& set : section.settings)
            {
                if(set.hasChanges())
                {
                    set.applyFunction(set, gctx);
                    set.applyChanges(setman);
                }
                set.reloadInitialValue();
            }
    }

    auto& sets { m_settingSections[m_currentTab].settings };
    for(auto& set : sets)
    {
        auto* elem { set.hudElementEditor };
        auto elemType { elem->getElementType() };
        switch(elemType)
        {
            case HUD_CHECKBOX:
                updateCheckboxElement(elem, set); break;
            case HUD_SLIDERINT:
                updateSliderIntElement(elem, set); break;
            case HUD_SLIDERFLOAT:
                updateSliderFloatElement(elem, set); break;
            default: break;
        }
    }

    ge.updateMouseControllerVariables();
    ge.updateKeyReleaseTrigger();
}

void
OptionsState::close()
{
    if(!m_inGame)
    {
        gctx.gameManager.clearGame();
        gctx.graphicsEngine.clearScene();
    }
}

bool 
OptionsState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

bool
OptionsState::updateCheckboxElement(HUDElement* baseElem, Setting& set)
{
    auto* elem { static_cast<HUDElementCheckbox*>(baseElem) };
    bool val { elem->getDataContainer() };
    if(val == set.currentValue_b) return set.hasChanges();
    set.currentValue_b = val;

    return set.hasChanges();
}

bool
OptionsState::updateSliderIntElement(HUDElement* baseElem, Setting& set)
{
    auto* elem { static_cast<HUDElementSlider*>(baseElem) };
    int val { elem->getDataContainerInt() };
    if(val == set.currentValue_f) return set.hasChanges();
    set.currentValue_f = val;

    return set.hasChanges();
}

bool
OptionsState::updateSliderFloatElement(HUDElement* baseElem, Setting& set)
{
    auto* elem { static_cast<HUDElementSlider*>(baseElem) };
    float val { elem->getDataContainerFloat() };
    if(val == set.currentValue_f) return set.hasChanges();
    set.currentValue_f = val;

    return set.hasChanges();
}

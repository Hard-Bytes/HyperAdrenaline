#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"
#include "optionsStateHelpers/optionHelperStructs.hpp"
#include "optionsStateHelpers/optionApplyFunc.hpp"

struct OptionsState : public State
{
    explicit OptionsState(GameContext& gc, bool p_ingame);
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    std::vector<SettingsSection> m_settingSections {};
    HUDElementButton* m_exitButton {nullptr};
    HUDElementButton* m_applyButton {nullptr};

    HUDElementButton* m_sectionAudioButton {nullptr};
    HUDElementButton* m_sectionGraphicsButton {nullptr};
    HUDElementButton* m_sectionUIButton {nullptr};

    int m_currentTab { 0 };
    int m_nTabs { 3 };
    bool m_inGame { false };
    bool m_hasChanges { false };

    bool m_alive { true };

    bool updateCheckboxElement(HUDElement* elem, Setting& set);
    bool updateSliderIntElement(HUDElement* elem, Setting& set);
    bool updateSliderFloatElement(HUDElement* elem, Setting& set);
};

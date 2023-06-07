#include "menuState.hpp"
#include "levelSelectionState.hpp"
#include "onlineSetupState.hpp"
#include "loadState.hpp"
#include "codexState.hpp"
#include "decisionState.hpp"
#include "optionsState.hpp"
#include <chrono>

MenuState::MenuState(StateManager& sm, GameContext& gc) 
: m_statemanager(sm), gctx(gc)
{
}

void
MenuState::init()
{ 
    // Load assets (load another state that will close itself)
    m_statemanager.pushState<LoadState>(gctx);

    // Initialize this hud
    this->hud.loadFromFile(UTIL_HUDOVERLAY_MAIN_MENU);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));
}

void
MenuState::resume()
{
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
    
    // Get random index
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    int selection = nanos % 2;

    // Get data
    Vector3f cameraPos        { 0   , 0   , 0    };
    Vector3f cameraInitLookat { -0.5, -0.2, 0.5 };
    std::string mapmesh { "assets/maps/SalaMenu1.obj" };
    bool useSkybox { true };
    if(selection==1)
    {
        cameraPos.set       ( 2  , 7   , 0    );
        cameraInitLookat.set(-0.5, -0.2, -0.5 );
        mapmesh = "assets/maps/SalaMenu2.obj";
        useSkybox = false;
    }

    auto& gameManager { gctx.gameManager };
    auto& graphicsEngine { gctx.graphicsEngine };
    this->cameraId = gameManager.createCameraEntity(cameraPos, cameraInitLookat.normalize());

    if(useSkybox)
        graphicsEngine.createSkybox();

    if(graphicsEngine.getDynamicLightning())
    {
        graphicsEngine.createLightNode(
            {-2,2,0}, (int)hyen::LightType::Point, 100
            , {0.06,0.06,0.06}
            , {0.7,0.7,0.7}
            , {0.5,0.5,0.5}
            , 0.3f
            , 0.009f
            , 0.0032f
        );
        graphicsEngine.createLightNode(
            {10,3,0}, (int)hyen::LightType::Point, 100
            , {0.06,0.06,0.06}
            , {0.7,0.7,0.7}
            , {0.5,0.5,0.5}
            , 0.3f
            , 0.009f
            , 0.0032f
        );
        graphicsEngine.createLightNode(
            {15,-4,57}, (int)hyen::LightType::Point, 100
            , {0.02,0.02,0.02}
            , {0.6,0.6,0.6}
            , {0.5,0.5,0.5}
            , 0.5f
            , 0.009f
            , 0.00032f
        );
    }
    else
    {
        gameManager.createBaseLight();
        gameManager.clearLightList();
    }
    auto* roomnode = graphicsEngine.createNode(
        {0,0,0}
        ,Vector3f{1,1,1}
        ,mapmesh
    );
    delete roomnode;
}

void
MenuState::update()
{
    gctx.audioManager.update();
    renderSys.updateAll(false);

    behaviourSystem.updateAll();

    if(gctx.graphicsEngine.getKeyKeyboardPress(Key::DOWN))
    {
        m_selection = (m_selection + 1) % m_maxSelection;
        repositionateArrow();
    }
    else if(gctx.graphicsEngine.getKeyKeyboardPress(Key::UP))
    {
        --m_selection;
        if(m_selection < 0)
            m_selection = m_maxSelection - 1;
        repositionateArrow();
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ENTER))
    {
        switch (m_selection)
        {
        case 0:
            m_statemanager.pushState<LevelSelectionState>(m_statemanager, gctx, false);
            break;
        case 1:
            m_statemanager.pushState<OnlineSetupState>(m_statemanager, gctx);
            break;
        case 2:
            m_statemanager.pushState<CodexState>(m_statemanager, gctx);
            break;
        case 3:
            m_statemanager.pushState<OptionsState>(gctx, false);
            break;
        case 4:
            m_statemanager.pushState<DecisionState>(gctx, m_alive, UTIL_HUDOVERLAY_SELECTION_EXITGAME);
            gctx.gameManager.clearGame();
            gctx.graphicsEngine.clearScene();
            break;
        }
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
    {
        m_statemanager.pushState<DecisionState>(gctx, m_alive, UTIL_HUDOVERLAY_SELECTION_EXITGAME);
        gctx.gameManager.clearGame();
        gctx.graphicsEngine.clearScene();
    }

    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void
MenuState::close()
{
    // Stop audio
    gctx.audioManager.stopAllAudio();
    gctx.audioManager.update();

    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
MenuState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void MenuState::repositionateArrow()
{
    auto sel = hud.getHUDElement("text_option"+std::to_string(m_selection));
    auto arrow = hud.getHUDElement("text_arrow");
    auto pos = arrow->getRelativePosition();
    pos.y = sel->getRelativePosition().y;
    arrow->setRelativePosition(pos);
}

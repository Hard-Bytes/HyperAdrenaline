#include "loadState.hpp"

LoadState::LoadState(GameContext& gc) 
: gctx(gc)
{
}

void
LoadState::init()
{
    this->hud.loadFromFile(UTIL_HUDOVERLAY_LOADING_SCREEN);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    // Load the screen
    renderSys.updateAll(false);

    // Assign audio properties
    auto& setman   { gctx.settingsManager };
    auto& audioman { gctx.audioManager };
    audioman.setVolume(SOUNDS_GLOBAL,       setman.get<float>("audio/globalVolume"));
    audioman.setVolume(SOUNDS_MUSIC,        setman.get<float>("audio/musicVolume"));
    audioman.setVolume(SOUNDS_VOICE,        setman.get<float>("audio/voiceVolume"));
    audioman.setVolume(SOUNDS_SFX_AMBIENCE, setman.get<float>("audio/sfxAmbienceVolume"));
    audioman.setVolume(SOUNDS_SFX_COMBAT,   setman.get<float>("audio/sfxCombatVolume"));

    // Assign various initial properties
    auto& graph { gctx.graphicsEngine };
    graph.setDynamicLightning(  setman.get<bool> (  "graphics/useDynamicLightning"  ));
    graph.setAnimationsEnabled( setman.get<bool>(   "graphics/useAnimations"        ));
    graph.setSkyboxQuality(     setman.get<int>(    "graphics/skyboxQuality"        ));
    graph.setParticleQuality(   setman.get<int>(    "graphics/particleQuality"      ));
    graph.setFieldOfView(       setman.get<float>(  "graphics/fov"                  ));
    graph.setMaxRenderDistance( setman.get<float>(  "graphics/maxRenderDistance"    ));
    graph.setScreenResolution(  setman.get<float>(  "graphics/resolutionX")
                                , setman.get<float>("graphics/resolutionY")         );
    gctx.gameManager.showUI  =  setman.get<bool>("ui/showUI");
    gctx.gameManager.showFPS =  setman.get<bool>("ui/showFPS");

    std::string objetos[11] =
    {
        MODEL_WEAPON_HANDGUN,
        MODEL_WEAPON_SHOTGUN,
        MODEL_WEAPON_MACHINEGUN,
        MODEL_WEAPON_SEMIAUTOMATIC,
        MODEL_WEAPON_SNIPER,
        MODEL_WEAPON_EXPLOSIVE,
        MODEL_WEAPON_EXPLOSIVE_BULLET,
        MODEL_WEAPON_CANNON,
        MODEL_WEAPON_CANNON_BULLET,
        MODEL_OBJECT_HEAL_KIT,
        MODEL_OBJECT_AMMO_KIT
    };

    for(auto model: objetos)
    {
        auto* node = graph.createNode(
            {0,0,-100}
            ,Vector3f{1,1,1}
            ,model
            ,false
        );
        delete node;
    }

    std::string objetosAnim[3] =
    {
        MODEL_CHARAC_ENEMY_MELEE
        ,MODEL_CHARAC_ENEMY_SHOOTER
        ,MODEL_CHARAC_ENEMY_KAMIKAZE
    };

    for(auto model: objetosAnim)
    {
        auto* node = graph.createAnimatedNode(
            {0,0,-100}
            ,Vector3f{1,1,1}
            ,model
            , 0.05f
            ,false
        );
        delete node;
    }
}

void
LoadState::resume()
{}

void
LoadState::update()
{
    // This state's purpose is loading the assets (see init())
    // So when we reach update(), destroy the state and return to previous state
    m_alive = false;
}

void
LoadState::close()
{
    // Stop audio
    gctx.audioManager.stopAllAudio();
    // Set background music
    gctx.audioManager.playSound(SOUND_MUSIC_MAIN_MENU, SOUNDS_MUSIC);
    gctx.audioManager.update();

    gctx.graphicsEngine.clearScene();
}

bool 
LoadState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

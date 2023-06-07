#include "optionApplyFunc.hpp"
#include <iostream>
#include "../../context/gameContext.hpp"
#include "optionHelperStructs.hpp"
#include "../../util/macros.hpp"

void
applyFuncMasterVolume(Setting& set, GameContext& gctx)
{
    gctx.audioManager.setVolume(SOUNDS_GLOBAL, set.currentValue_f);
}

void
applyFuncMusicVolume(Setting& set, GameContext& gctx)
{
    gctx.audioManager.setVolume(SOUNDS_MUSIC, set.currentValue_f);
}

void
applyFuncVoiceVolume(Setting& set, GameContext& gctx)
{
    gctx.audioManager.setVolume(SOUNDS_VOICE, set.currentValue_f);
}

void
applyFuncAmbienceSFXVolume(Setting& set, GameContext& gctx)
{
    gctx.audioManager.setVolume(SOUNDS_SFX_AMBIENCE, set.currentValue_f);
}

void
applyFuncCombatSFXVolume(Setting& set, GameContext& gctx)
{
    gctx.audioManager.setVolume(SOUNDS_SFX_COMBAT, set.currentValue_f);
}


void
applyFuncResolution(Setting& set, GameContext& gctx)
{
    // TODO:: Use a special setting with two floats
    // Note that this code is not executing at all yet
    gctx.graphicsEngine.setScreenResolution(
        gctx.settingsManager.get<float>("graphics/resolutionX")
        , gctx.settingsManager.get<float>("graphics/resolutionY")
    );
}

void
applyFuncUseDynamicLight(Setting& set, GameContext& gctx)
{
    bool dyn { set.currentValue_b };
    gctx.graphicsEngine.setDynamicLightning(dyn);
    gctx.gameManager.setDynamicLightning(dyn);
}

void
applyFuncFov(Setting& set, GameContext& gctx)
{
    gctx.graphicsEngine.setFieldOfView(set.currentValue_f);
}

void
applyFuncMaxRenderDistance(Setting& set, GameContext& gctx)
{
    gctx.graphicsEngine.setMaxRenderDistance(set.currentValue_f);
}

void
applyFuncUseAnimations(Setting& set, GameContext& gctx)
{
    gctx.graphicsEngine.setAnimationsEnabled(set.currentValue_b);
    // TODO:: Load / Unload resources of animated models
}

void
applyFuncSkyboxQuality(Setting& set, GameContext& gctx)
{
    int skyb { (int)set.currentValue_f };
    gctx.graphicsEngine.setSkyboxQuality(skyb);
    // TODO:: Free resources of current skybox / all skyboxes
    gctx.graphicsEngine.createSkybox();
}

void
applyFuncParticleQuality(Setting& set, GameContext& gctx)
{
    gctx.graphicsEngine.setParticleQuality((int)set.currentValue_f);
    // TODO:: Reload particles
}


void
applyFuncShowUI(Setting& set, GameContext& gctx)
{
    gctx.gameManager.showUI = set.currentValue_b;
}

void
applyFuncUIScale(Setting& set, GameContext& gctx)
{
    // TODO::
}

void
applyFuncShowFPS(Setting& set, GameContext& gctx)
{
    gctx.gameManager.showFPS = set.currentValue_b;
}

#pragma once

struct Setting;
class GameContext;

// Behaviour Functions
typedef void (*SettingApplyFunction) (Setting&, GameContext&);

void applyFuncMasterVolume(Setting&, GameContext&);
void applyFuncMusicVolume(Setting&, GameContext&);
void applyFuncVoiceVolume(Setting&, GameContext&);
void applyFuncAmbienceSFXVolume(Setting&, GameContext&);
void applyFuncCombatSFXVolume(Setting&, GameContext&);

void applyFuncResolution(Setting&, GameContext&);
void applyFuncUseDynamicLight(Setting&, GameContext&);
void applyFuncFov(Setting&, GameContext&);
void applyFuncMaxRenderDistance(Setting&, GameContext&);
void applyFuncUseAnimations(Setting&, GameContext&);
void applyFuncSkyboxQuality(Setting&, GameContext&);
void applyFuncParticleQuality(Setting&, GameContext&);

void applyFuncShowUI(Setting&, GameContext&);
void applyFuncUIScale(Setting&, GameContext&);
void applyFuncShowFPS(Setting&, GameContext&);

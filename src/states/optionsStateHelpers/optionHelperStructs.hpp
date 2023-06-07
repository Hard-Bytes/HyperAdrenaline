#pragma once
#include "optionApplyFunc.hpp"
#include "../../hud/hudElement.hpp"
#include "../../system/hud.hpp"

using ConstStr = const std::string&;
struct Setting
{
    explicit Setting(HUDElement* p_element, ConstStr p_settingName, float p_value)
        : hudElementEditor(p_element)
        , settingNameInFile(p_settingName)
        , initialValue_f(p_value)
        , currentValue_f(p_value)
    {
        setValueToHUD();
        setApplyFunction();
    }
    explicit Setting(HUDElement* p_element, ConstStr p_settingName, bool p_value)
        : hudElementEditor(p_element)
        , settingNameInFile(p_settingName)
        , initialValue_b(p_value)
        , currentValue_b(p_value)
    {
        setValueToHUD();
        setApplyFunction();
    }

    void applyChanges(SettingsManager& setman)
    {
        auto type { hudElementEditor->getElementType() };
        switch(type)
        {
        case HUD_CHECKBOX:
            setman.set<bool>(settingNameInFile, currentValue_b); break;
        case HUD_SLIDERINT:
            setman.set<int>(settingNameInFile, (int)currentValue_f); break;
        case HUD_SLIDERFLOAT:
            setman.set<float>(settingNameInFile, currentValue_f); break;
        default: break;
        }
    }
    void setValueToHUD()
    {
        auto type { hudElementEditor->getElementType() };
        switch(type)
        {
        case HUD_CHECKBOX:
            static_cast<HUDElementCheckbox*>(hudElementEditor)->setDataContainerValue(currentValue_b); break;
        case HUD_SLIDERINT:
            static_cast<HUDElementSlider*>(hudElementEditor)->setDataContainerValueInt((int)currentValue_f); break;
        case HUD_SLIDERFLOAT:
            static_cast<HUDElementSlider*>(hudElementEditor)->setDataContainerValueFloat(currentValue_f); break;
        default: break;
        }
    }
    void reloadInitialValue()
    {
        initialValue_b = currentValue_b;
        initialValue_f = currentValue_f;
    }
    bool hasChanges()
    {
        bool result { false };
        auto type { hudElementEditor->getElementType() };
        switch(type)
        {
            case HUD_CHECKBOX:
                result = (initialValue_b != currentValue_b); break;
            case HUD_SLIDERINT:
            case HUD_SLIDERFLOAT:
                result = (initialValue_f != currentValue_f); break;
            default: break;
        }
        return result;
    }
    void setApplyFunction()
    {
        auto it { applyFunctionMap.find(settingNameInFile) };
        if(it != applyFunctionMap.end())
            applyFunction = applyFunctionMap[settingNameInFile];
    }

    HUDElement* hudElementEditor;
    std::string settingNameInFile;
    bool initialValue_b;
    bool currentValue_b;
    float initialValue_f;
    float currentValue_f;

    SettingApplyFunction applyFunction { nullptr };

    using ApplyFuncMap = std::unordered_map<std::string, SettingApplyFunction>; 
    static inline ApplyFuncMap applyFunctionMap {
         { "audio/globalVolume"           , applyFuncMasterVolume }
        ,{ "audio/musicVolume"            , applyFuncMusicVolume }
        ,{ "audio/voiceVolume"            , applyFuncVoiceVolume }
        ,{ "audio/sfxAmbienceVolume"      , applyFuncAmbienceSFXVolume }
        ,{ "audio/sfxCombatVolume"        , applyFuncCombatSFXVolume }

        ,{ "graphics/resolutionX"         , applyFuncResolution }
        ,{ "graphics/useDynamicLightning" , applyFuncUseDynamicLight }
        ,{ "graphics/fov"                 , applyFuncFov }
        ,{ "graphics/maxRenderDistance"   , applyFuncMaxRenderDistance }
        ,{ "graphics/useAnimations"       , applyFuncUseAnimations }
        ,{ "graphics/skyboxQuality"       , applyFuncSkyboxQuality }
        ,{ "graphics/particleQuality"     , applyFuncParticleQuality }

        ,{ "ui/showUI"                    , applyFuncShowUI }
        ,{ "ui/uiScale"                   , applyFuncUIScale }
        ,{ "ui/showFPS"                   , applyFuncShowFPS }
    };
};
struct SettingsSection
{
    explicit SettingsSection(GameContext& gc, ConstStr dataFile, ConstStr hudFile)
        : hud {gc}
    {
        hud.loadFromFile(hudFile);
        this->loadSettingsFromFile(dataFile, gc.settingsManager);
    }
    ~SettingsSection() = default;

    HUDSystem hud;
    std::vector<Setting> settings;

private:
    void loadSettingsFromFile(ConstStr file, SettingsManager& setman)
    {
        rapidjson::Document doc;
        openJsonFile(file, doc);

        unsigned int numElements { doc.Size() };
        settings.reserve(numElements);

        for(unsigned int i = 0; i<numElements; ++i)
        {
            auto& data = doc[i];
            auto settingName { data["settingName"].GetString() };

            auto* hudElem { hud.getHUDElement(data["hudElement"].GetString()) };
            auto hudType { hudElem->getElementType() };
            switch(hudType)
            {
            case HUD_CHECKBOX:
                settings.emplace_back(
                    hudElem
                    , settingName
                    , setman.get<bool>(settingName)
                );
                break;
            case HUD_SLIDERINT:
                settings.emplace_back(
                    hudElem
                    , settingName
                    , (float)setman.get<int>(settingName)
                );
                break;
            case HUD_SLIDERFLOAT:
                settings.emplace_back(
                    hudElem
                    , settingName
                    , setman.get<float>(settingName)
                );
                break;
            default: break;
            }
        }
    }
};

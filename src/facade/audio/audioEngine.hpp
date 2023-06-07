#pragma once

 #include <fmod.h>
 #include <fmod_studio.h>
//extern "C" {
// #include "fmod_studio.hpp"
// #include "fmod.hpp"
//}

#include <map>
#include <string>
#include "../vector3f.hpp"

/*
Para HardBytes del futuro:
    Este motor solo usa Master.banks. A diferencia de lo que yo creía, si hay varios
    bancos cargados, el sistema sabe buscar cada evento en su sitio, por lo que cargar
    varios bancos a la vez no supone un problema. Cuando los bancos empiecen a crecer
    podemos hacer que vayan cargando y descargando los bancos necesarios (que son los que
    verdaderamente consumen memoria en el juego, y no las descripciones/instancias de los sonidos)
*/
enum Sound_Type
{
    SOUNDS_GLOBAL = 0,
    SOUNDS_VOICE,
    SOUNDS_MUSIC,
    SOUNDS_SFX_AMBIENCE,
    SOUNDS_SFX_COMBAT,
    SOUND_TYPE_SIZE// This must be the last element, don't you dare set something below this one
};

// Alias for FMOD types
using SoundDescription      = FMOD_STUDIO_EVENTDESCRIPTION;
using SoundInstance         = FMOD_STUDIO_EVENTINSTANCE;
// A map that storages sound descriptions, identified by a string
using SoundVector           = std::map<std::string, SoundDescription*>;
// A map that storages sound instances, each identified by a string
using SoundInstanceVector   = std::map<std::string, SoundInstance*>;
// Struct made to hold an instance vector and its volume
struct SoundTypeStorage
{
    SoundInstanceVector map{};
    float volume {1};
};
// A map that storages every SoundInstanceVector, each identified with its Sound_Type
using SoundStorage          = std::map<Sound_Type, SoundTypeStorage>;
// Used to instantiate the maps
using SoundPair             = std::pair<Sound_Type, SoundTypeStorage>;

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    void update();

    void stopAllAudio();
    void stopAllAudioOfType(Sound_Type);

    virtual void playSound(const std::string& evt, Sound_Type type);
    virtual void play3DSound(const std::string& evt, Sound_Type type, const Vector3f& pos, const Vector3f& speed={0,0,0}, const Vector3f& direction={0,0,0});
    
    float getSoundParameter(std::string evt, std::string param , Sound_Type type);
    void setSoundParameter(std::string evt, std::string param, float value, Sound_Type type);
    void setSoundParameterForType(std::string param, float value, Sound_Type type);

    void setListener3DPosition(const Vector3f& pos);

    void increaseVolume(Sound_Type, float);
    float getVolume(Sound_Type);
    void setVolume(Sound_Type, float);
private:
    // Private methods
    bool createInstance(std::string, SoundInstanceVector&, Sound_Type);
    void adjustInstanceVolumes(Sound_Type);
    void setEventParameter(SoundInstance* instance, SoundDescription* description, std::string param, float value);

    // System and banks
    FMOD_STUDIO_SYSTEM* system{nullptr};
    FMOD_STUDIO_BANK* masterBank{nullptr};
    FMOD_STUDIO_BANK* stringsBank{nullptr};

    // Sound descriptions and instances
    SoundVector eventDescriptions;
    SoundStorage eventInstances;
};

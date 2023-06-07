#include "audioEngine.hpp"
#include "../../util/macros.hpp"
#include "../../util/paths.hpp"
#include <iostream>

// Constructor
AudioEngine::AudioEngine()
{
    FMOD_Studio_System_Create(&this->system, FMOD_VERSION);
    //FMOD_STUDIO_SYSTEM *system, int maxchannels, FMOD_STUDIO_INITFLAGS studioflags, FMOD_INITFLAGS flags, void *extradriverdata
    //                          MAX CHANNELS, STUDIO FLAGS, INNIT FLAGS, EXTRADRIVERDATA 
    FMOD_Studio_System_Initialize(this->system, 1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL,nullptr);
    FMOD_Studio_System_LoadBankFile(this->system,"assets/audio/Master.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
    FMOD_Studio_System_LoadBankFile(this->system,"assets/audio/Master.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank);

    // Initialize all sound vectors
    for(int i=0; i<SOUND_TYPE_SIZE; ++i)
        eventInstances.insert(SoundPair((Sound_Type)i, SoundTypeStorage()));
}

// Destructor
AudioEngine::~AudioEngine()
{
    FMOD_Studio_System_Release(this->system);
    stopAllAudio();
    eventDescriptions.clear();
}

/*Update*/
// Updates the audio system. This should be done in the game's main loop
void AudioEngine::update()
{
    FMOD_Studio_System_Update(this->system);
}

/*Stop All Audio*/
// Stops all the audio
void AudioEngine::stopAllAudio()
{
    for(auto& evt : eventDescriptions)
        FMOD_Studio_EventDescription_ReleaseAllInstances(evt.second);
        //evt.second->releaseAllInstances();
    for(int i=0; i<SOUND_TYPE_SIZE; ++i)
        eventInstances[(Sound_Type)i].map.clear();
}

/*Stop All Audio Of Type*/
// Stops all the audio given a category
void AudioEngine::stopAllAudioOfType(Sound_Type type)
{
    if(type == SOUNDS_GLOBAL)
    {
        stopAllAudio();
        return;
    }
    auto& instances = eventInstances[type].map;
    for(auto& evt : instances)
    {
        FMOD_Studio_EventInstance_Stop(evt.second, FMOD_STUDIO_STOP_IMMEDIATE);
        //evt.second->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        FMOD_Studio_EventInstance_Release(evt.second);
        //evt.second->release();
    }
    instances.clear();
}

/*Play Sound*/
// Plays the given event instance if it exists, or creates it before if it doesn't
void AudioEngine::playSound(const std::string& name, Sound_Type type)
{
    // Check if the instance exists, and try to create it if not
    SoundInstanceVector &instanceVec = eventInstances[type].map;
    bool exists = createInstance(name, instanceVec, type);
    if(!exists) return;

    // Get references
    auto instance = instanceVec[name];

    // Set pan3d to zero to avoid 3d effect
    FMOD_BOOL is3D;
    //eventDescriptions[name]->is3D(&is3D);
    FMOD_Studio_EventDescription_Is3D(eventDescriptions[name], &is3D);
    if(is3D)
    {
        auto description = this->eventDescriptions[name];
        this->setEventParameter(instance, description, SOUND_PARAM_PAN3D, 1.f);
    }

    // Play the sound if possible
    //instance->start();
    FMOD_Studio_EventInstance_Start(instance);
}

/*Play Sound in 3D*/
// Plays the given event instance if it exists, or creates it before if it doesn't
void AudioEngine::play3DSound(const std::string& name, Sound_Type type, const Vector3f& pos, const Vector3f& speed, const Vector3f& direction)
{
    // Check if the instance exists, and try to create it if not
    SoundInstanceVector &instances = eventInstances[type].map;
    bool exists = createInstance(name, instances, type);
    if(!exists) return;
    auto instance = instances[name];
    auto description = this->eventDescriptions[name];
    
    // If it is a 2D event, throw an error
    FMOD_BOOL is3D;
    //description->is3D(&is3D);
    FMOD_Studio_EventDescription_Is3D(description,&is3D);
    if(!is3D) 
    {
        LOG_ERR("Throwing 2D sound as 3D sound: " << name.c_str());
        return;
    }

    // Set pan3D to use 3D positioning
    this->setEventParameter(instance, description, SOUND_PARAM_PAN3D, 0.f);

    // Get its 3D attributes
    FMOD_3D_ATTRIBUTES attributes;
    FMOD_RESULT res = FMOD_Studio_EventInstance_Get3DAttributes(instance, &attributes);//instance->get3DAttributes(&attributes);
    
    if(res > 0) // Error! Attributes not found
    {
        LOG_ERR("FMod 3D Attributes not found in [" << name.c_str() << "] (Error code: " << res << ")");
        return;
    }

    // Modify them and reassign them
    attributes.position = {pos.x,pos.y,pos.z};
    attributes.velocity = {speed.x,speed.y,speed.z};
    if(direction.length() > 0)
        attributes.forward = {direction.x,direction.y,direction.z};
    attributes.up = {0.0f,1.0f,0.0f};

    //instance->set3DAttributes(&attributes);
    FMOD_Studio_EventInstance_Set3DAttributes(instance, &attributes);
    // Play the sound
    //instance->start();
    FMOD_Studio_EventInstance_Start(instance);
}

/*Gets Sound Parameter*/
// Gets a value for the given parameter of the given event
float AudioEngine::getSoundParameter(std::string evt, std::string param, Sound_Type type)
{
    // Check if the instance exists, and try to create it if not
    SoundInstanceVector &instance = eventInstances[type].map;
    bool exists = createInstance(evt, instance, type);
    if(!exists) return 0;

    // Get the parameter data
    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
    //this->eventDescriptions[evt]->getParameterDescriptionByName(param.c_str(), &paramDesc);
    FMOD_Studio_EventDescription_GetParameterDescriptionByName(eventDescriptions[evt], param.c_str(), &paramDesc);

    // get the parameter
    float value;
    //instance[evt]->getParameterByID(paramDesc.id, &value );
    FMOD_Studio_EventInstance_GetParameterByID(instance[evt], paramDesc.id, &value, 0); //last param is final value, send 0 to ignore
    
    return value;
}

/*Set Sound Parameter*/
// Sets a value for the given parameter of the given event
void AudioEngine::setSoundParameter(std::string evt, std::string param, float value, Sound_Type type)
{
    // Check if the instance exists, and try to create it if not
    SoundInstanceVector &instanceVec = eventInstances[type].map;    
    bool exists = createInstance(evt, instanceVec, type);
    if(!exists) return;

    auto instance = instanceVec[evt];
    auto description = eventDescriptions[evt];
    this->setEventParameter(instance, description, param, value);
}

/*Set Sound Parameter For Type*/
// Sets a value to the specified parameter to all the sounds of the specified type
void AudioEngine::setSoundParameterForType(std::string param, float value, Sound_Type type)
{
    auto& evts = eventInstances[type].map;
    for(auto& evt : evts)
        setSoundParameter(evt.first, param, value, type);
}

void AudioEngine::setListener3DPosition(const Vector3f& pos)
{
    FMOD_3D_ATTRIBUTES attributes;
    int listener = 0;
    FMOD_RESULT res = FMOD_Studio_System_GetListenerAttributes(this->system, listener, &attributes);
    //system->getListenerAttributes(listener, &attributes);
    

    if(res > 0) 
    {
        LOG_ERR("FMod could not get listener 3D attributes");
        return;
    }

    attributes.position = {pos.x, pos.y, pos.z};
    //system->setListenerAttributes(listener, &attributes);
    FMOD_Studio_System_SetListenerAttributes(this->system, listener, &attributes);
}

/*Increase Volume*/
// Increase the volume of the given type by the specified amount
void AudioEngine::increaseVolume(Sound_Type type, float value)
{
    float volume = eventInstances[type].volume;
    volume += value;

    if(volume > 1.0f) volume = 1.0f;
    if(volume < 0.0f) volume = 0.0f;

    eventInstances[type].volume = volume;
    this->adjustInstanceVolumes(type);
}

/*Get Global Volume*/
// Returns the value (range [0,1]) of the global volume
float AudioEngine::getVolume(Sound_Type type)
{
    return this->eventInstances[type].volume;
}

/*Set Global Volume*/
// Sets the value of the global volume (capped at [0,1])
void AudioEngine::setVolume(Sound_Type type, float value)
{
    if(value > 1.0f) value = 1.0f;
    if(value < 0.0f) value = 0.0f;
    eventInstances[type].volume = value;

    this->adjustInstanceVolumes(type);
}

/*Create Instance*/
// Create an instance of the given sound if it didn't exist before
bool AudioEngine::createInstance(std::string name, SoundInstanceVector& instanceVector, Sound_Type type)
{
    // Check if we already have that sound's intance
    auto it = instanceVector.find(name);
    if(it == instanceVector.end())
    {
        // We don't have the instance, search it in the loaded events
        auto itDesc = eventDescriptions.find(name);
        if(itDesc == eventDescriptions.end())
        {
            // We don't have that event, try to load it from audio system
            std::string base = "event:/";
            SoundDescription* eventDescription = nullptr;
            //system->getEvent((base+name).c_str(), &eventDescription);
            FMOD_Studio_System_GetEvent(this->system, (base+name).c_str(), &eventDescription);

            // If that event does not exist, stop
            if(!eventDescription) 
            {
                LOG_ERR("Can't find sound named " << name.c_str());
                return false;
            }

            // If it exists, add it
            eventDescriptions[name] = eventDescription;
        }

        // Add the instance to the instance list (map)
        SoundInstance* eventInstance = nullptr;
        //eventDescriptions[name]->createInstance(&eventInstance);
        FMOD_Studio_EventDescription_CreateInstance(eventDescriptions[name], &eventInstance);
        instanceVector[name] = eventInstance;

        // Set global volume
        float volume = eventInstances[type].volume;
        //eventInstance->setVolume(this->eventInstances[SOUNDS_GLOBAL].volume * volume);
        FMOD_Studio_EventInstance_SetVolume(eventInstance, this->eventInstances[SOUNDS_GLOBAL].volume * volume);
    }

    return true;
}

/*Adjust Instance Volumes*/
// Adjust the volume of all active instances (not that bad)
void AudioEngine::adjustInstanceVolumes(Sound_Type queryType)
{
    if(queryType == SOUNDS_GLOBAL)
    {
        for(int type=1; type<SOUND_TYPE_SIZE; ++type)
            adjustInstanceVolumes((Sound_Type)type);
    }
    else
    {
        SoundTypeStorage& events = this->eventInstances[(Sound_Type)queryType];
        float globalVolume = this->eventInstances[SOUNDS_GLOBAL].volume;
        for(auto& insta : events.map)
            //insta.second->setVolume(events.volume * globalVolume);
            FMOD_Studio_EventInstance_SetVolume(insta.second, events.volume * globalVolume);
    }
}

/*Set Event Parameter*/
// Sets a parameter value given the sound instance instead of the string name
void AudioEngine::setEventParameter(SoundInstance* instance, SoundDescription* description, std::string param, float value)
{
    // Get the parameter data
    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
    //description->getParameterDescriptionByName(param.c_str(), &paramDesc);
    FMOD_Studio_EventDescription_GetParameterDescriptionByName(description, param.c_str(), &paramDesc);
    auto& id = paramDesc.id;

    // Error message, hidden because sometimes we just need to call
    // setParameter without knowing if the sound has that parameter or not
    /*
    if(id.data1 == 0 && id.data2 == 0)
        LOG_ERR("Can't find parameter " << param);
    */

    // Update the parameter
    //instance->setParameterByID(id, value);
    FMOD_Studio_EventInstance_SetParameterByID(instance, id, value, 0);
}

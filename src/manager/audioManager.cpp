#include "audioManager.hpp"

AudioManager::AudioManager()
    : AudioEngine()
{}

AudioManager::~AudioManager()
{}

void AudioManager::playSound(const std::string& evt, Sound_Type type)
{
    this->AudioEngine::playSound(evt, type);
}

void AudioManager::play3DSound(const std::string& evt, Sound_Type type, const Vector3f& pos, const Vector3f& speed, const Vector3f& direction)
{
    this->AudioEngine::play3DSound(evt, type, pos, speed, direction);
}

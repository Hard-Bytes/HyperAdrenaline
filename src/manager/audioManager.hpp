#pragma once
#include "../facade/audio/audioEngine.hpp"
#include "../sharedTypes/serverEnums.hpp"

class AudioManager : public AudioEngine
{
public:
    AudioManager();
    ~AudioManager();

    virtual void playSound(const std::string& evt, Sound_Type type) final;
    virtual void play3DSound(const std::string& evt, Sound_Type type, const Vector3f& pos, const Vector3f& speed={0,0,0}, const Vector3f& direction={0,0,0}) final;

    // Look management
    constexpr void       setPlayerLook(PlayerLook nl) noexcept
        { look = nl; }
    constexpr PlayerLook getPlayerLook() const noexcept
        { return look; }
private:
    PlayerLook look;
};

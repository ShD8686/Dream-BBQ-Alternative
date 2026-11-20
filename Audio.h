#pragma once
#include <memory>
#include <string>

class Audio
{
public:
    Audio();
    ~Audio();

    bool Init();
    void Shutdown();

    void PlayMusic(const std::string& path, float volume = 0.55f);
    void StopMusic();
    void SetMusicVolume(float volume);

    void PlayAmbient(const std::string& path, float volume = 0.45f);
    void StopAmbient();
    void SetAmbientVolume(float volume);

    void PlaySFX(const std::string& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
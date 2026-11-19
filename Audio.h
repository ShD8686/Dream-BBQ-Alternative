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

    // Background music (loops, stops previous track)
    void PlayMusic(const std::string& path, float volume = 0.55f);
    void StopMusic();
    void SetMusicVolume(float volume);

    // Ambient loop (e.g. fog wind, plays simultaneously with music)
    void PlayAmbient(const std::string& path, float volume = 0.45f);
    void StopAmbient();
    void SetAmbientVolume(float volume);

    // One-shot sound effects (overlapping allowed)
    void PlaySFX(const std::string& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
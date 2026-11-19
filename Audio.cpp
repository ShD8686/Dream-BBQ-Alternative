#include "Audio.h"
#include <fstream>
#include <iostream>

#define NOMINMAX
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#define NOMINMAX
#define MINIAUDIO_IMPLEMENTATION


static bool FileExists(const std::string& path)
{
    std::ifstream f(path);
    return f.good();
}

struct Audio::Impl
{
    ma_engine engine{};
    bool engineInit = false;

    ma_sound music{};
    bool musicLoaded = false;

    ma_sound ambient{};
    bool ambientLoaded = false;
};

Audio::Audio() : impl(std::make_unique<Impl>()) {}

Audio::~Audio()
{
    Shutdown();
}

bool Audio::Init()
{
    if (impl->engineInit) return true;
    if (ma_engine_init(nullptr, &impl->engine) != MA_SUCCESS)
    {
        std::cout << "Failed to initialize miniaudio engine\n";
        return false;
    }
    impl->engineInit = true;
    return true;
}

void Audio::Shutdown()
{
    if (!impl || !impl->engineInit) return;
    StopMusic();
    StopAmbient();
    ma_engine_uninit(&impl->engine);
    impl->engineInit = false;
}

void Audio::PlayMusic(const std::string& path, float volume)
{
    if (!impl->engineInit) return;
    StopMusic();
    if (!FileExists(path)) return;

    ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;
    if (ma_sound_init_from_file(&impl->engine, path.c_str(), flags, nullptr, nullptr, &impl->music) == MA_SUCCESS)
    {
        impl->musicLoaded = true;
        ma_sound_set_looping(&impl->music, MA_TRUE);
        ma_sound_set_volume(&impl->music, volume);
        ma_sound_start(&impl->music);
    }
}

void Audio::StopMusic()
{
    if (impl->musicLoaded)
    {
        ma_sound_stop(&impl->music);
        ma_sound_uninit(&impl->music);
        impl->musicLoaded = false;
    }
}

void Audio::SetMusicVolume(float volume)
{
    if (impl->musicLoaded)
        ma_sound_set_volume(&impl->music, volume);
}

void Audio::PlayAmbient(const std::string& path, float volume)
{
    if (!impl->engineInit) return;
    StopAmbient();
    if (!FileExists(path)) return;

    ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;
    if (ma_sound_init_from_file(&impl->engine, path.c_str(), flags, nullptr, nullptr, &impl->ambient) == MA_SUCCESS)
    {
        impl->ambientLoaded = true;
        ma_sound_set_looping(&impl->ambient, MA_TRUE);
        ma_sound_set_volume(&impl->ambient, volume);
        ma_sound_start(&impl->ambient);
    }
}

void Audio::StopAmbient()
{
    if (impl->ambientLoaded)
    {
        ma_sound_stop(&impl->ambient);
        ma_sound_uninit(&impl->ambient);
        impl->ambientLoaded = false;
    }
}

void Audio::SetAmbientVolume(float volume)
{
    if (impl->ambientLoaded)
        ma_sound_set_volume(&impl->ambient, volume);
}

void Audio::PlaySFX(const std::string& path)
{
    if (!impl->engineInit || !FileExists(path)) return;
    ma_engine_play_sound(&impl->engine, path.c_str(), nullptr);
}
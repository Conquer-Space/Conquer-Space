/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <thread>
#include <atomic>
#include <memory>
#include <string>

#include "engine/audio/audioasset.h"
#include "engine/audio/iaudiointerface.h"

namespace conquerspace {
namespace engine {
namespace audio {

class AudioInterface : public IAudioInterface {
 public:
    AudioInterface();
    ALCdevice* device;
    ALCcontext* context;
    ALboolean enumeration;

    void Initialize();
    void Pause(bool to_pause);
    void PauseMusic(bool to_pause);
    std::string GetAudioVersion();
    void Destruct();
    void StartWorker();
    void RequestPlayAudio();
    void SetMusicVolume(float volume);

    ~AudioInterface() {}

    std::thread worker_thread;
    std::unique_ptr<AudioAsset> LoadWav(std::ifstream &input);
    std::unique_ptr<AudioAsset> LoadOgg(std::ifstream& input);
    std::unique_ptr<AudioAsset> music = nullptr;

    std::shared_ptr<spdlog::logger> logger;

    std::atomic_bool to_quit;

    Hjson::Value playlist;

 private:
    void PrintInformation();
    void InitListener();
    void InitALContext();

    float music_volume = 0;
};
}  // namespace audio
}  // namespace engine
}  // namespace conquerspace

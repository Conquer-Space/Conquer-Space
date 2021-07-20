#pragma once

#include <string>

#include <hjson.h>

namespace conquerspace {
namespace engine {
namespace audio {
class IAudioInterface {
 public:
    virtual void Pause(bool to_pause) = 0;
    virtual void PauseMusic(bool to_pause) = 0;
    virtual std::string GetAudioVersion() = 0;
    virtual void Initialize() = 0;
    virtual void Destruct() = 0;
    virtual void StartWorker() = 0;
    virtual void RequestPlayAudio() = 0;
    virtual void SetMusicVolume(float volume) = 0;
};
}
}
}

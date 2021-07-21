/*
* Copyright 2021 Conquer Space
*/
#pragma once

namespace conquerspace {
namespace engine {
namespace audio {
class AudioAsset {
 public:
    virtual void SetGain(float gain) = 0;
    virtual void SetPitch(float pitch) = 0;
    virtual void SetLooping(bool looping) = 0;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Resume() = 0;
    virtual void Pause() = 0;
    virtual bool IsPlaying() = 0;
    virtual void Rewind() = 0;
    virtual float PlayPosition() = 0;

    /**
     * Length in seconds.
     */
    virtual float Length() = 0;

    virtual ~AudioAsset() {}

    // Get playlist
};
}  // namespace audio
}  // namespace engine
}  // namespace conquerspace

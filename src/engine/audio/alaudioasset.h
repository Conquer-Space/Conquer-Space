#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <fstream>

#include "engine/audio/audioasset.h"

namespace conquerspace::asset {
class ALAudioAsset : public AudioAsset {
 public:
    ALAudioAsset() {
        // Make audio things
        alGenBuffers(1, &buffer);
        alGenSources((ALuint)1, &source);

        alSourcef(source, AL_PITCH, 1);
        alSourcef(source, AL_GAIN, 1);

        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSource3f(source, AL_VELOCITY, 0, 0, 0);
        alSourcei(source, AL_LOOPING, AL_FALSE);
    }

    void SetGain(float gain) {
        alSourcef(source, AL_GAIN, gain);
    }

    void SetPitch(float pitch) {
        alSourcef(source, AL_PITCH, pitch);
    }

    void SetLooping(bool looping) {
        alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    }

    void Play() {
        alSourcePlay(source);
    }

    void Stop() {
        alSourceStop(source);
    }

    void Resume() {
        alSourcePlay(source);
    }

    void Pause() {
        alSourcePause(source);
    }

    void Rewind() {
        alSourceRewind(source);
    }

    bool IsPlaying() {
        ALint source_state;
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        return (source_state == AL_PLAYING);
    }

    float PlayPosition() {
        float length;
        alGetSourcef(source, AL_SEC_OFFSET, &length);
        return length;
    }

    /**
     * Length in seconds.
     */
    float Length() {
        return length;
    }

    ALuint source;
    ALuint buffer;
    float length = 0;

    ~ALAudioAsset() {
        alDeleteSources(1, &source);
        alDeleteBuffers(1, &buffer);
    }
};

std::unique_ptr<AudioAsset> LoadOgg(std::ifstream& input);
}  // namespace conquerspace::asset

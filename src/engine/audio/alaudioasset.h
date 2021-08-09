#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <fstream>

#include "engine/audio/audioasset.h"

namespace cqsp::asset {
class ALAudioAsset : public AudioAsset {
 public:
    ALAudioAsset() {
        // Make audio things
        alGenBuffers(1, &buffer);
    }

    /**
     * Length in seconds.
     */
    float Length() {
        return length;
    }

    ALuint buffer;
    float length = 0;

    ~ALAudioAsset() {
        alDeleteBuffers(1, &buffer);
    }
};

std::unique_ptr<AudioAsset> LoadOgg(std::ifstream& input);
}  // namespace cqsp::asset

/* Conquer Space
* Copyright (C) 2021 Conquer Space
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <fstream>
#include <memory>

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
std::unique_ptr<AudioAsset> LoadOgg(uint8_t* buffer, int size);
}  // namespace cqsp::asset

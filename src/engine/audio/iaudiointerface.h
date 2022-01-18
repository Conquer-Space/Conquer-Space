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

#include <hjson.h>

#include <string>

#include "engine/audio/audioasset.h"

namespace cqsp {
namespace engine {
namespace audio {
enum ChannelType {};
class IAudioInterface {
 public:
    virtual ~IAudioInterface() = default;
    virtual void Pause(bool to_pause) = 0;
    virtual void PauseMusic(bool to_pause) = 0;
    virtual std::string GetAudioVersion() = 0;
    virtual void Initialize() = 0;
    virtual void Destruct() = 0;
    virtual void StartWorker() = 0;
    virtual void RequestPlayAudio() = 0;
    virtual void SetMusicVolume(float volume) = 0;
    virtual void PlayAudioClip(cqsp::asset::AudioAsset *asset, int channel) = 0;
    virtual void SetChannelVolume(int channel, float gain) = 0;

    virtual void AddAudioClip(const std::string &key, cqsp::asset::AudioAsset *asset) = 0;
    virtual void PlayAudioClip(const std::string &key) = 0;
};
}  // namespace audio
}  // namespace engine
}  // namespace cqsp

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

#include "engine/asset.h"

namespace conquerspace {
namespace asset {
class AudioAsset : public conquerspace::asset::Asset{
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
}  // namespace asset
}  // namespace conquerspace

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
#include <spdlog/spdlog.h>

#include <atomic>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/audio/audioasset.h"
#include "engine/audio/iaudiointerface.h"

namespace cqsp {
namespace engine {
namespace audio {
struct AudioChannel {
    // Set all variables
    AudioChannel() {
        alGenSources((ALuint)1, &channel);

        alSourcef(channel, AL_PITCH, 1);
        alSourcef(channel, AL_GAIN, 1);

        alSource3f(channel, AL_POSITION, 0, 0, 0);
        alSource3f(channel, AL_VELOCITY, 0, 0, 0);
        alSourcei(channel, AL_LOOPING, AL_FALSE);
    }

    void SetGain(float gain) { alSourcef(channel, AL_GAIN, gain); }

    void SetPitch(float pitch) { alSourcef(channel, AL_PITCH, pitch); }

    void SetLooping(bool looping) { alSourcei(channel, AL_LOOPING, looping ? AL_TRUE : AL_FALSE); }

    void Play() { alSourcePlay(channel); }

    void Stop() { alSourceStop(channel); }

    void Resume() { alSourcePlay(channel); }

    void Pause() { alSourcePause(channel); }

    void Rewind() { alSourceRewind(channel); }

    bool IsPlaying() {
        ALint source_state;
        alGetSourcei(channel, AL_SOURCE_STATE, &source_state);
        return (source_state == AL_PLAYING);
    }

    float PlayPosition() {
        float length;
        alGetSourcef(channel, AL_SEC_OFFSET, &length);
        return length;
    }

    void EmptyBuffer() { alSourcei(channel, AL_BUFFER, NULL); }

    void SetBuffer(cqsp::asset::AudioAsset *buffer);
    float length = 0;

    ~AudioChannel() {
        // Halt playing
        Stop();

        EmptyBuffer();
        alDeleteSources(1, &channel);
    }

 private:
    ALuint channel;
};

class AudioInterface : public IAudioInterface {
 public:
    AudioInterface();
    ALCdevice *device;
    ALCcontext *context;
    ALboolean enumeration;

    void Initialize();
    void Pause(bool to_pause);
    void PauseMusic(bool to_pause);
    std::string GetAudioVersion();
    void Destruct();
    void StartWorker();
    void RequestPlayAudio();
    void SetMusicVolume(float volume);

    void AddAudioClip(const std::string &key, cqsp::asset::AudioAsset *asset);
    void PlayAudioClip(const std::string &key);
    void PlayAudioClip(cqsp::asset::AudioAsset *asset, int channel);
    void SetChannelVolume(int channel, float gain);
    void OnFrame() override;

    ~AudioInterface();

    std::unique_ptr<cqsp::asset::AudioAsset> LoadWav(std::ifstream &input);
    std::unique_ptr<cqsp::asset::AudioAsset> music_asset = nullptr;

    std::shared_ptr<spdlog::logger> logger;

    std::atomic_bool to_quit;

    Hjson::Value playlist;

 private:
    void PrintInformation();
    void InitListener();
    void InitALContext();
    std::map<std::string, cqsp::asset::AudioAsset *> assets;
    std::vector<std::unique_ptr<AudioChannel>> channels;

    float music_volume = 0;
};
}  // namespace audio
}  // namespace engine
}  // namespace cqsp

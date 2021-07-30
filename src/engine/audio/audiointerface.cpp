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
#include "engine/audio/audiointerface.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#include <chrono>
#include <string>

#include "engine/audio/alaudioasset.h"

using conquerspace::engine::audio::AudioInterface;
using conquerspace::asset::AudioAsset;

AudioInterface::AudioInterface() {
    logger = spdlog::stdout_color_mt("audio");
}

void AudioInterface::Initialize() {
    SPDLOG_LOGGER_INFO(logger, "Intializing OpenAL");
    InitALContext();
    PrintInformation();
    InitListener();

    // Load playlist
    std::ifstream playlist_input = std::ifstream("../data/core/music/music_list.hjson");
    Hjson::DecoderOptions decOpt;
    playlist_input >> Hjson::StreamDecoder(playlist, decOpt);
}

void AudioInterface::Pause(bool to_pause) {}

void AudioInterface::PauseMusic(bool to_pause) {}

std::string AudioInterface::GetAudioVersion() {
    return std::string();
}

void AudioInterface::Destruct() {
    to_quit = true;
    worker_thread.join();
    SPDLOG_LOGGER_INFO(logger, "Killed OpenAL");
    // Clear sources and buffers
    music.reset();
    // Kill off music
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void AudioInterface::StartWorker() {
    to_quit = false;
    // Worker thread
    worker_thread = std::thread([&]() {
        // Play music, handle playlist
        // Quickly inserted random algorithm to make linter happy.
        int selected_track = (time(0) * 0x0000BC8F) % 0x7FFFFFFF % playlist.size();
        while (!to_quit) {
            // Choose random song from thing
            selected_track++;
            selected_track %= playlist.size();
            Hjson::Value track_info = playlist[selected_track];
            std::string track_file = "../data/core/music/" + track_info["file"];
            SPDLOG_LOGGER_INFO(logger, "Loading track \'{}\'", track_info["name"]);
            auto mfile = std::ifstream(track_file, std::ios::binary);
            if (mfile.good()) {
                music = conquerspace::asset::LoadOgg(mfile);
                SPDLOG_LOGGER_INFO(logger, "Length of audio: {}", music->Length());
            } else {
                SPDLOG_LOGGER_ERROR(logger, "Failed to load audio {}", track_info["name"]);
            }

            if (music != nullptr) {
                music->SetGain(music_volume);
                music->Play();
                while (music->IsPlaying() && !to_quit) {
                    // Wait
                }
                // Stop music
                music->Stop();
                music.reset();
                SPDLOG_LOGGER_INFO(logger, "Completed track");
            }
        }
    });
}

void AudioInterface::RequestPlayAudio() {}

void AudioInterface::SetMusicVolume(float volume) {
    if (music != nullptr) {
        music->SetGain(volume);
    }
    music_volume = volume;
}

void conquerspace::engine::audio::AudioInterface::AddAudioClip(const std::string& key, AudioAsset* asset) {
    assets[key] = asset;
}

void conquerspace::engine::audio::AudioInterface::PlayAudioClip(const std::string& key) {
    if (assets.find(key) == assets.end()) {
        SPDLOG_LOGGER_WARN(logger, "Unable to find audio clip {}", key);
    } else {
        assets[key]->Rewind();
        assets[key]->Play();
    }
}

inline bool isBigEndian() {
    int a = 1;
    return !(reinterpret_cast<char*>(&a))[0];
}

inline int convertToInt(char* buffer, int len) {
    int a = 0;
    if (!isBigEndian()) {
        for (int i = 0; i < len; i++)
            (reinterpret_cast<char*>(&a))[i] = buffer[i];
    } else {
        for (int i = 0; i < len; i++)
            (reinterpret_cast<char*>(&a))[3 - i] = buffer[i];
    }
    return a;
}

inline ALenum to_al_format(int16_t channels, int16_t samples) {
    bool stereo = (channels > 1);

    switch (samples) {
    case 16:
        return (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    case 8:
        return (stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    default:
        return -1;
    }
}

std::unique_ptr<AudioAsset> AudioInterface::LoadWav(std::ifstream &in) {
    auto audio_asset = std::make_unique<conquerspace::asset::ALAudioAsset>();
    char buffer[4];
    in.read(buffer, 4);
    if (strncmp(buffer, "RIFF", 4) != 0) {
        // Not a valid file
    }
    in.read(buffer, 4);
    in.read(buffer, 4);      // WAVE
    in.read(buffer, 4);      // fmt
    in.read(buffer, 4);      // 16
    in.read(buffer, 2);      // 1
    in.read(buffer, 2);
    int channels = convertToInt(buffer, 2);
    in.read(buffer, 4);
    int samplerate = convertToInt(buffer, 4);
    in.read(buffer, 4);
    in.read(buffer, 2);
    in.read(buffer, 2);
    int frequency = convertToInt(buffer, 2);
    in.read(buffer, 4);      // data
    in.read(buffer, 4);
    int size = convertToInt(buffer, 4);
    char* data = new char[size];
    in.read(data, size);

    ALenum format = to_al_format(channels, frequency);

    // Copy buffer
    alBufferData(audio_asset->buffer, format, data, size, samplerate);
    alSourcei(audio_asset->source, AL_BUFFER, audio_asset->buffer);

    delete[] data;
    return audio_asset;
}

void AudioInterface::PrintInformation() {
    // OpenAL information
    SPDLOG_LOGGER_INFO(logger, "OpenAL version: {}", alGetString(AL_VERSION));
    SPDLOG_LOGGER_INFO(logger, "OpenAL renderer: {}", alGetString(AL_RENDERER));
    SPDLOG_LOGGER_INFO(logger, "OpenAL vendor: {}", alGetString(AL_VENDOR));
}

void AudioInterface::InitListener() {
    // Set listener
    alListener3f(AL_POSITION, 0, 0, -0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, listenerOri);
}

void AudioInterface::InitALContext() {
    // Init devices
    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_FALSE) {
        SPDLOG_LOGGER_ERROR(logger, "Enumeration extension not available");
    }

    const ALCchar* defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    device = alcOpenDevice(defaultDeviceName);
    if (!device) {
        SPDLOG_LOGGER_ERROR(logger, "Unable to open default device");
    }

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context)) {
        SPDLOG_LOGGER_ERROR(logger, "Failed to make default context");
    }
}

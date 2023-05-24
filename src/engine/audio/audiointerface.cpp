/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <chrono>
#include <future>
#include <random>
#include <string>

#include "common/util/logging.h"
#include "common/util/paths.h"
#include "engine/audio/alaudioasset.h"

using cqsp::asset::AudioAsset;
using cqsp::engine::audio::AudioInterface;

AudioInterface::AudioInterface() { logger = cqsp::common::util::make_logger("audio"); }

void AudioInterface::Initialize() {
    SPDLOG_LOGGER_INFO(logger, "Intializing OpenAL");
    InitALContext();
    PrintInformation();
    InitListener();

    // Load playlist
    // TODO(EhWhoAmI): Load playlist from all mod folders
    std::string music_playlist_path = cqsp::common::util::GetCqspDataPath() + "/core/music/music_list.hjson";
    std::ifstream playlist_input = std::ifstream(music_playlist_path);
    if (playlist_input.good()) {
        Hjson::DecoderOptions decOpt;
        playlist_input >> Hjson::StreamDecoder(playlist, decOpt);
    }
    SPDLOG_LOGGER_INFO(logger, "Playlist size: {}", playlist.size());

    channels.push_back(std::make_unique<AudioChannel>());
    channels.push_back(std::make_unique<AudioChannel>());
}

void AudioInterface::Pause(bool to_pause) {}

void AudioInterface::PauseMusic(bool to_pause) {}

std::string AudioInterface::GetAudioVersion() { return std::string(); }

void AudioInterface::Destruct() {
    to_quit = true;
    SPDLOG_LOGGER_INFO(logger, "Killing OpenAL");
    // Clear sources and buffers
    for (int i = 0; i < channels.size(); i++) channels[i].reset();
    music_asset.reset();
}

void AudioInterface::StartWorker() {
    to_quit = false;
    // Worker thread
}

void AudioInterface::RequestPlayAudio() {}

void AudioInterface::SetMusicVolume(float volume) {
    if (music_asset != nullptr) {
        channels[MUSIC_CHANNEL]->SetGain(volume);
    }
    music_volume = volume;
}

void cqsp::engine::audio::AudioInterface::AddAudioClip(const std::string& key, AudioAsset* asset) {
    assets[key] = asset;
}

void cqsp::engine::audio::AudioInterface::PlayAudioClip(const std::string& key) {
    if (assets.find(key) == assets.end()) {
        SPDLOG_LOGGER_WARN(logger, "Unable to find audio clip {}", key);
    } else {
        channels[UI_CHANNEL]->Stop();
        channels[UI_CHANNEL]->SetBuffer(assets[key]);
        channels[UI_CHANNEL]->Rewind();
        channels[UI_CHANNEL]->Play();
    }
}

void cqsp::engine::audio::AudioInterface::PlayAudioClip(cqsp::asset::AudioAsset* asset, int channel) {}

void cqsp::engine::audio::AudioInterface::SetChannelVolume(int channel, float gain) {
    channels[channel]->SetGain(gain);
}

void cqsp::engine::audio::AudioInterface::OnFrame() {
    bool to_quit = false;
    if (channels[MUSIC_CHANNEL]->IsPlaying() && !to_quit) {
        return;
    }

    if (channels[MUSIC_CHANNEL]->IsStopped()) {
        // Check for device error?
        int error = alcGetError(device);
        ALCint status;
        //alcGetIntegerv(device, ALC_, 1, &status);
        if (error != ALC_NO_ERROR) {
            SPDLOG_LOGGER_INFO(logger, "Error {}", error);
        }
    }

    if (music_asset != nullptr) {
        SPDLOG_LOGGER_INFO(logger, "Completed track");
        channels[MUSIC_CHANNEL]->Stop();
        channels[MUSIC_CHANNEL]->EmptyBuffer();
        music_asset.reset();
    }

    if (audio_future.valid()) {
        if (audio_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            music_asset = audio_future.get();
        }
    } else {
        audio_future = std::async(&AudioInterface::LoadNextFile, &*this);
    }
    // Get the thing after it loads
    if (music_asset != nullptr) {
        // Set the music_asset
        channels[MUSIC_CHANNEL]->SetBuffer(music_asset.get());
        channels[MUSIC_CHANNEL]->SetGain(music_volume);
        channels[MUSIC_CHANNEL]->Play();
    }
}

cqsp::engine::audio::AudioInterface::~AudioInterface() {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

namespace {
inline bool isBigEndian() {
    int a = 1;
    return !(reinterpret_cast<char*>(&a))[0];
}

inline int convertToInt(char* buffer, int len) {
    int a = 0;
    if (!isBigEndian()) {
        for (int i = 0; i < len; i++) (reinterpret_cast<char*>(&a))[i] = buffer[i];
    } else {
        for (int i = 0; i < len; i++) (reinterpret_cast<char*>(&a))[3 - i] = buffer[i];
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
}  // namespace

std::unique_ptr<AudioAsset> AudioInterface::LoadWav(std::ifstream& in) {
    auto audio_asset = std::make_unique<cqsp::asset::ALAudioAsset>();
    char buffer[4];
    in.read(buffer, 4);
    if (strncmp(buffer, "RIFF", 4) != 0) {
        // Not a valid file
    }
    in.read(buffer, 4);  // Flawfinder: ignore
    // Flawfinder: ignore
    in.read(buffer, 4);  // WAVE
    // Flawfinder: ignore
    in.read(buffer, 4);  // fmt
    // Flawfinder: ignore
    in.read(buffer, 4);  // 16
    // Flawfinder: ignore
    in.read(buffer, 2);  // 1
    in.read(buffer, 2);  // Flawfinder: ignore
    int channels = convertToInt(buffer, 2);
    in.read(buffer, 4);  // Flawfinder: ignore
    int samplerate = convertToInt(buffer, 4);
    in.read(buffer, 4);  // Flawfinder: ignore
    in.read(buffer, 2);  // Flawfinder: ignore
    in.read(buffer, 2);  // Flawfinder: ignore
    int frequency = convertToInt(buffer, 2);
    // Flawfinder: ignore
    in.read(buffer, 4);  // data
    in.read(buffer, 4);  // Flawfinder: ignore
    int size = convertToInt(buffer, 4);
    char* data = new char[size];
    in.read(data, size);  // Flawfinder: ignore

    ALenum format = to_al_format(channels, frequency);

    // Copy buffer
    alBufferData(audio_asset->buffer, format, data, size, samplerate);
    //alSourcei(audio_asset->source, AL_BUFFER, audio_asset->buffer);

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
    ALfloat listenerOri[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
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

std::unique_ptr<AudioAsset> cqsp::engine::audio::AudioInterface::LoadNextFile() {
    // Choose random song from playlist
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(
        0, playlist.size() - 1);  // distribution in range [1, 6]

    int selected_track = dist6(rng);

    Hjson::Value track_info = playlist[selected_track];
    std::string track_file = cqsp::common::util::GetCqspDataPath() + "/core/music/" + track_info["file"];
    SPDLOG_LOGGER_INFO(logger, "Loading track \'{}\'", track_info["name"]);
    auto mfile = std::ifstream(track_file, std::ios::binary);
    if (mfile.good()) {
        return cqsp::asset::LoadOgg(mfile);
        SPDLOG_LOGGER_INFO(logger, "Length of audio: {}", music_asset->Length());
    } else {
        SPDLOG_LOGGER_ERROR(logger, "Failed to load audio {}", track_info["name"]);
        return nullptr;
    }
}

void cqsp::engine::audio::AudioChannel::SetBuffer(cqsp::asset::AudioAsset* buffer) {
    alSourcei(channel, AL_BUFFER, dynamic_cast<cqsp::asset::ALAudioAsset*>(buffer)->buffer);
}

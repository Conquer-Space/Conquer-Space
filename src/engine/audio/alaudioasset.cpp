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
#include "engine/audio/alaudioasset.h"

// NOLINTBEGIN
#include <stb_vorbis.h>
// NOLINTEND

#include <utility>

namespace cqsp::asset {
std::unique_ptr<AudioAsset> LoadOgg(std::ifstream& input) {
    // Read file
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    char* data = new char[size];
    input.read(data, size);

    std::unique_ptr<ALAudioAsset> audio_asset = std::make_unique<ALAudioAsset>();
    int16_t* buffer = nullptr;
    int channels = 1;
    int sample_rate = 0;
    int num_samples = stb_vorbis_decode_memory((unsigned char*)(data),
                                static_cast<int>(size), &channels, &sample_rate, &buffer);

    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(audio_asset->buffer, format, buffer,
                                num_samples * channels * static_cast<int>(sizeof(int16_t)), sample_rate);
    audio_asset->length = static_cast<float>(num_samples) / static_cast<float>(sample_rate);

    // Free memory
    delete[] data;
    free(buffer);
    return audio_asset;
}

std::unique_ptr<AudioAsset> LoadOgg(uint8_t* buffer, int size) {
    std::unique_ptr<ALAudioAsset> audio_asset = std::make_unique<ALAudioAsset>();
    int16_t* output = nullptr;
    int channels = 1;
    int sample_rate = 0;
    int num_samples = stb_vorbis_decode_memory((unsigned char*)(buffer),
                                static_cast<int>(size), &channels, &sample_rate, &output);
    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(audio_asset->buffer, format, output,
                    num_samples * channels * static_cast<int>(sizeof(int16_t)), sample_rate);
    audio_asset->length = static_cast<float>(num_samples) / static_cast<float>(sample_rate);

    // Free memory
    free(output);
    return std::move(audio_asset);
}
} // namespace cqsp::asset

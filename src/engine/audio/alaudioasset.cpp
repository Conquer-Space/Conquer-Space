#include "engine/audio/alaudioasset.h"

#include <stb_vorbis.h>

#include <spdlog/spdlog.h>
namespace conquerspace::asset {
std::unique_ptr<AudioAsset> LoadOgg(std::ifstream& input) {
    // Read file
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    char* data = new char[size];
    input.read(data, size);

    std::unique_ptr<ALAudioAsset> audio_asset = std::make_unique<ALAudioAsset>();
    int16* buffer;
    int channels;
    int sample_rate;
    int numSamples = stb_vorbis_decode_memory((unsigned char*)(data),
                                size, &channels, &sample_rate, &buffer);

    ALenum format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(audio_asset->buffer, format, buffer,
                                numSamples * channels * sizeof(int16), sample_rate);
    ALenum error;
    if ((error = alGetError()) != ALC_NO_ERROR) {
    }
    audio_asset->length = static_cast<float>(numSamples) / static_cast<float>(sample_rate);

    // Free memory
    delete[] data;
    free(buffer);
    return audio_asset;
}
} // namespace conquerspace::asset

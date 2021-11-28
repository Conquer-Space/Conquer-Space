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

#include <vector>

#include "engine/asset/asset.h"

namespace cqsp {
namespace asset {

struct TextureLoadingOptions {
    /*
    * True is nearest, false is linear
    */
    bool mag_filter = false;
};

class Texture : public Asset {
 public:
    int width;
    int height;
    unsigned int id;
    int texture_type;

    void Delete() override;
    Texture();
    ~Texture();

    AssetType GetAssetType() { return AssetType::TEXTURE; }
};

unsigned int LoadTexture(const unsigned char* data,
                    int components,
                    int width,
                    int height,
                    TextureLoadingOptions& options);

void LoadTexture(Texture &texture, const unsigned char* data,
                    int components,
                    int width,
                    int height,
                    TextureLoadingOptions& options);

void LoadCubemap(Texture &texture, std::vector<unsigned char*>& data,
                    int components,
                    int width,
                    int height,
                    TextureLoadingOptions& options);

void SaveImage(const char* path, int width, int height,
               int components, const unsigned char* data);
}  // namespace asset
}  // namespace cqsp

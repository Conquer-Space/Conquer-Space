/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "engine/asset.h"

namespace conquerspace {
namespace asset {

struct TextureLoadingOptions {
    /*
    * True is nearest, false is linear
    */
    bool mag_filter = false;
};

class Texture : public Asset {
 public:
    int width, height;
    int id;
};

unsigned int LoadTexture(unsigned char*& data,
                    int components,
                    int width,
                    int height,
                    TextureLoadingOptions& options);

void LoadTexture(Texture &texture, unsigned char*& data,
                    int components,
                    int width,
                    int height,
                    TextureLoadingOptions& options);
}  // namespace asset
}  // namespace conquerspace

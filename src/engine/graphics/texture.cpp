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
#include "engine/graphics/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <glad/glad.h>

unsigned int cqsp::asset::CreateTexture(unsigned char*& data, int width,
                                        int height,
                                        int components,
                                        TextureLoadingOptions& options) {
    unsigned int texid;
    glGenTextures(1, &texid);
    GLenum format;
    if (components == 1)
        format = GL_RED;
    else if (components == 3)
        format = GL_RGB;
    else if (components == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, texid);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                    format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);

    if (options.mag_filter) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return texid;
}

void cqsp::asset::CreateTexture(Texture& texture, unsigned char*& data, int width,
                              int height, int components, TextureLoadingOptions& options) {
    texture.id = CreateTexture(data, width, height, components, options);
    texture.width = width;
    texture.height = height;
    // It's a 2d texture, so
    texture.texture_type = GL_TEXTURE_2D;
}

void cqsp::asset::LoadCubemapData(Texture &texture,
                                std::vector<unsigned char*>& faces,
                                int width,
                                int height,
                                int components,
                                TextureLoadingOptions& options) {
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.id);

    GLenum format;
    if (components == 1)
        format = GL_RED;
    else if (components == 3)
        format = GL_RGB;
    else if (components == 4)
        format = GL_RGBA;

    for (unsigned int i = 0; i < faces.size(); i++) {
        if (faces[i]) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, faces[i]);
            stbi_image_free(faces[i]);
        } else {
            stbi_image_free(faces[i]);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    texture.width = width;
    texture.height = height;
    texture.texture_type = GL_TEXTURE_CUBE_MAP;
}

bool cqsp::asset::SaveImage(const char* path, int width, int height,
                            int components, const unsigned char* data, bool flip) {
    if (flip) {
        // Flip because everybody can't agree on having one origin for the window.
        stbi_flip_vertically_on_write(1);
    }
    return stbi_write_png(path, width, height, components, data, width * components);
}

cqsp::asset::Texture::Texture() : width(-1), height(-1), id(0), texture_type(-1) {
}

cqsp::asset::Texture::~Texture() {
    // Delete textures
    // If it's a null texture, then no need to destroy it
    if (texture_type != -1) {
        glDeleteTextures(1, &id);
    }
}

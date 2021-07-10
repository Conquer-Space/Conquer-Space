/*
* Copyright 2021 Conquer Space
*/
#include "engine/renderer/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

unsigned int conquerspace::asset::LoadTexture(unsigned char*& data,
                                        int components,
                                        int width,
                                        int height,
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

    stbi_image_free(data);
    return texid;
}

void conquerspace::asset::LoadTexture(Texture& texture, unsigned char*& data,
                                      int components, int width, int height,
                                      TextureLoadingOptions& options) {
    texture.id = LoadTexture(data, components, width, height, options);
    texture.width = width;
    texture.height = height;
}

void conquerspace::asset::LoadCubemap(Texture &texture, std::vector<unsigned char*>& faces,
                    int components,
                    int width,
                    int height,
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

conquerspace::asset::Texture::Texture() { texture_type = GL_TEXTURE_2D; }

conquerspace::asset::Texture::~Texture() {
    // Delete textures
    glDeleteTextures(1, &id);
}

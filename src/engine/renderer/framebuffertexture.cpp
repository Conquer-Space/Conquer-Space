/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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
#include "engine/renderer/framebuffertexture.h"

#include <glad/glad.h>

#include "engine/enginelogger.h"
#include "engine/glfwdebug.h"
#include "engine/renderer/framebuffer.h"

namespace cqsp::engine {
FramebufferTexture::FramebufferTexture() {
    id = -1;
    texture_type = GL_TEXTURE_2D;
}
void FramebufferTexture::InitTexture(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    this->width = width;
    this->height = height;
    GenerateFrameBuffer(framebuffer);

    // create a multisampled color attachment texture
    glGenTextures(1, &mscat);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mscat);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mscat, 0);

    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
    GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
        ENGINE_LOG_ERROR("Incomplete framebuffer: 0x{:x} ({})!", framebuffer_status,
                         FramebufferStatusToString(framebuffer_status));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferTexture::BeginDraw() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, width, height);
    LogGlError("Error when binding framebuffer!");
}

void FramebufferTexture::EndDraw() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void FramebufferTexture::Resolve() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferTexture::FreeTextures() {
    if (framebuffer != 0) glDeleteFramebuffers(1, &framebuffer);
    if (intermediateFBO != 0) glDeleteFramebuffers(1, &intermediateFBO);
    if (mscat != 0) glDeleteTextures(1, &mscat);
    if (rbo != 0) glDeleteRenderbuffers(1, &rbo);
    framebuffer = 0;
    intermediateFBO = 0;
    mscat = 0;
    rbo = 0;
}

FramebufferTexture::~FramebufferTexture() { FreeTextures(); }
}  // namespace cqsp::engine

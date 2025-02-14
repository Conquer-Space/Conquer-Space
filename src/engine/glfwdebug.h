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
#pragma once

#include <glad/glad.h>

#include "engine/enginelogger.h"

namespace cqsp::engine {
const inline char* ParseType(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            return ("Error");
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return ("Deprecated Behaviour");
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return ("Undefined Behaviour");
        case GL_DEBUG_TYPE_PORTABILITY:
            return ("Portability");
        case GL_DEBUG_TYPE_PERFORMANCE:
            return ("Performance");
        case GL_DEBUG_TYPE_MARKER:
            return ("Marker");
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return ("Push Group");
        case GL_DEBUG_TYPE_POP_GROUP:
            return ("Pop Group");
        case GL_DEBUG_TYPE_OTHER:
        default:
            return ("Other");
    }
}

const inline char* ParseSeverity(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            return ("high");
        case GL_DEBUG_SEVERITY_MEDIUM:
            return ("medium");
        case GL_DEBUG_SEVERITY_LOW:
            return ("low");
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        default:
            return ("notification");
    }
}

const inline char* ParseSource(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            return ("API");
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return ("Window System");
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return ("Shader Compiler");
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return ("Third Party");
        case GL_DEBUG_SOURCE_APPLICATION:
            return ("Application");
        case GL_DEBUG_SOURCE_OTHER:
        default:
            return ("Other");
    }
}

void inline APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                                   const char* message, const void* userParam) {
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;  // ignore these non-significant error codes

    ENGINE_LOG_INFO("{} message from {} ({}:{}): {}", ParseType(type), ParseSource(source), ParseSeverity(severity), id,
                    message);
}
}  // namespace cqsp::engine

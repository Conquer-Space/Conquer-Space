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
#include "core/util/color.h"

#include <sstream>

namespace cqsp::util {
std::tuple<int, int, int> HexToRgb(const std::string& str) {
    if (str.empty()) {
        return std::make_tuple(0, 0, 0);
    }
    std::stringstream ss;
    if (str.at(0) == '#') {
        // Then we substring it
        ss << std::hex << str.substr(1, std::string::npos).c_str();
    } else {
        ss << std::hex << str.c_str();
    }
    // If it starts with a # then we shouldn't parse it?

    unsigned int x;
    ss >> x;
    // Then we convert

    int r = (x & 0xFF0000) >> 16;
    int g = (x & 0x00FF00) >> 8;
    int b = (x & 0x0000FF);
    return std::make_tuple(r, g, b);
}

namespace {
float hue2rgb(float p, float q, float t) {
    if (t < 0) {
        t += 1;
    }
    if (t > 1) {
        t -= 1;
    }
    if (t < 1. / 6) {
        return p + (q - p) * 6 * t;
    }
    if (t < 1. / 2) {
        return q;
    }
    if (t < 2. / 3) {
        return p + (q - p) * (2. / 3 - t) * 6;
    }
    return p;
}
}  // namespace

glm::vec3 toHSL(const glm::vec3 rgb) {
    const float X_max = std::max(rgb.r, std::max(rgb.g, rgb.b));
    const float V = X_max;
    const float X_min = std::min(rgb.r, std::min(rgb.g, rgb.b));
    const float C = X_max - X_min;
    const float L = (X_max + X_min) / 2;
    float H = 0;
    if (C == 0) {
        H = 0;
    } else if (V == rgb.r) {
        H = (rgb.g - rgb.b) / C + (rgb.g < rgb.b ? 6 : 0);
    } else if (V == rgb.g) {
        H = (rgb.b - rgb.r) / C + 2;
    } else if (V == rgb.b) {
        H = (rgb.r - rgb.b) / C + 4;
    }
    H /= 6;
    float S = 0;
    if (L != 0.f && L != 1.f) {
        S = 2 * (V - L) / (1 - std::abs(2 * L - 1));
    }
    return glm::vec3(H, S, L);
}

glm::vec3 toRGB(const glm::vec3 hsl) {
    glm::vec3 result;

    if (0 == hsl.y) {
        result.r = result.g = result.b = hsl.z;  // achromatic
    } else {
        float q = hsl.z < 0.5 ? hsl.z * (1 + hsl.y) : hsl.z + hsl.y - hsl.z * hsl.y;
        float p = 2 * hsl.z - q;
        result.r = hue2rgb(p, q, hsl.x + 1. / 3);
        result.g = hue2rgb(p, q, hsl.x);
        result.b = hue2rgb(p, q, hsl.x - 1. / 3);
    }

    return result;
}
}  // namespace cqsp::util

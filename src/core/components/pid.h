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

namespace cqsp::core::components {
struct PIDConfig {
    double Kp;
    double Ki;
    double Kd;

    PIDConfig(double Kp, double Ki, double Kd) : Kp(Kp), Ki(Ki), Kd(Kd) {}
};

struct PID {
    PID() : proportional(0), integral(0), derivative(0) {}
    double proportional;
    double integral;
    double derivative;

    void Update(double value);
    double GetValue(const PIDConfig& conf);
};
}  // namespace cqsp::core::components

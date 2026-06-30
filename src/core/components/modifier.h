/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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

#include <fmt/format.h>

#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include <entt/entity/entity.hpp>

namespace cqsp::core::components {
// Lowk hate this method
enum class ModifierTarget : uint16_t { ExpertiseGain };

struct Modifier {
    // This should be applied to something else?
    double amount;
    // What we should modify and stuff like that?
    ModifierTarget target;
};

// A double-like value with a cached breakdown: base + sum = value.
// Call recalculate() after modifying the modifications vector.
// Arithmetic operators keep all three fields in sync without re-summing the vector.
struct ModifiableValue {
    double base = 0.0;   // unmodified starting value
    double sum = 1.0;    // cached sum of all modification amounts
    double value = 0.0;  // base * sum

    std::vector<std::pair<entt::entity, double>> modifiers;

    // Call after adding/removing entries in modifications.
    void recalculate() {
        sum = std::accumulate(modifiers.begin(), modifiers.end(), 1.0,
                              [](double acc, const auto& m) { return acc + m.second; });
        value = base * sum;
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator double() const { return value; }

    ModifiableValue() = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    ModifiableValue(double v) : base(v), value(v) {}

    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator=(T v) {
        base = static_cast<double>(v);
        value = base * sum;
        return *this;
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator+=(T v) {
        base += static_cast<double>(v);
        value = base * sum;
        return *this;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator-=(T v) {
        base -= static_cast<double>(v);
        value = base * sum;
        return *this;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator*=(T v) {
        base *= static_cast<double>(v);
        value = base * sum;
        return *this;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator/=(T v) {
        base /= static_cast<double>(v);
        value = base * sum;
        return *this;
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    bool operator==(T v) const {
        return value == static_cast<double>(v);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    bool operator!=(T v) const {
        return value != static_cast<double>(v);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    bool operator<(T v) const {
        return value < static_cast<double>(v);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    bool operator>(T v) const {
        return value > static_cast<double>(v);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    bool operator<=(T v) const {
        return value <= static_cast<double>(v);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    bool operator>=(T v) const {
        return value >= static_cast<double>(v);
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator+(ModifiableValue lhs, T rhs) {
        return lhs.value + static_cast<double>(rhs);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator+(T lhs, ModifiableValue rhs) {
        return static_cast<double>(lhs) + rhs.value;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator-(ModifiableValue lhs, T rhs) {
        return lhs.value - static_cast<double>(rhs);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator-(T lhs, ModifiableValue rhs) {
        return static_cast<double>(lhs) - rhs.value;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator*(ModifiableValue lhs, T rhs) {
        return lhs.value * static_cast<double>(rhs);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator*(T lhs, ModifiableValue rhs) {
        return static_cast<double>(lhs) * rhs.value;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator/(ModifiableValue lhs, T rhs) {
        return lhs.value / static_cast<double>(rhs);
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    friend double operator/(T lhs, ModifiableValue rhs) {
        return static_cast<double>(lhs) / rhs.value;
    }
};
}  // namespace cqsp::core::components

template <>
struct fmt::formatter<cqsp::core::components::ModifiableValue> : fmt::formatter<double> {
    template <typename FormatContext>
    auto format(const cqsp::core::components::ModifiableValue& v, FormatContext& ctx) const {
        return fmt::formatter<double>::format(static_cast<double>(v), ctx);
    }
};

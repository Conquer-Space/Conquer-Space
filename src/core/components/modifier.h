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
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <entt/entity/entity.hpp>

// Add a new target here — the enum, to-string, and from-string are all derived from this list.
// X(EnumName, "string_key")
#define CQSP_MODIFIER_TARGET_LIST      \
    X(ExpertiseGain, "expertise_gain") \
    X(MaxExpertise, "max_expertise")

namespace cqsp::core::components {
enum class ModifierTarget : uint16_t {
#define X(name, str) name,
    CQSP_MODIFIER_TARGET_LIST
#undef X
};

std::string_view ModifierTargetToString(ModifierTarget t);
std::optional<ModifierTarget> ModifierTargetFromString(std::string_view s);

struct Modifier {
    double amount;
    ModifierTarget target;
};

enum class ModifierOp { Additive, Multiplicative };

/**
 * Modifier value for additive or multiplicative values
 * Call recalculate() after adding a modifier
 */
template <ModifierOp Op = ModifierOp::Multiplicative>
struct ModifiableValue {
    static constexpr double identity = (Op == ModifierOp::Additive) ? 0.0 : 1.0;

    double base = 0.0;
    double sum = identity;
    double value = 0.0;

    std::vector<std::pair<entt::entity, double>> modifiers;

    void recalculate() {
        sum = std::accumulate(modifiers.begin(), modifiers.end(), identity,
                              [](double acc, const auto& m) { return acc + m.second; });
        if constexpr (Op == ModifierOp::Additive) {
            value = base + sum;
        } else {
            value = base * sum;
        }
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
        if constexpr (Op == ModifierOp::Additive) {
            value = base + sum;
        } else {
            value = base * sum;
        }
        return *this;
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator+=(T v) {
        base += static_cast<double>(v);
        value = _combine();
        return *this;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator-=(T v) {
        base -= static_cast<double>(v);
        value = _combine();
        return *this;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator*=(T v) {
        base *= static_cast<double>(v);
        value = _combine();
        return *this;
    }
    template <typename T>
        requires std::is_arithmetic_v<T>
    ModifiableValue& operator/=(T v) {
        base /= static_cast<double>(v);
        value = _combine();
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

 private:
    constexpr double _combine() const {
        if constexpr (Op == ModifierOp::Additive) {
            return base + sum;
        } else {
            return base * sum;
        }
    }
};

typedef ModifiableValue<ModifierOp::Additive> AdditiveModifier;
typedef ModifiableValue<ModifierOp::Multiplicative> MultiplicativeModifier;

}  // namespace cqsp::core::components

template <cqsp::core::components::ModifierOp Op>
struct fmt::formatter<cqsp::core::components::ModifiableValue<Op>> : fmt::formatter<double> {
    template <typename FormatContext>
    auto format(const cqsp::core::components::ModifiableValue<Op>& v, FormatContext& ctx) const {
        return fmt::formatter<double>::format(static_cast<double>(v), ctx);
    }
};

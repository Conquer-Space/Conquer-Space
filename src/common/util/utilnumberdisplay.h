/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <fmt/format.h>

#include <cmath>
#include <string>

namespace conquerspace {
namespace util {
inline std::string LongToHumanString(int64_t l) {
    if (abs(l) < 1000) {
        return fmt::format("{}", l);
    }
    static const std::string numbers[] = {
        "thousand",    "million",    "billion",    "trillion", "quadrillion",
        "quintillion", "sextillion", "septillion", "octillion"
    };
    int exponent = static_cast<int>(log10(abs(l)) / 3);

    // Now get the number
    double d = static_cast<double>(l) / pow(10, exponent * 3);

    // Round this to two decimal points
    const int precision = 100;
    d = round(d * precision) / precision;

    std::string s = fmt::format("{} {}", d, numbers[exponent - 1]);

    return s;
}
}  // namespace util
}  // namespace conquerspace

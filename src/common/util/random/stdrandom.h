/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <random>

#include "common/util/random/random.h"

namespace conquerspace::common::util {
class StdRandom : public IRandom {
   public:
    StdRandom(int _seed) : IRandom(_seed), random_gen(_seed){};

    int GetRandomInt(int min, int max) {
        std::uniform_int_distribution<> dist(min, max);
        return dist(random_gen);
    }

    int GetRandomNormal(double mean, double sd) {
        std::normal_distribution<> norm{mean, sd};
        return static_cast<int>(round(norm(random_gen)));
    }

 private:
    std::mt19937 random_gen;
};
}  // namespace conquerspace::common::util

/*
 * Copyright 2021 Conquer Space
 */
#pragma once

namespace conquerspace::common::util {
class IRandom {
 public:
    IRandom(int _seed) : seed(_seed) {};
    virtual int GetRandomInt(int, int) = 0;
    virtual int GetRandomNormal(double, double) = 0;
 protected:
    int seed;
};
}

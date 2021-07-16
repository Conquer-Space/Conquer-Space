/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <hjson.h>

#include <string>
#include <vector>

namespace conquerspace {
namespace asset {
class Asset {
 public:
    // Virtual destructor to make class virtual
    virtual ~Asset(){}
};

class TextAsset : public Asset {
 public:
    std::string data;
};

class TextDirectoryAsset : public Asset {
 public:
    std::vector<std::string> data;
};

class HjsonAsset : public Asset {
 public:
    Hjson::Value data;
};
}  // namespace asset
}  // namespace conquerspace

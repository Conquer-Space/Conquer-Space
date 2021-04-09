/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <hjson.h>
#include <istream>
#include <string>

namespace conquerspace {
namespace client {
class ProgramOptions {
 public:
     void LoadOptions(std::istream& stream);

     Hjson::Value& GetOptions() { return options; }

     const Hjson::Value operator[](const std::string& val) const { return options[val]; }
     const Hjson::Value operator[](const char* val) const { return options[val]; }
     const Hjson::Value operator[](char* val) const { return options[val]; }
     const Hjson::Value& operator[](int val) { return options[val]; }

     void LoadDefaultOptions();

     void CommandLineOptions(int argc, char* argv[]);
 private:
     Hjson::Value options;
};
}  // namespace client
}  // namespace conquerspace

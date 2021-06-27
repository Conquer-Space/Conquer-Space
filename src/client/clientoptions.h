/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <hjson.h>
#include <istream>
#include <fstream>
#include <string>

namespace conquerspace {
namespace client {
class ProgramOptions {
 public:
     void LoadOptions(std::istream& stream);
     void WriteOptions(std::ostream& stream);

     void LoadDefaultOptions();

     Hjson::Value& GetOptions() { return options; }

     void CommandLineOptions(int argc, char* argv[]);

     Hjson::Value GetDefaultOptions();

     std::string GetDefaultLocation() { return "settings.hjson"; }

 private:
     Hjson::Value options;
};
}  // namespace client
}  // namespace conquerspace

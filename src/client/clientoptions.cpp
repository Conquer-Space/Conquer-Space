/*
 * Copyright 2021 Conquer Space
 */
#include "client/clientoptions.h"

#include <boost/program_options.hpp>

void conquerspace::client::ProgramOptions::LoadOptions(std::istream& inputstream) {
    Hjson::DecoderOptions decOpt;
    inputstream >> Hjson::StreamDecoder(options, decOpt);
    options = Hjson::Merge(GetDefaultOptions(), options);
}

void conquerspace::client::ProgramOptions::WriteOptions(std::ostream& output_stream) {
    Hjson::EncoderOptions decOpt;
    output_stream << Hjson::StreamEncoder(options, decOpt);
}

void conquerspace::client::ProgramOptions::LoadDefaultOptions() {
    options = Hjson::Merge(GetDefaultOptions(), options);
}

void conquerspace::client::ProgramOptions::CommandLineOptions(int argc, char* argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()("help", "produce help message");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
}

Hjson::Value conquerspace::client::ProgramOptions::GetDefaultOptions() {
    Hjson::Value default_options;
    default_options["window"]["width"] = 1280;
    default_options["window"]["height"] = 720;
    default_options["full_screen"] = false;
    default_options["icon"] = "icon.png";
    return default_options;
}

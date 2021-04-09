/*
 * Copyright 2021 Conquer Space
 */
#include "client/clientoptions.h"

#include <boost/program_options.hpp>

void conquerspace::client::ProgramOptions::LoadOptions(std::istream& inputstream) {
    Hjson::DecoderOptions decOpt;
    inputstream >> Hjson::StreamDecoder(options, decOpt);
}

void conquerspace::client::ProgramOptions::LoadDefaultOptions() {
    options["window"]["width"] = 1280;
    options["window"]["height"] = 720;
}

void conquerspace::client::ProgramOptions::CommandLineOptions(int argc,
                                                              char* argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()("help", "produce help message")(
        "compression", po::value<int>(), "set compression level");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
}

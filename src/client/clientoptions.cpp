/*
 * Copyright 2021 Conquer Space
 */
#include "client/clientoptions.h"

void conquerspace::client::ClientOptions::LoadOptions(std::istream& inputstream) {
    Hjson::DecoderOptions decOpt;
    inputstream >> Hjson::StreamDecoder(options, decOpt);
    options = Hjson::Merge(GetDefaultOptions(), options);
}

void conquerspace::client::ClientOptions::WriteOptions(std::ostream& output_stream) {
    Hjson::EncoderOptions decOpt;
    output_stream << Hjson::StreamEncoder(options, decOpt);
}

void conquerspace::client::ClientOptions::LoadDefaultOptions() {
    options = Hjson::Merge(GetDefaultOptions(), options);
}

Hjson::Value conquerspace::client::ClientOptions::GetDefaultOptions() {
    Hjson::Value default_options;
    default_options["window"]["width"] = 1280;
    default_options["window"]["height"] = 720;
    default_options["full_screen"] = false;
    default_options["icon"] = "icon.png";
    return default_options;
}

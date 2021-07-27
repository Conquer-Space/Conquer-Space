/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "engine/clientoptions.h"

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
    default_options["audio"]["music"] = 1.0f;
    return default_options;
}

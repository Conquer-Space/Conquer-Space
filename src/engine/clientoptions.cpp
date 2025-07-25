/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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

#include "common/util/paths.h"

namespace cqsp::client {
void ClientOptions::LoadOptions(std::istream& inputstream) {
    Hjson::DecoderOptions decOpt;
    inputstream >> Hjson::StreamDecoder(options, decOpt);
    options = Hjson::Merge(GetDefaultOptions(), options);
}

void ClientOptions::WriteOptions(std::ostream& output_stream) {
    Hjson::EncoderOptions decOpt;
    output_stream << Hjson::StreamEncoder(options, decOpt);
}

void ClientOptions::WriteOptions() {
    std::ofstream config_path(GetDefaultLocation(), std::ios::trunc);
    WriteOptions(config_path);
}

void ClientOptions::LoadDefaultOptions() { options = Hjson::Merge(GetDefaultOptions(), options); }

Hjson::Value ClientOptions::GetDefaultOptions() {
    Hjson::Value default_options;
    default_options["window"]["width"] = 1280;
    default_options["window"]["height"] = 720;
    default_options["window"]["decorated"] = true;
    default_options["full_screen"] = false;
    default_options["icon"] = "icon.png";
    default_options["audio"]["music"] = 1.0f;
    default_options["audio"]["ui"] = 0.80f;
    default_options["splashscreens"] = "../data/core/gui/splashscreens";
    default_options["samples"] = 4;
    return default_options;
}

std::string ClientOptions::GetDefaultLocation() { return common::util::GetCqspAppDataPath() + "/settings.hjson"; }
}  // namespace cqsp::client
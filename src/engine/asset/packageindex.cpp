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
#include "engine/asset/packageindex.h"

#include <hjson.h>

#include "engine/enginelogger.h"

namespace cqsp::asset {
PackageIndex::PackageIndex(const IVirtualDirectoryPtr& directory) : valid(false) {
    // Load the package information
    Hjson::DecoderOptions dec_opt;
    dec_opt.comments = false;
    dec_opt.duplicateKeyException = true;

    for (int i = 0; i < directory->GetSize(); i++) {
        auto resource_file = directory->GetFile(i);

        std::string resource_file_path = resource_file->Path();
        if (GetFilename(resource_file_path) != "resource.hjson") {
            continue;
        }

        // Then load the file and the path the entire time, and the type as well
        std::string asset_data = ReadAllFromVFileToString(resource_file.get());
        Hjson::Value asset_value;

        // Try to load and check for duplicate options, sadly hjson doesn't provide good
        // ways to see which keys are duplicated, except by exception, so we'll have
        // to do this as a hack for now
        try {
            asset_value = Hjson::Unmarshal(asset_data, dec_opt);
        } catch (Hjson::syntax_error& se) {
            ENGINE_LOG_WARN(se.what());
            ENGINE_LOG_WARN("File: {}", resource_file_path);
            // Then try again without the options
            dec_opt.duplicateKeyException = false;
            asset_value = Hjson::Unmarshal(asset_data, dec_opt);
        }

        // Scan through all the listed resources
        for (const auto& [key, val] : asset_value) {
            std::string path;
            if (GetParentPath(resource_file_path).empty()) {
                path = val["path"].to_string();
            } else {
                path = GetParentPath(resource_file_path) + "/" + val["path"].to_string();
            }
            ENGINE_LOG_TRACE("Loading path {}", path);
            assets[key] = AssetEntry(path, FromString(val["type"]));
        }
    }
}
}  // namespace cqsp::asset

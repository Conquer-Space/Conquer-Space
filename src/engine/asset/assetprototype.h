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
#pragma once

#include <string>

#include "engine/asset/asset.h"

namespace cqsp::asset {
enum PrototypeType {
    NONE = 0,
    TEXTURE,  //!< texture prototype
    SHADER,   //!< shader prototype
    FONT,     //!< Font prototype
    CUBEMAP,  //!< cubemap prototype
    MODEL     //!< Model prototype
};

///
/// Asset Prototypes are for assets that need additional processing
/// in the main thread, such as images.
///
class AssetPrototype {
 public:
    std::string key;
    /// <summary>
    /// Store the asset here so that at least we have the promise of an asset to the thing
    /// </summary>
    Asset* asset;
    virtual int GetPrototypeType() { return PrototypeType::NONE; }
};
}  // namespace cqsp::asset

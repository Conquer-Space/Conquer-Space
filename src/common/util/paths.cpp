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
#include "common/util/paths.h"

#include <filesystem>

char* cqsp::common::util::exe_path = "";

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <iostream>
#endif
#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

char* get_home_dir(uid_t uid) {
    struct passwd pwent;
    struct passwd *pwentp;
    char buf[1024];

    if (getpwuid_r(uid, &pwent, buf, sizeof buf, &pwentp)) {
        return "~";  // Easy trick because we couldn't find the dir
    } else {
        return pwent.pw_dir;
    }
}
#endif

std::string cqsp::common::util::GetCqspSavePath() {
    std::string directory = "";
    std::string dirname = "cqsp";
#ifdef _WIN32
    // Set log folder
    CHAR my_documents[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

    directory = std::string(my_documents);
#else
    // Get home directory to put the save data, and other data
    const char *homedir = get_home_dir(getuid());
    directory = std::string(homedir);
    dirname = "." + dirname;
#endif

    // Create folder
    auto filesystem = std::filesystem::path(directory);
    filesystem /= dirname;
    // Create dirs, and be done with it
    if (!std::filesystem::exists(filesystem))
        std::filesystem::create_directories(filesystem);
    return filesystem.string();
}

std::string cqsp::common::util::GetCqspExePath() {
    // Get current path
    std::filesystem::path p(exe_path);
    p = p.remove_filename();
    return std::filesystem::canonical(p).string();
}

std::string cqsp::common::util::GetCqspDataPath() {
    // If it's cmake, then the directory may be different
#if defined(_DEBUG) && defined(_MSC_VER)
    // so if it's debug, we'd automatically assume we're running from the local windows debugger
    // Because apparently linux doesn't build the debug version.
    // Not sure about other versions, but we'd probably have do deal with it in the future
    // Usually, the output is at build\src\Debug, so we need to access ../../../binaries/data
    return std::filesystem::canonical(std::filesystem::path(GetCqspExePath()) /
                                      ".."/".."/".."/"binaries"/"data").string();
#else
    // Then just search the default path
    std::string path = GetCqspExePath();
    // The folder structure is
    // - binaries
    //   - bin
    //   - data <-- data is here, so it's ../data/
    return std::filesystem::canonical(std::filesystem::path(GetCqspExePath()) / "../data").string();
#endif
}

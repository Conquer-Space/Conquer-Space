/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>

#include <iostream>
#endif
#ifdef __linux__
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
namespace cqsp::common::util {
char* get_home_dir(uid_t uid) {
    struct passwd pwent;
    struct passwd* pwentp;
    char buf[1024];

    if (getpwuid_r(uid, &pwent, buf, sizeof buf, &pwentp) != 0) {
        return (char*)"~";  // Easy trick because we couldn't find the dir
    } else {
        return pwent.pw_dir;
    }
}
} // namespace cqsp::common::util
#endif
//Split due to ifdefs
namespace cqsp::common::util {
std::string ExePath::exe_path = std::string();  // NOLINT

std::string GetCqspAppDataPath() {
    std::string directory;
    std::string dirname = "cqsp";
#ifdef _WIN32
    // Set log folder
    CHAR my_documents[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

    directory = std::string(my_documents);
#else
    // Get home directory to put the save data, and other data
    const char* homedir = get_home_dir(getuid());
    directory = std::string(homedir);
    dirname = "." + dirname;
#endif

    // Create folder
    auto filesystem = std::filesystem::path(directory);
    filesystem /= dirname;
    // Create dirs, and be done with it
    if (!std::filesystem::exists(filesystem)) std::filesystem::create_directories(filesystem);
    return filesystem.string();
}

std::string GetCqspExePath() {
    // Get current path
    std::filesystem::path p(ExePath::exe_path);
    p = p.remove_filename();
    return std::filesystem::canonical(p).string();
}

std::string GetCqspDataPath() {
    // If it's cmake, then the directory may be different
#if defined(_DEBUG) && defined(_MSC_VER)
    // so if it's debug, we'd automatically assume we're running from the local
    // windows debugger Because apparently linux doesn't build the debug
    // version. Not sure about other versions, but we'd probably have do deal
    // with it in the future. Usually, the output is at build\src\Debug, so we
    // need to access ../../../binaries/data
    return std::filesystem::canonical(std::filesystem::path(GetCqspExePath()) / ".." / ".." / ".." / "binaries" /
                                      "data")
        .string();
#else
    // Then just search the default path
    std::string path = GetCqspExePath();
    // The folder structure is
    // - binaries
    //   - bin
    //   - data <-- data is here, so it's ../data/
    return std::filesystem::canonical(std::filesystem::path(GetCqspExePath()) / ".." / "data").string();
#endif
}
std::string GetCqspSavePath() {
    std::filesystem::path path = GetCqspAppDataPath();
    return (path / "saves").string();
}
}  // namespace cqsp::common::util

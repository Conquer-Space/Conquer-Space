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
#include "engine/paths.h"

#include <filesystem>

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

std::string cqsp::engine::GetcqspPath() {
    std::string directory = "";
#ifdef _WIN32
    // Set log folder
    CHAR my_documents[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

    directory = std::string(my_documents);
#else
    const char *homedir = get_home_dir(getuid());
    directory = std::string(homedir);
#endif

    // Create folder
    auto filesystem = std::filesystem::path(directory);
    filesystem /= "cqsp";
    // Create dirs, and be done with it
    if (!std::filesystem::exists(filesystem))
        std::filesystem::create_directories(filesystem);
    return filesystem.string();
}

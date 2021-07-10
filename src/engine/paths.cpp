#include "engine/paths.h"

#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <shlobj.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

std::string conquerspace::engine::GetConquerSpacePath() {
    std::string directory = "";
#ifdef _WIN32
    // Set log folder
    CHAR my_documents[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

    directory = std::string(my_documents);
#else
    // Help
    struct passwd *pw = getpwuid(getuid());

    const char *homedir = pw->pw_dir;
    directory = std::string(homedir);
#endif
    // Create folder
    auto filesystem = std::filesystem::path(directory);
    filesystem /= "ConquerSpace";
    // Create dirs, and be done with it
    if (!std::filesystem::exists(filesystem))
        std::filesystem::create_directories(filesystem);
    return filesystem.string();
}
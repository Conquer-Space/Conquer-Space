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
#include "engine/asset/vfs/nativevfs.h"

#include <filesystem>

#include <iostream>
#include <spdlog/spdlog.h>

cqsp::asset::NativeFileSystem::NativeFileSystem(const char* _root) : root(_root){
}

std::shared_ptr<cqsp::asset::IVirtualFile> cqsp::asset::NativeFileSystem::Open(
    const char* file_path, FileModes modes) {
    std::string path = (std::filesystem::path(root) / file_path).string();
    std::shared_ptr<NativeFile> nfile = std::make_shared<NativeFile>(this, file_path);
    // Always open binary for carrige return purposes.
    // TODO(EhWhoAmI): Make this able to read text without carrige return.
    nfile->file.open(path, std::ios::binary);
    // Get the size
    nfile->file.seekg(0, std::ios::end);
    nfile->size = nfile->file.tellg();
    nfile->file.seekg(0);

    // Check if file is good, and return pointer to file
    if (nfile->file.good()) {
        return nfile;
    } else {
        return nullptr;
    }
}

void cqsp::asset::NativeFileSystem::Close(std::shared_ptr<IVirtualFile>& vf) {
    // Cast the pointer
    NativeFile* f = dynamic_cast<NativeFile*>(vf.get());
    f->file.close();
}

std::shared_ptr<cqsp::asset::IVirtualDirectory> cqsp::asset::NativeFileSystem::OpenDirectory(const char* dir) {
    // get the directory
    std::string path = std::filesystem::absolute(std::filesystem::path(root) / dir).string();
    if (!std::filesystem::is_directory(path)) {
        return nullptr;
    }

    std::shared_ptr<NativeDirectory> native_dir = std::make_shared<NativeDirectory>(this, dir);
    for (const auto& dir_entry :
         std::filesystem::recursive_directory_iterator(path)) {
        // Add to the virtual directory
        if (dir_entry.is_regular_file()) {
            native_dir->paths.push_back(std::filesystem::relative(dir_entry.path(), std::filesystem::path(root) / path).string());
        }
    }
    // Return the directory
    return native_dir;
}

bool cqsp::asset::NativeFileSystem::IsFile(const char* path) {
    return std::filesystem::is_regular_file(std::filesystem::path(root) / path);
}

bool cqsp::asset::NativeFileSystem::IsDirectory(const char* path) {
    return std::filesystem::is_directory(std::filesystem::path(root) / path);
}

bool cqsp::asset::NativeFileSystem::Exists(const char* path) {
    return std::filesystem::exists(std::filesystem::path(root) / path);
}

const char* cqsp::asset::NativeFile::Path() { return path; }

uint64_t cqsp::asset::NativeFile::Size() {
    return size;
}

void cqsp::asset::NativeFile::Read(uint8_t* buffer, int num_bytes) {
    // Text mode is mildly screwed up, because of carrige return on windows.
    file.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(num_bytes));
}

bool cqsp::asset::NativeFile::Seek(long offset, Offset origin) {
    std::ios_base::seekdir seek;
    switch (origin) {
        case Offset::Beg:
            seek = std::ios_base::beg;
            break;
        case Offset::Cur:
            seek = std::ios_base::cur;
            break;
        case Offset::End:
            seek = std::ios_base::end;
            break;
    }
    file.seekg(offset, seek);
    return true;
}

uint64_t cqsp::asset::NativeFile::Tell() { return file.tellg(); }

uint64_t cqsp::asset::NativeDirectory::GetSize() { return paths.size(); }

const char* cqsp::asset::NativeDirectory::GetRoot() { return root; }

std::shared_ptr<cqsp::asset::IVirtualFile>
cqsp::asset::NativeDirectory::GetFile(int index, FileModes modes) {
    // Get the file
    std::string path = (std::string(root) + "/" + paths[index]);
    return nfs->Open(path.c_str(), modes);
}

cqsp::asset::IVirtualFileSystem* cqsp::asset::NativeDirectory::GetFileSystem() {
    return nfs;
}

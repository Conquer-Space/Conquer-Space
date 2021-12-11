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
#include <memory>
#include <string>
#include <iostream>

cqsp::asset::NativeFileSystem::NativeFileSystem(const std::string& _root) : root(_root) {
}

std::shared_ptr<cqsp::asset::IVirtualFile>
cqsp::asset::NativeFileSystem::Open(const std::string& file_path, FileModes modes) {
    std::string file_pos = file_path;
    // Remove initial '/' if it has it, or std::filesystem goes crazy and thinks that it's
    // at the root directory of the drive, not the root directory of the filesystem
    if (file_pos.size() > 0 && file_pos.at(0) == '/') {
        file_pos = file_pos.erase(0, 1);
    }

    std::string path = (std::filesystem::path(root) / file_pos).string();

    // Erase slash in front, just in case.
    std::string file_name = file_path;
    if (file_name.size() > 0 && file_name.at(0) == '/') {
            file_name = file_name.erase(0, 1);
    }
    std::shared_ptr<NativeFile> nfile = std::make_shared<NativeFile>(this, file_name);
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

std::shared_ptr<cqsp::asset::IVirtualDirectory> cqsp::asset::NativeFileSystem::OpenDirectory(const std::string& dir) {
    // get the directory
    std::string path = std::filesystem::absolute(std::filesystem::path(root) / dir).string();
    if (!std::filesystem::is_directory(path)) {
        return nullptr;
    }

    std::shared_ptr<NativeDirectory> native_dir = std::make_shared<NativeDirectory>(this, dir);
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(path)) {
        // Add to the virtual directory
        if (!dir_entry.is_regular_file()) {
            continue;
        }
        // Replace the backslashes with forward slashes so that we keep it consistent
        std::string vfile_path = std::filesystem::relative(dir_entry.path(), std::filesystem::path(root) / path)
                .string();
        std::replace(vfile_path.begin(), vfile_path.end(), '\\', '/');
        // Remove initial '/' if it has it, or std::filesystem goes crazy and thinks that it's
        // at the root directory of the drive, not the root directory of the filesystem
        if (vfile_path.size() > 0 && vfile_path.at(0) == '/') {
            vfile_path = vfile_path.erase(0, 1);
        }

        native_dir->paths.push_back(vfile_path);
    }
    // Return the directory
    return native_dir;
}

bool cqsp::asset::NativeFileSystem::IsFile(const std::string& path) {
    return std::filesystem::is_regular_file(std::filesystem::path(root) / path);
}

bool cqsp::asset::NativeFileSystem::IsDirectory(const std::string& path) {
    return std::filesystem::is_directory(std::filesystem::path(root) / path);
}

bool cqsp::asset::NativeFileSystem::Exists(const std::string& path) {
    return std::filesystem::exists(std::filesystem::path(root) / path);
}

cqsp::asset::NativeFile::~NativeFile() { file.close(); }

const std::string& cqsp::asset::NativeFile::Path() { return path; }

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

const std::string& cqsp::asset::NativeDirectory::GetRoot() { return root; }

std::shared_ptr<cqsp::asset::IVirtualFile>
cqsp::asset::NativeDirectory::GetFile(int index, FileModes modes) {
    // Get the file
    std::string path = (std::string(root) + "/" + paths[index]);
    return nfs->Open(path.c_str(), modes);
}

const std::string& cqsp::asset::NativeDirectory::GetFilename(int index) {
    return paths[index];
}

cqsp::asset::IVirtualFileSystem* cqsp::asset::NativeDirectory::GetFileSystem() {
    return nfs;
}

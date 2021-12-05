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
#include "engine/asset/vfs/vfs.h"

cqsp::asset::VirtualMounter::~VirtualMounter() {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        delete it->second;
    }
}

void cqsp::asset::VirtualMounter::AddMountPoint(const char* point,
                                                IVirtualFileSystem* fs) {
    mount_points[std::string(point)] = fs;
}

std::shared_ptr<cqsp::asset::IVirtualFile> cqsp::asset::VirtualMounter::Open(
    const char* path, FileModes mode) {
    std::string path_str = path;
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path_str.rfind(it->first, 0) == 0) {
            // Then it's the mount point
            // Remove the path string and the path separator, and get the path
            // to get from the file system
            std::string mount_path = path_str.substr(it->first.size() + 1, path_str.size());
            return it->second->Open(mount_path.c_str(), mode);
        }
    }
    return nullptr;
}

std::shared_ptr<cqsp::asset::IVirtualDirectory> cqsp::asset::VirtualMounter::OpenDirectory(
    const char* path) {
    std::string path_str = path;
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path_str.rfind(it->first, 0) == 0) {
            // Then it's the mount point
            // Remove the path string and the path separator, and get the path
            // to get from the file system
            std::string mount_path = path_str.substr(it->first.size() + 1, path_str.size());
            return it->second->OpenDirectory(mount_path.c_str());
        }
    }
    return nullptr;
}

bool cqsp::asset::VirtualMounter::IsFile(const char* path) {
    std::string path_str = path;
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path_str.rfind(it->first, 0) == 0) {
            // Then it's the mount point
            // Remove the path string and the path separator, and get the path
            // to get from the file system
            std::string mount_path = path_str.substr(it->first.size() + 1, path_str.size());
            if(it->second->IsFile(mount_path.c_str())) {
                return true;
            }
        }
    }
    return false;
}

bool cqsp::asset::VirtualMounter::IsDirectory(const char* path) {
    std::string path_str = path;
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path_str.rfind(it->first, 0) == 0) {
            std::string mount_path = path_str.substr(it->first.size() + 1, path_str.size());
            if (it->second->IsDirectory(mount_path.c_str())) {
                return true;
            }
        }
    }
    return false;
}

bool cqsp::asset::VirtualMounter::Exists(const char* path) {
    std::string path_str = path;
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path_str.rfind(it->first, 0) == 0) {
            std::string mount_path = path_str.substr(it->first.size() + 1, path_str.size());
            if (it->second->Exists(mount_path.c_str())) {
                return true;
            }
        }
    }
    return false;
}

uint8_t* cqsp::asset::ReadAllFromVFile(IVirtualFile* file) {
    int size = file->Size();
    uint8_t* buffer = new uint8_t[size];
    file->Read(buffer, size);
    return buffer;
}

int cqsp::asset::ReadAllFromVFile(uint8_t* buf, IVirtualFile* file) {
    int size = file->Size();
    buf = new uint8_t[size];
    file->Read(buf, size);
    return size;
}

std::string cqsp::asset::ReadAllFromVFileToString(IVirtualFile* file) {
    int size = file->Size();
    uint8_t* buf = new uint8_t[size];
    file->Read(buf, size);
    // Output to string
    std::string str(reinterpret_cast<char*>(buf), size);
    // Now convert to a proper string
    size_t start_pos = 0;
    // Replace carrige returns because it's text mode
    while((start_pos = str.find("\r\n", start_pos)) != std::string::npos) {
        str.replace(start_pos, strlen("\r\n"), "\n");
        start_pos += 1; // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    delete[] buf;
    return str;
}
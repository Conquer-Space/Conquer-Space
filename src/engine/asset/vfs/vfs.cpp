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
#include "engine/asset/vfs/vfs.h"

#include <cstring>

namespace cqsp::asset {
VirtualMounter::~VirtualMounter() {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        delete it->second;
    }
}

void VirtualMounter::AddMountPoint(const std::string& path, IVirtualFileSystem* fs) {
    mount_points[std::string(path)] = fs;
}

std::shared_ptr<IVirtualFile> VirtualMounter::Open(const std::string& path, FileModes mode) {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path.rfind(it->first, 0) == 0) {
            // Then it's the mount point
            // Remove the path string and the path separator, and get the path
            // to get from the file system
            std::string mount_path = path.substr(it->first.size() + 1, path.size());
            return it->second->Open(mount_path, mode);
        }
    }
    return nullptr;
}

std::shared_ptr<IVirtualFile> VirtualMounter::Open(const std::string& mount, const std::string& path, FileModes mode) {
    return mount_points[mount]->Open(path, mode);
}

std::shared_ptr<IVirtualDirectory> VirtualMounter::OpenDirectory(const std::string& path) {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path.rfind(it->first, 0) != 0) {
            continue;
        }
        // Then it's the mount point
        // Remove the path string and the path separator, and get the path
        // to get from the file system
        std::string mount_path;
        if (it->first.size() >= path.size()) {
            mount_path = it->first;
        } else {
            mount_path = path.substr(it->first.size() + 1, path.size());
        }

        return it->second->OpenDirectory(mount_path);
    }
    return nullptr;
}

std::shared_ptr<IVirtualDirectory> VirtualMounter::OpenDirectory(const std::string& mount, const std::string& path) {
    return mount_points[mount]->OpenDirectory(path);
}

bool VirtualMounter::IsFile(const std::string& path) {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path.rfind(it->first, 0) == 0) {
            // Then it's the mount point
            // Remove the path string and the path separator, and get the path
            // to get from the file system
            std::string mount_path = path.substr(it->first.size() + 1, path.size());
            if (it->second->IsFile(mount_path)) {
                return true;
            }
        }
    }
    return false;
}

bool VirtualMounter::IsFile(const std::string& mount, const std::string& path) {
    return mount_points[mount]->IsFile(path);
}

bool VirtualMounter::IsDirectory(const std::string& path) {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path.rfind(it->first, 0) == 0) {
            std::string mount_path = path.substr(it->first.size() + 1, path.size());
            if (it->second->IsDirectory(mount_path)) {
                return true;
            }
        }
    }
    return false;
}

bool VirtualMounter::IsDirectory(const std::string& mount, const std::string& path) {
    return mount_points[mount]->IsDirectory(path);
}

bool VirtualMounter::Exists(const std::string& path) {
    for (auto it = mount_points.begin(); it != mount_points.end(); it++) {
        if (path.rfind(it->first, 0) == 0) {
            std::string mount_path = path.substr(it->first.size() + 1, path.size());
            if (it->second->Exists(mount_path)) {
                return true;
            }
        }
    }
    return false;
}

bool VirtualMounter::Exists(const std::string& mount, const std::string& path) {
    return mount_points[mount]->Exists(path);
}

std::vector<uint8_t> ReadAllFromVFile(IVirtualFile* file) {
    int size = file->Size();
    std::vector<uint8_t> buffer;
    buffer.resize(size);
    file->Read(buffer.data(), size);
    return buffer;
}

std::string ReadAllFromVFileToString(IVirtualFile* file) {
    int size = file->Size();
    uint8_t* buf = new uint8_t[size];
    file->Read(buf, size);
    // Output to string
    std::string str(reinterpret_cast<char*>(buf), size);
    // Now convert to a proper string
    size_t start_pos = 0;
    // Replace carrige returns because it's text mode
    // Carrige return is 2 characters long, and findflaws is stupid and complaining about strlen.
    const int return_length = 2;
    while ((start_pos = str.find("\r\n", start_pos)) != std::string::npos) {
        str.replace(start_pos, return_length, "\n");
        start_pos += 1;  // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    delete[] buf;
    return str;
}
}  // namespace cqsp::asset

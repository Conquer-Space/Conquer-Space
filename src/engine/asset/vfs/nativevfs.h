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
#pragma once

#include <fstream>

#include "engine/asset/vfs/vfs.h"

namespace cqsp {
namespace asset {
class NativeFileSystem;

class NativeFile : public IVirtualFile {
 public:
    NativeFile(NativeFileSystem* _nfs) : IVirtualFile(), nfs(_nfs), path("") {}
    NativeFile(NativeFileSystem* _nfs, const char* file) : IVirtualFile(), nfs(_nfs), path(file) {}

    const char* Path();
    uint64_t Size();

    void Read(uint8_t* buffer, int bytes);

    bool Seek(long offset, Offset origin);
    uint64_t Tell();

    IVirtualFileSystem* GetFileSystem() {
        return (IVirtualFileSystem*) nfs;
    }

    friend NativeFileSystem;

 private:
    const char* path;
    std::ifstream file;
    int size;

    const NativeFileSystem* nfs;
};

class NativeFileSystem : public IVirtualFileSystem {
 public:
    NativeFileSystem(const char* root);
    bool Initialize() override {
        return true;
    }

    std::shared_ptr<IVirtualFile> Open(const char* path, FileModes = None) override;
    void Close(std::shared_ptr<IVirtualFile>&);

 private:
    const char* root;
};
}
}  // namespace cqsp

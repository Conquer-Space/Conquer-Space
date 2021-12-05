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
#include <vector>

#include "engine/asset/vfs/vfs.h"

namespace cqsp {
namespace asset {
class NativeFileSystem;
class NativeDirectory;

class NativeFile : public IVirtualFile {
 public:
    NativeFile(NativeFileSystem* _nfs) : IVirtualFile(), nfs(_nfs), path("") {}
    NativeFile(NativeFileSystem* _nfs, const char* file) : IVirtualFile(), nfs(_nfs), path(file) {}

    const char* Path()  override;
    uint64_t Size()  override;

    void Read(uint8_t* buffer, int bytes)  override;

    bool Seek(long offset, Offset origin);
    uint64_t Tell()  override;

    IVirtualFileSystem* GetFileSystem() override {
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
    NativeFileSystem(std::string _root) : root(_root) {}
    bool Initialize() override {
        return true;
    }

    std::shared_ptr<IVirtualFile> Open(const char* path, FileModes = None) override;
    void Close(std::shared_ptr<IVirtualFile>&) override;
    std::shared_ptr<IVirtualDirectory> OpenDirectory(const char* path) override;

    bool IsFile(const char* path) override;
    bool IsDirectory(const char* path) override;
    bool Exists(const char* path) override;
    const char* GetRoot() { return root.c_str(); }

 private:
    std::string root;
    friend NativeDirectory;
};

class NativeDirectory : public IVirtualDirectory {
 public:
    NativeDirectory(NativeFileSystem* _nfs, const char* _root) : nfs(_nfs), root(_root) {}

    virtual uint64_t GetSize() override;
    const char* GetRoot() override;
    std::shared_ptr<IVirtualFile> GetFile(int index, FileModes modes = None) override;
    IVirtualFileSystem* GetFileSystem() override;
 private:
    friend NativeFileSystem;
    std::vector<std::string> paths;
    const char* root;
    NativeFileSystem* nfs;
};
}
}  // namespace cqsp

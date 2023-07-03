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
#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "engine/asset/vfs/vfs.h"

namespace cqsp {
namespace asset {
class NativeFileSystem;
class NativeDirectory;

class NativeFile : public IVirtualFile {
 public:
    explicit NativeFile(NativeFileSystem* _nfs) : IVirtualFile(), nfs(_nfs), path("") {}
    NativeFile(NativeFileSystem* _nfs, const std::string& file) : IVirtualFile(), nfs(_nfs), path(file) {}

    ~NativeFile();

    const std::string& Path() override;
    uint64_t Size() override;

    void Read(uint8_t* buffer, int bytes) override;

    bool Seek(long offset, Offset origin) override;
    uint64_t Tell() override;

    IVirtualFileSystem* GetFileSystem() override { return reinterpret_cast<IVirtualFileSystem*>(nfs); }

    friend NativeFileSystem;

 private:
    std::string path;
    std::ifstream file;
    int size;

    NativeFileSystem* const nfs;
};

class NativeFileSystem : public IVirtualFileSystem {
 public:
    explicit NativeFileSystem(const std::string& root);
    ~NativeFileSystem();

    bool Initialize() override { return true; }

    std::shared_ptr<IVirtualFile> Open(const std::string& path, FileModes = None) override;
    void Close(std::shared_ptr<IVirtualFile>&) override;
    std::shared_ptr<IVirtualDirectory> OpenDirectory(const std::string& path) override;

    bool IsFile(const std::string& path) override;
    bool IsDirectory(const std::string& path) override;
    bool Exists(const std::string& path) override;
    const std::string& GetRoot() { return root; }

 private:
    std::string root;
    friend NativeDirectory;
};

class NativeDirectory : public IVirtualDirectory {
 public:
    NativeDirectory(NativeFileSystem* _nfs, const std::string& _root) : nfs(_nfs), root(_root) {}

    uint64_t GetSize() override;
    const std::string& GetRoot() override;
    std::shared_ptr<IVirtualFile> GetFile(int index, FileModes modes = None) override;
    const std::string& GetFilename(int index) override;
    IVirtualFileSystem* GetFileSystem() override;

 private:
    friend NativeFileSystem;
    std::vector<std::string> paths;
    std::string root;
    NativeFileSystem* const nfs;
};
}  // namespace asset
}  // namespace cqsp

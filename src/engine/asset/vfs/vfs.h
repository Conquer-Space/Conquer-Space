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

#include <memory>

namespace cqsp {
namespace asset {
enum FileModes {
    None = 0,
    Binary = 1 << 0 // If true, file is opened in binary mode, if false, file is opened in text mode
};

enum class Offset {
    Beg,
    End,
    Cur
};

class IVirtualFile;

/// <summary>
/// The main functionality for this is to read files, so writing to files will
/// not really be supported.
/// </summary>
class IVirtualFileSystem {
   public:
    IVirtualFileSystem() {}
    virtual ~IVirtualFileSystem() {}

    virtual bool Initialize() = 0;
    /// <summary>
    /// Opens file from the filesystem. Returns nullptr if file is invalid
    /// </summary>
    virtual std::shared_ptr<IVirtualFile> Open(const char* path, FileModes modes = None) = 0;

    virtual void Close(std::shared_ptr<IVirtualFile>&) = 0;
 private:
};

class IVirtualFile {
 public:
    IVirtualFile() {}
    virtual ~IVirtualFile() {}

    /// <summary>
    /// Size of file in bytes
    /// </summary>
    virtual uint64_t Size() = 0;

    virtual void Read(uint8_t* buffer, int num_bytes) = 0;
    virtual bool Seek(long offset, Offset origin = Offset::Cur) = 0;
    virtual uint64_t Tell() = 0;

    /// <summary>
    /// Get file path relative to the filesystem.
    /// </summary>
    virtual const char* Path() = 0;


    virtual IVirtualFileSystem* GetFileSystem() = 0;
 protected:
};
}  // namespace asset
}  // namespace cqsp

/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cqsp::asset {
enum FileModes {
    Text = 0,        // Opens in text mode
    Binary = 1 << 0  // If true, file is opened in binary mode, if false, file is opened in text mode
};

enum class Offset { Beg, End, Cur };

class IVirtualFile;
class IVirtualDirectory;
typedef std::shared_ptr<IVirtualFile> IVirtualFilePtr;
typedef std::shared_ptr<IVirtualDirectory> IVirtualDirectoryPtr;

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
    virtual IVirtualFilePtr Open(const std::string& path, FileModes modes = Text) = 0;

    /// <summary>
    /// Closes the file.
    /// </summary>
    /// <param name=""></param>
    virtual void Close(IVirtualFilePtr&) = 0;

    /// <summary>
    /// Lists all the files in the directory.
    /// Does not list directories in the list because for our purposes, we do not need
    /// to traverse and process directories with this.
    /// Returns nullptr if path does not exist or is not a directory.
    /// </summary>
    virtual IVirtualDirectoryPtr OpenDirectory(const std::string& path) = 0;

    /// <summary>
    /// Is the file a file
    /// </summary>
    virtual bool IsFile(const std::string& path) = 0;

    /// <summary>
    /// Is the file a directory
    /// </summary
    virtual bool IsDirectory(const std::string& path) = 0;

    /// <summary>
    /// Does the file exist
    /// </summary>
    virtual bool Exists(const std::string& path) = 0;
};

/// <summary>
/// Meant to list all the files and sub files in the directory.
/// This is sort of a replacement for directory iterator.
/// If you want to cd a directory, just access it from the initial file system, I guess.
/// </summary>
class IVirtualDirectory {
 public:
    IVirtualDirectory() {}
    virtual ~IVirtualDirectory() {}

    /// <summary>
    /// Get number of files in directory
    /// </summary>
    virtual uint64_t GetSize() = 0;

    /// <summary>
    /// The root directory, relative to the filesystem.
    /// </summary>
    /// <returns></returns>
    virtual const std::string& GetRoot() = 0;

    /// <summary>
    /// Opens file of index.
    /// </summary>
    virtual IVirtualFilePtr GetFile(int index, FileModes modes = Text) = 0;

    /// <summary>
    /// Get file name relative to this directory
    /// </summary>
    virtual const std::string& GetFilename(int index) = 0;

    /// <summary>
    /// Get the filesystem this refers to.
    /// </summary>
    virtual IVirtualFileSystem* GetFileSystem() = 0;
};

class IVirtualFile {
 public:
    IVirtualFile() {}
    virtual ~IVirtualFile() {}

    /// <summary>
    /// Size of file in bytes
    /// </summary>
    virtual uint64_t Size() = 0;

    /// <summary>
    /// Reads `num_bytes` of bytes into `buffer`. Frees memory for the number
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="num_bytes"></param>
    virtual void Read(uint8_t* buffer, int num_bytes) = 0;

    /// <summary>
    /// Goto position of file, `offset` away from `origin`
    /// </summary>
    virtual bool Seek(long offset, Offset origin = Offset::Cur) = 0;

    /// <summary>
    /// Get current file position
    /// </summary>
    virtual uint64_t Tell() = 0;

    /// <summary>
    /// Get file path relative to the filesystem.
    /// </summary>
    virtual const std::string& Path() = 0;

    virtual IVirtualFileSystem* GetFileSystem() = 0;
};

class VirtualMounter {
 public:
    ~VirtualMounter();
    void AddMountPoint(const std::string& path, IVirtualFileSystem* fs);
    /// <summary>
    /// Opens file.
    /// </summary>
    IVirtualFilePtr Open(const std::string& path, FileModes mode = FileModes::Text);

    /// <summary>
    /// Opens file in mounted filesystem `mount`, with the mount path relative path `path`
    /// </summary>
    IVirtualFilePtr Open(const std::string& mount, const std::string& path, FileModes mode = FileModes::Text);

    /// <summary>
    /// Opens directory
    /// </summary>
    std::shared_ptr<IVirtualDirectory> OpenDirectory(const std::string& path);

    /// <summary>
    /// Opens directory in mounted filesystem `mount`, with the mount path relative path `path`
    /// </summary>
    std::shared_ptr<IVirtualDirectory> OpenDirectory(const std::string& mount, const std::string& path);

    bool IsFile(const std::string& path);
    bool IsFile(const std::string& mount, const std::string& path);
    bool IsDirectory(const std::string& path);
    bool IsDirectory(const std::string& mount, const std::string& path);
    bool Exists(const std::string& path);
    bool Exists(const std::string& mount, const std::string& path);

 private:
    std::map<std::string, IVirtualFileSystem*> mount_points;
};

// These functions feel like a hack

/// <summary>
/// Reads all the data from the virtual file.
/// </summary>
/// <param name=""></param>
/// <returns></returns>
std::vector<uint8_t> ReadAllFromVFile(IVirtualFile*);

/// <summary>
/// Don't really want this, but ah well, it cannot be helped.
/// </summary>
std::string ReadAllFromVFileToString(IVirtualFile* file);

/// <summary>
/// Gets filename from path.
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
inline std::string GetFilename(const std::string& path) {
    size_t last = path.find_last_of("/\\");
    if (last == std::string::npos) {
        // It's probably the whole thing
        return path;
    }
    return path.substr(last + 1);
}

inline std::string GetParentPath(const std::string& path) {
    size_t last = path.find_last_of("/\\");

    if (last == std::string::npos) {
        return "";
    }
    return path.substr(0, last);
}
}  // namespace cqsp::asset

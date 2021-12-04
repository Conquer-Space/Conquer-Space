#include "engine/asset/vfs/nativevfs.h"

#include <filesystem>

#include <iostream>

cqsp::asset::NativeFileSystem::NativeFileSystem(const char* _root) : root(_root){
}

std::shared_ptr<cqsp::asset::IVirtualFile> cqsp::asset::NativeFileSystem::Open(
    const char* file_path, FileModes modes) {
    std::string path = std::filesystem::absolute(std::filesystem::path(root) / file_path).string();
    std::shared_ptr<NativeFile> nfile = std::make_shared<NativeFile>(this, file_path);
    if (modes & FileModes::Binary) {
        // Open binary
        nfile->file.open(path, std::ios::binary);
    } else {
        nfile->file.open(path);
    }
    // Set the size
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

const char* cqsp::asset::NativeFile::Path() { return path; }

uint64_t cqsp::asset::NativeFile::Size() {
    return size;
}

void cqsp::asset::NativeFile::Read(uint8_t* buffer, int num_bytes) {
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

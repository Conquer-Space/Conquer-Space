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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <filesystem>

#include "engine/asset/vfs/nativevfs.h"

class NativeVfsTest : public ::testing::Test {
 protected:
    NativeVfsTest() : nfs(package_root.c_str()) {
        full_name = (std::filesystem::path(package_root) / test_file).string();
    }
    void SetUp() {
        if (!std::filesystem::exists(full_name)) {
            FAIL() << "File " << full_name << ", which is needed for this test";
        }
        nfs.Initialize();
    }

    std::string package_root = "../data/core";
    std::string test_file = "info.hjson";
    std::string full_name;
    cqsp::asset::NativeFileSystem nfs;
};

/// <summary>
/// Tests if opening files works as needed.
/// </summary>
TEST_F(NativeVfsTest, OpenTest) {
    // Check if the test files exist, if they do, then complain

    auto ptr = nfs.Open(test_file.c_str());
    ASSERT_NE(ptr, nullptr);
    ASSERT_STREQ(ptr->Path(), test_file.c_str());
    // Close the file
    nfs.Close(ptr);
}

TEST_F(NativeVfsTest, FileSizeTest) {
    int size = std::filesystem::file_size(full_name);

    auto ptr = nfs.Open(test_file.c_str());
    ASSERT_EQ(ptr->Size(), size);

    // Close the file
    nfs.Close(ptr);
}

TEST_F(NativeVfsTest, FileReadTest) {
    // Read the entire file and compare contents, I guess
    int size = std::filesystem::file_size(full_name);

    auto ptr = nfs.Open(test_file.c_str());
    uint8_t* test_buffer = new uint8_t[size];
    ptr->Read(test_buffer, size);

    // Read file for what the file actually says
    std::ifstream file(full_name);
    file.seekg(0, std::ios::end);
    file.seekg(0);
    char *true_buffer = new char[size];
    file.read(true_buffer, size);

    std::string to_test(reinterpret_cast<char*>(test_buffer), size);
    std::string truth(true_buffer, size);

    ASSERT_EQ(to_test, truth);

    // Close the file
    nfs.Close(ptr);
    delete[] test_buffer;
    delete[] true_buffer;
}

TEST_F(NativeVfsTest, SeekTest) {
    int size = std::filesystem::file_size(full_name);

    auto ptr = nfs.Open(test_file.c_str());
    ptr->Seek(10);
    ASSERT_EQ(ptr->Tell(), 10);

    ptr->Seek(30);
    ASSERT_EQ(ptr->Tell(), 40);

    ptr->Seek(10, cqsp::asset::Offset::Beg);
    ASSERT_EQ(ptr->Tell(), 10);

    ptr->Seek(-10, cqsp::asset::Offset::End);
    ASSERT_EQ(ptr->Tell(), size - 10);
    nfs.Close(ptr);
}

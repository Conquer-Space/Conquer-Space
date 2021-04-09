/*
 * Copyright 2021 Conquer Space
*/
#include <gtest/gtest.h>

#include <sstream>

#include "client/clientoptions.h"

// Tests for input from client options
TEST(Client_ClientOptionsTest, InputTest) {
    namespace cqspco = conquerspace::client;

    cqspco::ProgramOptions programOptions;

    std::stringstream input;

    input << "{\n";
    input << "window : { \n";
    input << "width : 1280 \n";
    input << "height : 720 \n";
    input << "} \n";
    input << "}";

    programOptions.LoadOptions(input);

    EXPECT_EQ(1280, static_cast<int>(programOptions["window"]["width"]));
    EXPECT_EQ(720, static_cast<int>(programOptions["window"]["height"]));
}

// Tests for input from client options
TEST(Client_ClientOptionsTest, DefaultOptionsTest) {
    namespace cqspco = conquerspace::client;

    cqspco::ProgramOptions programOptions;

    programOptions.LoadDefaultOptions();

    EXPECT_EQ(1280, static_cast<int>(programOptions["window"]["width"]));
    EXPECT_EQ(720, static_cast<int>(programOptions["window"]["height"]));
}

/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <map>
#include <string>

#include <boost/chrono/chrono.hpp>

extern std::map<std::string, int> profiler_information_map;
#define BEGIN_TIMED_BLOCK(NAME) boost::chrono::high_resolution_clock::time_point \
                                block_start_##NAME = boost::chrono::high_resolution_clock::now();

#define END_TIMED_BLOCK(NAME) boost::chrono::high_resolution_clock::time_point block_end_##NAME = \
                                                boost::chrono::high_resolution_clock::now(); \
                                profiler_information_map[#NAME] =\
                                boost::chrono::duration_cast<boost::chrono::microseconds> \
                                        (block_end_##NAME - block_start_##NAME).count();

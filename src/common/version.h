/*
* Copyright 2021 Conquer Space
*/
#pragma once

#define GIT_INFO "8e34f6a-dirty"
#define CQSP_VERSION "0.0.0"

// Make sure that things are still defined
#ifndef GIT_INFO
#define GIT_INFO "N/A"
#endif
#ifndef CQSP_VERSION
#define CQSP_VERSION "N/A"
#endif

#define CQSP_VERSION_STRING CQSP_VERSION " (" GIT_INFO ")"

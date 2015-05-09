
#pragma once
#define WIN32_LEAN_AND_MEAN
#define WINVER       0x0502 // WinXP++
#define _WIN32_WINNT 0x0502
#include <windows.h>
#include <stdlib.h>
#include <intrin.h>
#pragma intrinsic(memset, memcpy, strcat, strcmp, strcpy, strlen)

// IDA libs
#define USE_DANGEROUS_FUNCTIONS
#include <ida.hpp>
#include <auto.hpp>
#include <loader.hpp>
#include <typeinf.hpp>

#include "Utility.h"

#define MY_VERSION MAKEWORD(5, 1) // Low, high, 0 to 99

// ****************************************************************************
// File: Utility.h
// Desc: Utility functions
//
// ****************************************************************************
#pragma once
#include <string>

// Size of string with out terminator
#define SIZESTR(x) (sizeof(x) - 1)

// Data and function alignment
#define ALIGN(_x_) __declspec(align(_x_))

// Tick IDA's Qt message pump so it will show msg() output
#define refreshUI() WaitBox::processIdaEvents()

#define CATCH() catch (...) { msg("** Exception in %s()! ***\n", __FUNCTION__); }

// Stack alignment trick, based on Douglas Walker's post
// http://www.gamasutra.com/view/feature/3975/data_alignment_part_2_objects_on_.php
#define STACKALIGN(name, type) \
	BYTE space_##name[sizeof(type) + (16-1)]; \
	type &name = *reinterpret_cast<type *>((UINT_PTR) (space_##name + (16-1)) & ~(16-1))

// ea_t zero padded hex number format
#ifndef __EA64__
#define EAFORMAT "%08X"
#else
#define EAFORMAT "%016I64X"
#endif

typedef double TIMESTAMP;
#define SECOND 1
#define MINUTE (60 * SECOND)
#define HOUR   (60 * MINUTE)
#define DAY    (HOUR * 24)

TIMESTAMP getTimeStamp();
LPCSTR timeString(TIMESTAMP Time);
LPSTR  prettyNumberString(UINT64 n, __bcount(32) LPSTR buffer);

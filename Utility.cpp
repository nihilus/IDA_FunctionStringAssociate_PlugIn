
// ****************************************************************************
// File: Utility.cpp
// Desc: Utility functions
//
// ****************************************************************************
#include "stdafx.h"


// ****************************************************************************
// Func: GetTimeSamp()
// Desc: Get elapsed factional seconds
//
// ****************************************************************************
TIMESTAMP getTimeStamp()
{
	LARGE_INTEGER tLarge;
	QueryPerformanceCounter(&tLarge);

	static ALIGN(16) TIMESTAMP s_ClockFreq;
	if(s_ClockFreq == 0.0)
	{
		LARGE_INTEGER tLarge;
		QueryPerformanceFrequency(&tLarge);
		s_ClockFreq = (TIMESTAMP) tLarge.QuadPart;
	}

	return((TIMESTAMP) tLarge.QuadPart / s_ClockFreq);
}


// Return a comma formatted string for a given number
LPSTR prettyNumberString(UINT64 n, __bcount(32) LPSTR buffer)
{
    std::string s;
    int c = 0;
    do
    {
        s.insert(0, 1, char('0' + (n % 10)));
        n /= 10;
        if ((c += (3 && n)) >= 3)
        {
            s.insert(0, 1, ',');
            c = 0;
        }
    } while (n);
    strncpy(buffer, s.c_str(), 31);
    return(buffer);
}


// Get a pretty delta time string for output
LPCSTR timeString(TIMESTAMP time)
{
	static char szBuff[64];

	if(time >= HOUR)
		_snprintf(szBuff, SIZESTR(szBuff), "%.2f hours", (time / (TIMESTAMP) HOUR));
	else
	if(time >= MINUTE)
		_snprintf(szBuff, SIZESTR(szBuff), "%.2f minutes", (time / (TIMESTAMP) MINUTE));
	else
	if(time < (TIMESTAMP) 0.01)
		_snprintf(szBuff, SIZESTR(szBuff), "%.2f milliseconds", (time * (TIMESTAMP) 1000.0));
	else
		_snprintf(szBuff, SIZESTR(szBuff), "%.2f seconds", time);

	return(szBuff);
}
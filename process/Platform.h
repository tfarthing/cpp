#pragma once

#undef WINVER
#undef _WIN32_WINNT
#define WINVER _WIN32_WINNT_WIN8
#define _WIN32_WINNT _WIN32_WINNT_WIN8

#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX

#pragma warning (disable: 4355)

#include <winsock2.h>
#include <Windows.h>

namespace cpp
{

    struct Platform
    {
		constexpr static bool isWindows( )
            { return true; }
    };

}

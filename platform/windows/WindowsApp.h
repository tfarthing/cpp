#pragma once

#include "../../../cpp/data/String.h"

namespace cpp
{
	namespace windows
	{
		struct App
		{
			static String::Array parseCommandLine( const String & cmdline );
		};
	}
}
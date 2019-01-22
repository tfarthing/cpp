#pragma once

#include "../../data/String.h"

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
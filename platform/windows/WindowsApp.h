#pragma once

#include "../../../cpp/data/DataArray.h"

namespace cpp
{
	namespace windows
	{
		struct App
		{
			static StringArray parseCommandLine( const String & cmdline );
		};
	}
}
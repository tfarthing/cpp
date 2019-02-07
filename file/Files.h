#pragma once

#include <filesystem>

#include "../../cpp/data/String.h"

namespace cpp
{

	namespace Files = std::filesystem;
	using FilePath = Files::path;

	inline String toString( const FilePath & value )
	{
		return value.generic_u8string();
	}



}
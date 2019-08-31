#pragma once

/*

	FilePath is an encapsulation of std::filesystem::path.
		(1) always encoded as utf-8
		(2) always uses '/' as seperator

*/

#include <type_traits>
#include <filesystem>
#include <vector>

#include "../../cpp/data/Memory.h"
#include "../../cpp/data/String.h"


namespace cpp
{

	struct FilePath
	{
	public:
		typedef std::vector<FilePath> Array;

		static FilePath						currentPath( );
		static FilePath						tempPath( );
		static FilePath						tempFile( Memory prefix = "temp", Memory ext = "tmp" );

											FilePath( );
                                            FilePath( const FilePath & copy );
                                            FilePath( FilePath && move ) noexcept;
                                            FilePath( const char * path );
                                            FilePath( const Memory & path );
                                            FilePath( const std::string & path );
                                            FilePath( const String & path );

		bool								isEmpty( ) const;
		bool								notEmpty( ) const;
		bool								operator!( ) const;

		bool								isAbsolute( ) const;

		FilePath							normal( ) const;
		FilePath							relative( const FilePath & base ) const;
		FilePath							absolute( ) const;
		FilePath							canonical( ) const;

		static int							compare( const FilePath & lhs, const FilePath & rhs );

		FilePath &							assign( const FilePath & copy );
		FilePath &							assign( FilePath && move );

		FilePath &							operator=( const FilePath & copy );
		FilePath &							operator=( FilePath && move );

		FilePath &							append( const FilePath & path );
		FilePath &							operator/=( const FilePath & path );

		FilePath &							concat( const FilePath & path );
		FilePath &							operator+=( const FilePath & path );

		void								clear( );

		//  FilePath{ "c:/dir/file.ext1.ext2" }.parent() == "c:/dir"
		FilePath							parent( ) const;
		//  FilePath{ "c:/dir/file.ext1.ext2" }.filename() == "file.ext1.ext2"
		Memory								filename( ) const;
		//  FilePath{ "c:/dir/file.ext1.ext2" }.name() == "file"
		//  FilePath{ "c:/dir/file.ext1.ext2" }.name(true) == "file.ext1"
		Memory								stem( bool shortExtension = true ) const;
		//  FilePath{ "c:/dir/file.ext1.ext2" }.extension() == "ext1.ext2"
		//  FilePath{ "c:/dir/file.ext1.ext2" }.extension(true) == "ext2"
		Memory								extension( bool shortExtension = true ) const;

        std::filesystem::path               to_path( ) const;
        std::string                         toString( bool nativeSeperator = false ) const;
		const wchar_t *						toWindows( std::wstring & buffer = std::wstring{} ) const;

        String                              path;
	};

}

cpp::FilePath operator/( const cpp::FilePath & lhs, const cpp::FilePath & rhs );

bool operator==( const cpp::FilePath & lhs, const cpp::FilePath & rhs );
bool operator!=( const cpp::FilePath & lhs, const cpp::FilePath & rhs );
bool operator<( const cpp::FilePath & lhs, const cpp::FilePath & rhs );
bool operator<=( const cpp::FilePath & lhs, const cpp::FilePath & rhs );
bool operator>( const cpp::FilePath & lhs, const cpp::FilePath & rhs );
bool operator>=( const cpp::FilePath & lhs, const cpp::FilePath & rhs );

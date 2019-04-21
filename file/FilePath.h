#pragma once

/*

	FilePath is an encapsulation of std::filesystem::path.
		(1) always encoded as utf-8
		(2) always uses '/' as seperator

*/

#include <filesystem>
#include <vector>

#include "../../cpp/data/Memory.h"


namespace cpp
{

	class FilePath
	{
	public:
		typedef std::vector<FilePath> Array;

		static FilePath						currentPath( );
		static FilePath						tempPath( );
		static FilePath						tempFile( Memory prefix = "temp", Memory ext = "tmp" );

											FilePath( );
											FilePath( std::string path );
											FilePath( const std::filesystem::path & path );
											FilePath( const FilePath & copy );
											FilePath( FilePath && move );
		
		bool								isEmpty( ) const;
		bool								notEmpty( ) const;
		bool								operator!( ) const;

		bool								isAbsolute( ) const;

		FilePath							normal( ) const;
		FilePath							relative( const FilePath & base ) const;
		FilePath							absolute( ) const;
		FilePath							canonical( ) const;

		static int							compare( const FilePath & lhs, const FilePath & rhs );

		FilePath &							assign( std::string path );
		FilePath &							assign( const FilePath & copy );
		FilePath &							assign( FilePath && move );

		FilePath &							operator=( std::string path );
		FilePath &							operator=( const FilePath & copy );
		FilePath &							operator=( FilePath && move );

		FilePath &							append( Memory path );
		FilePath &							operator/=( Memory path );

		FilePath &							concat( Memory path );
		FilePath &							operator+=( Memory path );

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

											operator std::filesystem::path( ) const;
											operator std::string( ) const;
		std::string							toString( bool nativeSeperator = false ) const;

	private:
		std::string m_path;
	};

}

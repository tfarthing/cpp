#ifndef TEST

#include "../../cpp/data/Integer.h"
#include "../../cpp/data/String.h"
#include "../../cpp/file/Files.h"
#include "../../cpp/file/FilePath.h"
#include "../../cpp/process/Program.h"
#include "../../cpp/process/Random.h"
#include "../../cpp/text/Utf16.h"




namespace cpp
{

	FilePath FilePath::currentPath( )
	{
		return FilePath{ std::filesystem::current_path( ).generic_string( ) };
	}


	FilePath FilePath::tempPath( )
	{
		return FilePath{ std::filesystem::temp_directory_path( ).generic_string( ) };
	}


	FilePath FilePath::tempFile( Memory prefix, Memory ext )
	{
		FilePath filepath = tempPath( );

		do
		{
			uint32_t rnd = (uint32_t)Program::rng( ).rand( );
            filepath = tempPath( ) / cpp::format( "%%.%", prefix, Integer::toHex( rnd ), ext );
		} while ( Files::exists( filepath ) );

		return filepath;
	}


	FilePath::FilePath( )
	{
	}


    FilePath::FilePath( const FilePath & copy )
        : path( copy.path )
    {
    }


    FilePath::FilePath( FilePath && move ) noexcept
        : path( std::move( move.path ) )
    {
    }

    FilePath::FilePath( const Memory & filepath )
        : path( std::filesystem::path{ (const char8_t *)filepath.begin( ), (const char8_t *)filepath.end( ) }.generic_string( ) )
    {
    }


    FilePath::FilePath( const char * filepath )
        : path( std::filesystem::path{ (const char8_t *)filepath }.generic_string( ) )
    {
    }


    FilePath::FilePath( const std::string & filepath )
        : path( std::filesystem::path{ (const char8_t *)filepath.c_str( ), (const char8_t *)filepath.c_str( ) + filepath.length( ) }.generic_string( ) )
    {
    }


    FilePath::FilePath( const String & filepath )
        : path( std::filesystem::path{ (const char8_t *)filepath.begin( ), (const char8_t *)filepath.end( ) }.generic_string( ) )
    {
    }


	bool FilePath::isEmpty( ) const
	{
		return path.isEmpty( );
	}


	bool FilePath::notEmpty( ) const
	{
		return path.notEmpty( );
	}


	bool FilePath::operator!( ) const
	{
		return path.notEmpty( );
	}


	bool FilePath::isAbsolute( ) const
	{
		return to_path( ).is_absolute( );
	}


	FilePath FilePath::normal( ) const
	{
        return to_path( ).lexically_normal( ).generic_string( );
	}


	FilePath FilePath::relative( const FilePath & base ) const
	{
        return to_path( ).lexically_relative( base.to_path( ) ).generic_string( );
	}


	FilePath FilePath::absolute( ) const
	{
        return std::filesystem::absolute( to_path( ) ).generic_string( );
	}


	FilePath FilePath::canonical( ) const
	{
        return std::filesystem::canonical( to_path( ) ).generic_string( );
	}


	int FilePath::compare( const FilePath & lhs, const FilePath & rhs )
	{
		return Memory::compare( lhs.path, rhs.path );
	}


	FilePath & FilePath::assign( const FilePath & copy )
	{
		path = copy.path;
		return *this;
	}


	FilePath & FilePath::assign( FilePath && move )
	{
		path = std::move( move.path );
		return *this;
	}

	
	FilePath & FilePath::operator=( const FilePath & copy )
	{
		path = copy.path;
		return *this;
	}


	FilePath & FilePath::operator=( FilePath && move )
	{
		path = std::move( move.path );
		return *this;
	}


	FilePath & FilePath::append( const FilePath & other )
	{
        path += '/';
        path += other.path;
		return *this;
	}


	FilePath & FilePath::operator/=( const FilePath & other )
	{
		return append( other );
	}


	FilePath & FilePath::concat( const FilePath & other )
	{
        path += other.path;
		return *this;
	}


	FilePath & FilePath::operator+=( const FilePath & other )
	{
		return concat( other );
	}


	void FilePath::clear( )
	{
		path.clear( );
	}


	//  FilePath{ "c:/dir/file.ext1.ext2" }.parent() == "c:/dir"
	FilePath FilePath::parent( ) const
	{
		size_t pos = path.findLastOf( "/" );
		return ( pos != std::string::npos )
            ? FilePath{ Memory{ path }.substr( 0, pos ) }
			: FilePath{};
	}


	//  FilePath{ "c:/dir/file.ext1.ext2" }.filename() == "file.ext1.ext2"
	Memory FilePath::filename( ) const
	{
		size_t pos = path.findLastOf( "/" );
		return ( pos != std::string::npos )
			? Memory{ path }.substr( pos + 1 )
			: Memory{ path };
	}

	//  FilePath{ "c:/dir/file.ext1.ext2" }.name() == "file"
	//  FilePath{ "c:/dir/file.ext1.ext2" }.name(true) == "file.ext1"
	Memory FilePath::stem( bool shortExtension ) const
	{
		Memory name = filename( );
		size_t pos = shortExtension
			? name.rfind( '.' )
			: name.find( '.' );
		return ( pos != std::string::npos )
			? name.substr( 0, pos )
			: name;
	}


	//  FilePath{ "c:/dir/file.ext1.ext2" }.extension() == "ext1.ext2"
	//  FilePath{ "c:/dir/file.ext1.ext2" }.extension(true) == "ext2"
	Memory FilePath::extension( bool shortExtension ) const
	{
		Memory name = filename( );
		size_t pos = shortExtension
			? name.rfind( '.' )
			: name.find( '.' );
		return ( pos != std::string::npos )
			? name.substr( pos + 1 )
			: Memory::Empty;
	}


    std::filesystem::path FilePath::to_path( ) const
    {
        return std::filesystem::path{ (const char8_t *)path.begin( ), (const char8_t *)path.end( ) };
    }


	std::string FilePath::toString( bool nativeSeperator ) const
	{
		if ( !nativeSeperator )
			{ return path.data; }

		return Memory{ path }.replaceAll( "/", "\\" );
	}


	std::wstring FilePath::toWindows( ) const
	{
		return toUtf16( toString( true ) );
	}

}


cpp::FilePath operator/( const cpp::FilePath & lhs, const cpp::Memory & rhs )
{
	cpp::FilePath result = lhs;
	return result /= rhs;
}


cpp::FilePath operator/( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	cpp::FilePath result = lhs;
	return result /= rhs;
}


bool operator==( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	return cpp::FilePath::compare( lhs, rhs ) == 0;
}


bool operator!=( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	return cpp::FilePath::compare( lhs, rhs ) != 0;
}


bool operator<( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	return cpp::FilePath::compare( lhs, rhs ) < 0;
}


bool operator<=( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	return cpp::FilePath::compare( lhs, rhs ) <= 0;
}


bool operator>( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	return cpp::FilePath::compare( lhs, rhs ) > 0;
}

bool operator>=( const cpp::FilePath & lhs, const cpp::FilePath & rhs )
{
	return cpp::FilePath::compare( lhs, rhs ) >= 0;
}

#else



#include "../meta/Test.h"
#include "../../cpp/file/FilePath.h"



TEST_CASE( "FilePath" )
{
    using namespace cpp;

    const char * path1 = "something";
    std::string path2 = "something";
    String path3 = "something";

    FilePath f1{ "something" };
    FilePath f2{ path1 };
    FilePath f3{ path2 };
    FilePath f4{ path3 };
    FilePath f5{ std::string{ path1 } };
    FilePath f6 = "something";
    FilePath f7 = path1;
    FilePath f8 = path2;
    FilePath f9 = path3;

    f6 = "something";
    f7 = path1;
    f8 = path2;
    f9 = path3;

    REQUIRE( f1 == path1 );
    REQUIRE( f2 == path1 );
    REQUIRE( f3 == path1 );
    REQUIRE( f4 == path1 );
    REQUIRE( f5 == path1 );
    REQUIRE( f6 == path1 );
    REQUIRE( f7 == path1 );
    REQUIRE( f8 == path1 );
    REQUIRE( f9 == path1 );
}


#endif


#include "../../cpp/data/String.h"
#include "../../cpp/file/FilePath.h"

namespace cpp
{

	FilePath FilePath::currentPath( )
	{
		return std::filesystem::current_path( );
	}


	FilePath FilePath::tempPath( )
	{
		return std::filesystem::temp_directory_path( );
	}


	FilePath FilePath::tempFile( Memory prefix, Memory ext )
	{
		FilePath path = tempPath( );

		do
		{
			uint32_t rnd = (uint32_t)Program::rng( ).rand( );
			path = tempPath( ) / String::format( "%%.%", prefix, Integer::toHex( rnd, 8, 0, false, true, false ), ext );
		} while ( File::exists( path ) );

		return path;
	}


	FilePath::FilePath( )
	{

	}


	FilePath::FilePath( std::string path )
		: m_path( std::move( path ) )
	{

	}


	FilePath::FilePath( const std::filesystem::path & path )
		: m_path( path.generic_string( ) )
	{

	}


	FilePath::FilePath( const FilePath & copy )
		: m_path( copy.m_path )
	{

	}


	FilePath::FilePath( FilePath && move )
		: m_path( std::move( move.m_path) )
	{

	}


	bool FilePath::isEmpty( ) const
	{
		return m_path.empty( );
	}


	bool FilePath::notEmpty( ) const
	{
		return !m_path.empty( );
	}


	bool FilePath::operator!( ) const
	{
		return !m_path.empty( );
	}


	bool FilePath::isAbsolute( ) const
	{
		return std::filesystem::path{ m_path }.is_absolute( );
	}


	FilePath FilePath::normal( ) const
	{
		return std::filesystem::path{ m_path }.lexically_normal().generic_string( );
	}


	FilePath FilePath::relative( const FilePath & base ) const
	{
		return std::filesystem::path{ m_path }.lexically_relative( base.m_path ).generic_string( );
	}


	FilePath FilePath::absolute( ) const
	{
		return std::filesystem::absolute( std::filesystem::path{ m_path } ).generic_string( );
	}


	FilePath FilePath::canonical( ) const
	{
		return std::filesystem::canonical( std::filesystem::path{ m_path } ).generic_string( );
	}


	int FilePath::compare( const FilePath & lhs, const FilePath & rhs )
	{
		return strcmp( lhs.m_path.c_str( ), rhs.m_path.c_str( ) );
	}


	FilePath & FilePath::assign( std::string path )
	{
		m_path = std::move( path );
		return *this;
	}


	FilePath & FilePath::assign( const FilePath & copy )
	{
		m_path = copy.m_path;
		return *this;
	}


	FilePath & FilePath::assign( FilePath && move )
	{
		m_path = std::move( move.m_path );
		return *this;
	}

	
	FilePath & FilePath::operator=( std::string path )
	{
		m_path = std::move( path );
		return *this;
	}


	FilePath & FilePath::operator=( const FilePath & copy )
	{
		m_path = copy.m_path;
		return *this;
	}


	FilePath & FilePath::operator=( FilePath && move )
	{
		m_path = std::move( move.m_path );
		return *this;
	}


	FilePath & FilePath::append( Memory path )
	{
		m_path += '/';
		m_path.append( path.begin( ), path.end( ) );
		return *this;
	}


	FilePath & FilePath::operator/=( Memory path )
	{
		return append( path );
	}


	FilePath & FilePath::concat( Memory path )
	{
		m_path.append( path.begin( ), path.end( ) );
		return *this;
	}


	FilePath & FilePath::operator+=( Memory path )
	{
		return concat( path );
	}


	void FilePath::clear( )
	{
		m_path.clear( );
	}


	//  FilePath{ "c:/dir/file.ext1.ext2" }.parent() == "c:/dir"
	FilePath FilePath::parent( ) const
	{
		size_t pos = m_path.find_last_of( '/' );
		return ( pos != std::string::npos )
			? FilePath{ m_path.substr( 0, pos ) }
			: FilePath{};
	}


	//  FilePath{ "c:/dir/file.ext1.ext2" }.filename() == "file.ext1.ext2"
	Memory FilePath::filename( ) const
	{
		size_t pos = m_path.find_last_of( '/' );
		return ( pos != std::string::npos )
			? FilePath{ m_path.substr( pos + 1 ) }
			: m_path;
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


	FilePath::operator std::filesystem::path( ) const
	{
		return std::filesystem::path{ m_path };
	}


	FilePath::operator std::string( ) const
	{
		return m_path;
	}


	std::string FilePath::toString( bool nativeSeperator ) const
	{
		if ( !nativeSeperator )
			{ return m_path; }

		String path = m_path;
		path.replaceAll( "/", "\\" );
		return path;
	}

}
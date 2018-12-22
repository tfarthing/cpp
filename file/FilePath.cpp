#include <cctype>
#include <cpp/Integer.h>
#include <cpp/Program.h>
#include <cpp/io/file/FilePath.h>
#include <cpp/io/file/File.h>
#include <cpp/util/Path.h>
#include <cpp/util/Utf16.h>

namespace cpp
{

    FilePath FilePath::getTempPath( )
    {
        std::wstring tempPath( 1024, L'\0' );
        int len = GetTempPath( (DWORD)tempPath.size( ), (LPWSTR)tempPath.c_str( ) );
        if ( len > 0 )
            { tempPath.resize(len); }
        return FilePath{ toUtf8( tempPath ) };
    }

    FilePath FilePath::getTempFile( String prefix, String ext )
    {
        FilePath tempPath = getTempPath( );

        FilePath path;
        do
        {
            uint32_t rnd = (uint32_t)Program::rand( );
            path = FilePath{ tempPath, String::format( "%%.%", prefix, Integer::toHex(rnd, 8, 0, false, true, false), ext ) };
        } while ( File::exists( path ) );
        
        return path;
    }

    FilePath FilePath::getWorkingPath( )
    {
        return Program::workingPath( );
    }

    FilePath & FilePath::assign( const Memory & path )
    {
        //  determine if the specified path is an absolute path:
        //  * "/dir"
        //  * "c:/dir"
        //  * "//unc/share/dir"

        bool isAbsolutePath = false;
        if ( path.length( ) > 1 && ( path[0] == '/' || path[0] == '\\' ) )
            { isAbsolutePath = true; }
        else if ( path.length() > 2 && std::isalpha( path[0] ) && path[1] == ':' )
            { isAbsolutePath = true; }

        if ( isAbsolutePath )
        {
            m_filePath = path;
            //  normalize path separatorsto '/'
            m_filePath.replaceAll( "\\", "/" );

        }
        else
        {
            m_filePath.clear();
            append( path );
        }

        if ( !m_filePath.isEmpty() && m_filePath.at( m_filePath.length() - 1 ) == '/' )
            { m_filePath.resize( m_filePath.length( ) - 1 ); }

        return *this;
    }

    FilePath & FilePath::append( const Memory & child )
    {
        String childPath = child;

        //  path separators are normalized to '/'
        childPath.replaceAll( "\\", "/" );

        String parentPath = m_filePath.isEmpty() ? getWorkingPath().toString() : m_filePath;
        auto nodes = path_t::split( parentPath );
        auto childNodes = path_t::split( childPath );

        for ( Memory node : childNodes )
        {
            if ( node == ".." )
            {
                check<FilePath::Exception>( nodes.size( ) > 1, "Invalid file path: " + childPath );
                nodes.pop_back( );
            }
            else if ( node != "." && !node.isEmpty( ) )
            {
                nodes.push_back( node );
            }
        }

        m_filePath.clear( );
        bool initialNode = true;
        for ( auto node : nodes )
        {
            if ( !initialNode )
                { m_filePath += path_t::Separator; }
            m_filePath += node;
            initialNode = false;
        }

        return *this;
    }

    FilePath FilePath::parent( ) const
    { 
        return path_t::parent( m_filePath ); 
    }

    Memory FilePath::filename( ) const
    {
        return path_t::element( m_filePath );
    }

    Memory FilePath::name( bool shortExtension ) const
    {
        Memory filename = FilePath::filename( );
        size_t pos = shortExtension ? filename.find_last_of( "." ) : filename.find_first_of( "." );
        return pos == Memory::npos ? filename : filename.substr( 0, pos );
    }

    Memory FilePath::extension( bool shortExtension ) const
    {
        Memory filename = FilePath::filename( );
        size_t pos = shortExtension ? filename.find_last_of( "." ) : filename.find_first_of( "." );
        return pos == Memory::npos ? Memory::Empty : filename.substr( pos + 1 );
    }

    String FilePath::toString( char separator ) const
    {
        if ( separator == '/' )
            { return m_filePath; }

        String path = m_filePath;
        path.replaceAll( "/", Memory::ofValue( separator ) );
        return path;
    }

    String FilePath::toString( const cpp::FilePath & relativePath, char separator ) const
    {
        String path = m_filePath;

        String workingPath = relativePath.m_filePath + "/";
        if ( workingPath == Memory{ path }.substr( 0, workingPath.length( ) ) )
            { path = path.substr( workingPath.length( ) ); }
        if ( separator != '/' )
            { path.replaceAll( "/", Memory::ofValue( separator ) ); }
        return path;
    }

}


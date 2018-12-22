#pragma once

#include <cpp/Memory.h>
#include <cpp/String.h>
#include <cpp/Exception.h>
#include <cpp/util/Comparable.h>
#include <cpp/util/Path.h>

namespace cpp
{
    class FilePath
        : public cpp::Comparable<FilePath>
    {
    public:
        using path_t = Path<'/'>;
        using Array = std::vector<FilePath>;

        static FilePath getTempPath( );
        static FilePath getTempFile( String prefix = "temp", String ext = "tmp" );
        static FilePath getWorkingPath( );
        
        FilePath( )
            {}
        FilePath( const char * path )
            { assign( Memory{ path } ); }
        FilePath( const Memory & path )
            { assign( path ); }
        FilePath( const String & path )
            { assign( Memory{ path } ); }
        FilePath( const FilePath & copy )
            : m_filePath(copy.m_filePath) { }
        FilePath( FilePath && move )
            : m_filePath( std::move(move.m_filePath) ) { }
        FilePath( const FilePath & parent, const Memory & child )
            { assign( parent ).append(child); }

        bool isEmpty() const
            { return m_filePath.isEmpty(); }
        bool operator!( ) const
            { return isEmpty(); }

        bool isUnique( ) const
            { return m_filePath.find_first_of("*") != String::npos; }

        static int compare( const FilePath & lhs, const FilePath & rhs )
            { return strcmp( lhs.m_filePath.c_str( ), rhs.m_filePath.c_str( ) ); }

        Memory::Array nodes( ) const
            { return path_t::split( m_filePath); }

        FilePath withExtension( Memory ext, bool shortExtension = false )
            { return FilePath{ parent(), name(shortExtension) + "." + ext }; }

        //  FilePath::assign()
        //
        //  Description: This function will convert the specified path to its
        //      absolute form.  If the path is relative then it will be appended 
        //      to the Program::workingPath().  The following rules apply:
        //  
        //      * Empty nodes or nodes equal to "." are dropped.
        //      * Nodes equal to ".." back up one directory.
        //
        FilePath & assign( const Memory & path );
        FilePath & assign( const FilePath & copy )
            { m_filePath = copy.m_filePath; return *this; }
        FilePath & assign( FilePath && move )
            { m_filePath = std::move( move.m_filePath ); return *this; }

        FilePath & operator=( const Memory & path )
            { return assign( path ); }
        FilePath & operator=( const FilePath & copy )
            { return assign( copy ); }
        FilePath & operator=( FilePath && move )
            { return assign( std::move( move ) ); }

        FilePath & append( const Memory & child );
        FilePath & operator+=( const Memory & child )
            { return append( child ); }

        void clear()
            { m_filePath.clear(); }

        //  FilePath{ "c:/dir/file.ext1.ext2" }.parent() == "c:/dir"
        FilePath parent( ) const;
        //  FilePath{ "c:/dir/file.ext1.ext2" }.filename() == "file.ext1.ext2"
        Memory filename( ) const;
        //  FilePath{ "c:/dir/file.ext1.ext2" }.name() == "file"
        //  FilePath{ "c:/dir/file.ext1.ext2" }.name(true) == "file.ext1"
        Memory name( bool shortExtension = false ) const;
        //  FilePath{ "c:/dir/file.ext1.ext2" }.extension() == "ext1.ext2"
        //  FilePath{ "c:/dir/file.ext1.ext2" }.extension(true) == "ext2"
        Memory extension( bool shortExtension = false ) const;

        String toString( char separator = '/' ) const;
        String toString( const FilePath & relativePath, char separator = '/' ) const;

        static Array toArray( const Memory::Array & value )
            { return Array{ value.begin(), value.end() }; }
        static Array toArray( const String::Array & value )
            { return Array{ value.begin(), value.end() }; }

    public:
        class Exception
            : cpp::Exception
        {
        public:
            Exception( String message )
                : cpp::Exception( std::move( message ) ) { }
        };

    private:
        String m_filePath;
    };

}

inline cpp::FilePath operator+( const cpp::FilePath & lhs, const cpp::Memory & rhs )
    { return cpp::FilePath{ lhs, rhs }; }

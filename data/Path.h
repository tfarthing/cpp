#pragma once

#include <cpp/String.h>
#include <cpp/Exception.h>

namespace cpp
{

    template<char separator>
    struct Path
    {
        static const char Separator = separator;

        static String append( const Memory & parent, const Memory & element );

        static int depth( const Memory & path );
        static Memory node( const Memory & path, int index );
        static bool contains( const Memory & path, const Memory & name );

        static Memory parent( const Memory & path );
        static Memory parent( const Memory & path, int parentDepth );

        static Memory element( const Memory & path );
        static Memory element( const Memory & path, int parentDepth );

        static Memory::Array split( const Memory & path );

    };

    template<char separator>
    String Path<separator>::append( const Memory & parent, const Memory & element )
    {
        return ( parent.isEmpty( ) ) ? element : String{ parent } + separator + element;
    }

    template<char separator>
    int Path<separator>::depth( const Memory &path )
    {
        size_t offset = 0;
        size_t next = Memory::npos;
        int nodeIndex = 0;
        while ( offset < path.length( ) && ( next = path.find_first_of( Memory::ofValue( separator ), offset ) ) != Memory::npos )
            { nodeIndex++; offset = next + 1; }
        if ( offset == path.length( ) || next == Memory::npos )
            { nodeIndex++; }
        return nodeIndex;
    }

    template<char separator>
    Memory Path<separator>::node( const Memory & path, int index )
    {
        size_t offset = 0;
        size_t next = Memory::npos;
        int nodeIndex = 0;
        while ( offset < path.length( ) && ( next = path.find_first_of( Memory::ofValue( separator ), offset ) ) != Memory::npos && nodeIndex < index )
            { nodeIndex++; offset = next + 1; }
        if ( nodeIndex != index )
            { throw Exception( "Node index out-of-bounds of Path" ); }
        return ( next != Memory::npos ) ? path.substr( offset, next - offset ) : path.substr( offset );
    }

    template<char separator>
    bool Path<separator>::contains( const Memory & path, const Memory & name )
    {
        return path == name.substr( 0, path.length( ) ) && 
            ( path.isEmpty() || name.length() == path.length() || name[path.length( )] == separator ) ;
    }

    template<char separator>
    Memory Path<separator>::parent( const Memory & path )
    {
        size_t offset = path.find_last_of( Memory::ofValue( separator ) );
        return ( offset != Memory::npos ) ? path.substr( 0, offset ) : "";
    }

    template<char separator>
    Memory Path<separator>::parent( const Memory & path, int parentDepth )
    {
        size_t offset = 0;
        for ( int i = 0; i<parentDepth; i++ )
        {
            offset = path.find_first_of( Memory::ofValue( separator ), i ? offset + 1 : offset );
            if ( offset == Memory::npos )
                { return path; }
        }
        return path.substr( 0, offset );
    }

    template<char separator>
    Memory Path<separator>::element( const Memory & path )
    {
        size_t offset = path.find_last_of( Memory::ofValue( separator ) );
        return offset != Memory::npos ? path.substr( offset + 1 ) : path;
    }

    template<char separator>
    Memory Path<separator>::element( const Memory & path, int parentDepth )
    {
        size_t offset = 0;
        for ( int i = 0; i<parentDepth; i++ )
        {
            offset = path.find_first_of( Memory::ofValue( separator ), i ? offset + 1 : offset );
            if ( offset == Memory::npos )
               { return ""; }
        }
        return path.substr( offset + 1 );
    }

    template<char separator>
    Memory::Array Path<separator>::split( const Memory & path )
    {
        return path.split( Memory::ofValue(Separator), Memory::whitespace, false );
    }


}
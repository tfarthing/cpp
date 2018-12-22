#pragma once

#include <set>
#include <map>
#include <vector>
#include <cpp/String.h>
#include <cpp/util/Path.h>

namespace cpp
{

    class Properties
    {
    public:
        typedef std::map<String, String> Map;
        typedef std::set<String> KeySet;
        typedef Path<'.'> Path;

        static Properties fromText( const Memory & text );
        static Properties fromTextArray( const Memory::Array & textArray );
        static Properties fromTextArray( const String::Array & textArray );
        static Properties fromFiles( const Memory::Array & files );
        static Properties fromFiles( const String::Array & files );

        Properties( );

        Properties & add( Properties properties, const Memory & prefix = "" );
        Properties & addText( const Memory & text, const Memory & prefix = "" );
        Properties & addTextArray( const Memory::Array & textArray, const Memory & prefix = "" );
        Properties & addTextArray( const String::Array & textArray, const Memory & prefix = "" );
        Properties & addFiles( const Memory::Array & filenames, const Memory & prefix = "" );
        Properties & addFiles( const String::Array & filenames, const Memory & prefix = "" );

        String findKey( const Memory & key, const Memory::Array & path ) const;

        bool has( const Memory & key ) const;
        bool has( const Memory & key, const Memory::Array & path ) const;
        Memory get( const Memory & key ) const;
        Memory get( const Memory & key, const Memory::Array & path ) const;
        int64_t getInteger( const Memory & key ) const;
        int64_t getInteger( const Memory & key, const Memory::Array & path ) const;
        f64_t getFloat( const Memory & key ) const;
        f64_t getFloat( const Memory & key, const Memory::Array & path ) const;
        String expand( const Memory & key ) const;
        String expand( const Memory & key, const Memory::Array & path ) const;

        Memory operator[](const Memory & key) const;

        Properties extract( const Memory & category ) const;
        Properties extract( const Memory::Array & categoryList ) const;
        Properties extract( const String::Array & categoryList ) const;

        KeySet keysAt( const Memory & category ) const;

        void set( String key, String value );

        void remove( const Memory & key );
        void removeCategory( const Memory & category );

        size_t size( ) const;

        String toString( ) const;
        size_t findKeyLength( ) const;

        const Map & getMap() const
            { return m_map; }

    protected:
        void addTextEntry( const Memory & entry, const Memory & prefix );
        String expand( const Memory & key, const Memory & value ) const;

    protected:
        Map m_map;
    };

}
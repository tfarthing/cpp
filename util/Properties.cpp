#ifndef TEST

#include <cpp/Integer.h>
#include <cpp/Float.h>
#include <cpp/util/Properties.h>
#include <cpp/util/Path.h>

namespace cpp
{

    Properties Properties::fromText( const Memory & text )
        { return Properties{ }.addText( text ); }

    Properties Properties::fromTextArray( const Memory::Array & textArray )
        { return Properties{ }.addTextArray( textArray ); }

    Properties Properties::fromTextArray( const String::Array & textArray )
        { return Properties{}.addTextArray( textArray ); }

    Properties Properties::fromFiles( const Memory::Array & files )
        { return Properties{ }.addFiles( files ); }

    Properties Properties::fromFiles( const String::Array & files )
        { return Properties{}.addFiles( files ); }

    Properties::Properties( )
        { }

    Properties & Properties::add( Properties properties, const Memory & prefix )
    {
        for ( auto entry : properties.m_map )
        {
            set( Path::append( prefix, entry.first ), entry.second );
        }
        return *this;
    }

    void Properties::addTextEntry( const Memory & entry, const Memory & prefix )
    {
        const char * key_value_delimiter = " \t\r\n:=";
        if ( entry[0] == '#' || ( entry[0] == '/' && entry[1] == '/' ) )
            { return; }

        size_t valueBegin = String::npos;
        size_t keyEnd = entry.find_first_of( key_value_delimiter );
        if ( keyEnd != String::npos )
            { valueBegin = entry.find_first_not_of( key_value_delimiter, keyEnd ); }
        String key = entry.substr( 0, keyEnd ).trim( );
        String value = ( valueBegin != String::npos ) ? entry.substr( valueBegin ).trim( "\r\n" ) : "";

        if ( key.isEmpty( ) )
            { }
        else if ( key != "[remove]" )
            { set( Path::append(prefix, key), value ); }
        else if ( value.isEmpty( ) )
            { m_map.clear( ); }
        else
            { m_map.erase( value ); }
    }

    Properties & Properties::addText( const Memory & text, const Memory & prefix )
    {
        size_t offset = 0;
        size_t next = 0;
        while ( offset != String::npos )
        {
            next = text.find_first_of( "\r\n", offset );
            String entry = ( next == String::npos ) ? text.substr( offset ) : text.substr( offset, next - offset );
            offset = ( next == String::npos ) ? next : next + 1;

            addTextEntry( entry, prefix );
        }
        return *this;
    }

    Properties & Properties::addTextArray( const Memory::Array & textArray, const Memory & prefix )
    {
        for ( auto & entry : textArray )
            { addTextEntry( entry, prefix ); }
        return *this;
    }

    Properties & Properties::addTextArray( const String::Array & textArray, const Memory & prefix )
    {
        return addTextArray( Memory::Array{ textArray.begin(), textArray.end() } );
    }

    Properties & Properties::addFiles( const Memory::Array & filenames, const Memory & prefix )
    {
        for ( auto & filename : filenames )
        {
            //addText( File::toString( filename ), prefix );
            /*
            StdFile file;
            File::result_t result = file.open( filename, File::mode_read, true );
            if ( result == File::result_success )
            {
            while ( !file.eof( ) )
            {
            string line = file.gets( );
            if ( line.substr( 0, 8 ) == "#include" )
            {
            properties.add( Properties::read( File::pathParseParent( filename ) + line.substr( 8 ).trim( " \t\r\n\"<>" ) ) );
            }
            else
            {
            properties.add( line );
            }
            }
            file.close( );
            }
            */
        }
        return *this;
    }

    Properties & Properties::addFiles( const String::Array & filenames, const Memory & prefix )
    {
        return addFiles( Memory::Array{ filenames.begin( ), filenames.end( ) }, prefix );
    }

    String Properties::findKey( const Memory & key, const Memory::Array & path ) const
    {
        for ( auto & category : path )
        {
            String categoryKey = Path::append( category, key );
            if ( has( categoryKey ) )
                { return categoryKey; }
        }
        return key;
    }

    bool Properties::has( const Memory & key ) const
    {
        return m_map.count( key ) > 0;
    }

    bool Properties::has( const Memory & key, const Memory::Array & path ) const
    {
        return has( findKey( key, path ) );
    }

    Memory Properties::get( const Memory & key ) const
    {
        auto itr = m_map.find( key );
        return ( itr != m_map.end( ) ) ? Memory{ itr->second } : Memory::Null;
    }

    Memory Properties::get( const Memory & key, const Memory::Array & path ) const
    {
        return get( findKey( key, path ) );
    }

    int64_t Properties::getInteger( const Memory & key ) const
    {
        auto value = get( key );
        return value ? cpp::Integer::parse( value ) : 0;
    }

    int64_t Properties::getInteger( const Memory & key, const Memory::Array & path ) const
    {
        auto value = get( key, path );
        return value ? cpp::Integer::parse( value ) : 0;
    }

    f64_t Properties::getFloat( const Memory & key ) const
    {
        auto value = get( key );
        return value ? cpp::Float::parse( value ) : 0;
    }

    f64_t Properties::getFloat( const Memory & key, const Memory::Array & path ) const
    {
        auto value = get( key, path );
        return value ? cpp::Float::parse( value ) : 0;
    }

    String Properties::expand( const Memory & key ) const
    {
        auto itr = m_map.find( key );
        return ( itr != m_map.end( ) ) ? expand(key, itr->second) : "";
    }

    String Properties::expand( const Memory & key, const Memory::Array & path ) const
    {
        return expand( findKey( key, path ) );
    }

    Memory Properties::operator[]( const Memory & key ) const
    {
        return get( key );
    }

    void Properties::set( String key, String value )
    {
        m_map[ std::move( key ) ] = std::move( value );
    }

    void Properties::remove( const Memory & key )
    {
        m_map.erase( key );
    }

    void Properties::removeCategory( const Memory & category )
    {
        Properties keys = extract( category );
        for ( auto itr : keys.m_map )
            { remove( itr.first ); }
    }

    Properties Properties::extract( const Memory & category ) const
    {
        Properties results;

        size_t prefixLen = category.length( );
        auto itr = m_map.lower_bound( category );
        while ( itr != m_map.end( ) )
        {
            const String & key = itr->first;
            if ( key.substr( 0, prefixLen ) != category )
                { break; }
            results.set( (prefixLen > 0) ? key.substr( prefixLen + 1 ) : key, expand( key, itr->second ) );
            itr++;
        }

        return results;
    }

    Properties Properties::extract( const Memory::Array & categoryList ) const
    {
        Properties result;
        for ( auto & category : categoryList )
            { result.add( std::move( extract( category ) ) ); }
        return result;
    }

    Properties::KeySet Properties::keysAt( const Memory & category ) const
    {
        Properties::KeySet results;

        size_t prefixLen = category.length( );
        auto itr = m_map.lower_bound( category );
        while ( itr != m_map.end( ) )
        {
            const String & key = itr->first;
            if ( itr == m_map.end( ) || key.substr( 0, prefixLen ) != category )
                { break; }
            if ( key.length() <= prefixLen || ( category.length( ) != 0 && key.at( prefixLen ) != '.' ) )
                { continue; }
            
            String baseKey = category.length( ) == 0 ? Path::node(key, 0) : Path::node( key.substr( prefixLen + 1 ), 0 );
            results.insert( baseKey );
            itr++;
        }

        return results;
    }

    String Properties::expand( const Memory & key, const Memory & value ) const
    {
        String result = value;
        size_t openBrace = value.find_first_of( "{" );
        if ( openBrace != String::npos )
        {
            size_t closeBrace = openBrace;
            while ( closeBrace == openBrace )
            {
                closeBrace = value.find_first_of( "{}", openBrace + 1 );
                if ( closeBrace != String::npos && value[closeBrace] == '{' )
                    { openBrace = closeBrace; }
            }
            if ( openBrace != closeBrace && closeBrace != String::npos )
            {
                String token = value.substr( openBrace + 1, closeBrace - openBrace - 1 );
                String keypath = key;
                while ( true )
                {
                    String subkey = Path::append( keypath, token );
                    auto itr = m_map.find( subkey );
                    if ( itr != m_map.end( ) && itr->second != value )
                    {
                        result.replace( openBrace, closeBrace - openBrace + 1, expand( subkey, itr->second ) );
                        result = expand( key, result );
                        break;
                    }
                    if ( keypath.empty( ) )
                        { break; }
                    keypath = Path::parent( keypath );
                };

            }
        }
        return result;
    }

    size_t Properties::size( ) const
    {
        return m_map.size( );
    }

    String Properties::toString( ) const
    {
        String result;
        String lastGroup;
        int lastGroupLines = 0;
        String format = String::printf( "%%-%ds\t%%s\n", findKeyLength( ) + 8 );
        for ( auto entry : m_map )
        {
            const String & key = entry.first;
            const String & value = entry.second;
            if ( !key.empty( ) )
            {
                String group = Path::parent( key );
                if ( group != lastGroup )
                {
                    if ( lastGroupLines > 1 )
                    {
                        result += "\n";
                        lastGroup = group;
                    }
                    lastGroupLines = 0;
                }
                lastGroupLines++;

                result.reserve( result.length( ) + 8 + format.length( ) + key.length( ) + value.length( ) );
                result += String::printf( format.c_str( ), key.c_str( ), value.c_str( ) );
            }
        }
        return result;
    }

    size_t Properties::findKeyLength( ) const
    {
        size_t len = 0;
        for ( auto entry : m_map )
        {
            if ( entry.first.length( ) > len )
                { len = entry.first.length( ); }
        }
        return len;
    }

}

#else

#include <cpp/util/Properties.h>
#include <cpp/meta/Unittest.h>

SUITE( Properties )
{
    using namespace cpp;

    TEST( fromText )
    {
        auto properties = Properties::fromText( 
            "a=1\n"
            "b: 2\n"
            "c 3\n" );
        CHECK( properties.size() == 3 );
        CHECK( properties["a"] == String( "1" ) );
        CHECK( properties["b"] == String( "2" ) );
        CHECK( properties["c"] == String( "3" ) );
    }

    TEST( fromTextArray )
    {
        auto properties = Properties::fromTextArray( Memory::Array{ "a=1", "b: 2", "c 3" } );
        CHECK( properties.size( ) == 3 );
        CHECK( properties["a"] == "1" );
        CHECK( properties["b"] == "2" );
        CHECK( properties["c"] == "3" );
    }

    TEST( add )
    {
        auto properties = Properties{ };

        properties.add( Properties::fromTextArray( Memory::Array{ "a=1", "b=2" } ) );
        CHECK( properties.size( ) == 2 );

        properties.addText( "b=3\nc=4" );
        CHECK( properties.size( ) == 3 );
        CHECK( properties["b"] == "3" );

        properties.addTextArray( Memory::Array{ "x=5", "y=6" } );
        CHECK( properties.size( ) == 5 );

        properties.addText( "[remove]: c" );
        CHECK( properties.size( ) == 4 );
        CHECK( properties["c"] == Memory::Null );

        properties.addText( "[remove]" );
        CHECK( properties.size( ) == 0 );

        properties.addText( "a=1\nb=2", "parent.dir" );
        CHECK( properties.size( ) == 2 );
        CHECK( properties["parent.dir.b"] == "2" );
    }

    TEST( expand1 )
    {
        Properties map;
        map.set( "platform.host.0", "localhost1");
        map.set( "platform.host.1", "localhost2");
        map.set( "platform.config_addr", "{srv_config.0}, {srv_config.1}");
        map.set( "platform.srv_config.0", "{host.0}:{port}");
        map.set( "platform.srv_config.0.port", "55000");
        map.set( "platform.srv_config.1", "{host.1}:{port}");
        map.set( "platform.srv_config.1.port", "55001");
        map.set( "platform.connector", "{srv_config.0}, {srv_config.1}");
        map.set( "space.srv_proxy.ext.all.connector", "{platform.config_addr}");

        CHECK( map.expand( "platform.config_addr" ) == "localhost1:55000, localhost2:55001" );
        CHECK( map.expand( "space.srv_proxy.ext.all.connector" ) == "localhost1:55000, localhost2:55001" );

    }

    TEST( expand2 )
    {
        Properties properties = Properties::fromTextArray( String{ "red=1 green=2 dir.red=3 dir.green=4+{red}" }.split( " " ) );

        CHECK( properties.expand( "green" ) == "2" );

        CHECK( properties.get( "red", Memory::Array{ "dir", "other" } ) == "3" );
        CHECK( properties.get( "green", Memory::Array{ "dir", "other" } ) == "4+{red}" );
        CHECK( properties.expand( "green", Memory::Array{ "dir", "other" } ) == "4+3" );

        properties.remove( "dir.red" );
        CHECK( properties.expand( "green", Memory::Array{ "dir", "other" } ) == "4+1" );
    }

}

#endif

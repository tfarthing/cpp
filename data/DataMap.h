#pragma once

#include <map>

#include <cpp/String.h>
#include <cpp/chrono/DateTime.h>

namespace cpp
{

    class StringMap;

    class DataMap
        : public std::map<Memory, Memory>
    {
    public:
        DataMap( );
        DataMap( DataMap && move );
        DataMap( const DataMap & copy );
        DataMap( const StringMap & copy );
        DataMap( std::initializer_list<std::map<Memory, Memory>::value_type> init );

        DataMap & operator=( DataMap && move );
        DataMap & operator=( const DataMap & copy );
        DataMap & operator=( const StringMap & copy );

        Memory operator[]( Memory key ) const;

        bool isEmpty( ) const;
        bool notEmpty( ) const;

        Memory get( Memory key ) const;
        void set( Memory key, Memory value );
        void remove( Memory key );
    };

    class StringMap
        : public std::map<String, String>
    {
    public:
        StringMap( );

        StringMap( StringMap && move );
        StringMap( const StringMap & copy );
        StringMap( const DataMap & copy );
        StringMap( std::initializer_list<std::map<String, String>::value_type> init );

        StringMap & operator=( StringMap && move );
        StringMap & operator=( const StringMap & copy );
        StringMap & operator=( const DataMap && copy );

        Memory operator[]( Memory key ) const;

        bool isEmpty( ) const;
        bool notEmpty( ) const;

        Memory get( Memory key ) const;
        void set( Memory key, Memory value );
        void remove( Memory key );
    };

    class RecordMap
        : public std::map<cpp::String, StringMap>
    {
    public:
        RecordMap( );
        RecordMap( RecordMap && move );
        RecordMap( const RecordMap & copy );
        RecordMap( const DataMap & copy );

        RecordMap & operator=( RecordMap && move );
        RecordMap & operator=( const RecordMap & copy );
        RecordMap & operator=( const DataMap & copy );

        RecordMap & operator+=( const DataMap & datamap );

        bool isEmpty( ) const;
        bool notEmpty( ) const;

        uint64_t getUint( Memory key ) const;
        uint64_t getHex( Memory key ) const;
        int64_t getInt( Memory key ) const;
        f64_t getFloat( Memory key ) const;
        bool getBool( Memory key ) const;
        DateTime getTime( Memory key ) const;

        void setUint( Memory key, uint64_t value );
        void setHex( Memory key, uint64_t value );
        void setInt( Memory key, int64_t value );
        void setFloat( Memory key, f64_t value );
        void setBool( Memory key, bool value );
        void setTime( Memory key, DateTime value );

        Memory get( Memory key ) const;
        Memory get( Memory recordKey, Memory fieldKey ) const;
        void set( Memory key, Memory value );
        void set( Memory recordKey, Memory fieldKey, Memory value );
        void remove( Memory key );
        void remove( Memory recordKey, Memory fieldKey );

        DataMap getRecord( Memory recordKey ) const;
        void setRecord( Memory recordKey, DataMap data );
        void removeRecord( Memory recordKey );

        static std::pair<Memory, Memory> splitKey( Memory key );
    };



    inline DataMap::DataMap( )
        : std::map<Memory, Memory>( ) {}
    inline DataMap::DataMap( DataMap && move )
        : std::map<Memory, Memory>( std::move( move ) ) {}
    inline DataMap::DataMap( const DataMap & copy )
        : std::map<Memory, Memory>( copy ) {}
    inline DataMap::DataMap( const StringMap & copy )
        : std::map<Memory, Memory>( ) { for ( auto & itr : copy ) { insert_or_assign( itr.first, itr.second ); } }
    inline DataMap::DataMap( std::initializer_list<std::map<Memory, Memory>::value_type> init )
        : std::map<Memory, Memory>( init ) {}

    inline DataMap & DataMap::operator=( DataMap && move )
        { std::map<Memory, Memory>::operator=( std::move( move ) ); return *this; }
    inline DataMap & DataMap::operator=( const DataMap & copy )
        { std::map<Memory, Memory>::operator=( copy ); return *this; }
    inline DataMap & DataMap::operator=( const StringMap & copy )
        { for ( auto itr : copy ) { insert_or_assign( itr.first, itr.second ); } return *this; }

    inline Memory DataMap::operator[]( Memory key ) const
        { return get( key ); }

    inline bool DataMap::isEmpty( ) const
        { return empty( ); }
    inline bool DataMap::notEmpty( ) const
        { return !empty( ); }

    inline Memory DataMap::get( Memory key ) const
        { auto & itr = find( key ); return itr != end( ) ? itr->second : Memory::Null; }
    inline void DataMap::set( Memory key, Memory value )
        { insert_or_assign( key, value ); }
    inline void DataMap::remove( Memory key )
        { erase( key ); }



    inline StringMap::StringMap( )
        : std::map<String, String>( ) {}
    inline StringMap::StringMap( StringMap && move )
        : std::map<String, String>( std::move( move ) ) {}
    inline StringMap::StringMap( const StringMap & copy )
        : std::map<String, String>( copy ) {}
    inline StringMap::StringMap( const DataMap & copy )
        : std::map<String, String>( ) { for ( auto & itr : copy ) { insert_or_assign( itr.first, itr.second ); } }
    inline StringMap::StringMap( std::initializer_list<std::map<String, String>::value_type> init )
        : std::map<String, String>( init ) {}

    inline StringMap & StringMap::operator=( StringMap && move )
        { std::map<String, String>::operator=( std::move( move ) ); return *this; }
    inline StringMap & StringMap::operator=( const StringMap & copy )
        { std::map<String, String>::operator=( copy );  return *this;}
    inline StringMap & StringMap::operator=( const DataMap && copy )
        { for ( auto itr : copy ) { insert_or_assign( itr.first, itr.second ); } return *this; }

    inline Memory StringMap::operator[]( Memory key ) const
        { return get( key ); }

    inline bool StringMap::isEmpty( ) const
        { return empty( ); }
    inline bool StringMap::notEmpty( ) const
        { return !empty( ); }

    inline Memory StringMap::get( Memory key ) const
        { auto & itr = find( key ); return itr != end( ) ? itr->second : Memory::Null; }
    inline void StringMap::set( Memory key, Memory value )
        { insert_or_assign( key, value ); }
    inline void StringMap::remove( Memory key )
        { erase( key ); }



    inline RecordMap::RecordMap( )
        : std::map<cpp::String, StringMap>( ) {}
    inline RecordMap::RecordMap( RecordMap && move )
        : std::map<cpp::String, StringMap>( std::move(move) ) {}
    inline RecordMap::RecordMap( const RecordMap & copy )
        : std::map<cpp::String, StringMap>( copy ) {}
    inline RecordMap::RecordMap( const DataMap & copy )
        : std::map<cpp::String, StringMap>( ) { operator+=( copy ); }

    inline RecordMap & RecordMap::operator=( RecordMap && move )
        { std::map<cpp::String, StringMap>::operator=( std::move( move ) ); return *this; }
    inline RecordMap & RecordMap::operator=( const RecordMap & copy )
        { std::map<cpp::String, StringMap>::operator=( copy ); return *this; }
    inline RecordMap & RecordMap::operator=( const DataMap & map )
        { clear(); return operator+=(map); }

    inline RecordMap & RecordMap::operator+=( const DataMap & map )
        { for ( auto & itr : map ) { set( itr.first, itr.second ); } return *this; }

    inline bool RecordMap::isEmpty( ) const
        { return empty( ); }
    inline bool RecordMap::notEmpty( ) const
        { return !empty( ); }

    inline Memory RecordMap::get( Memory key ) const
        { auto keys = splitKey( key ); return get( keys.first, keys.second ); }
    inline Memory RecordMap::get( Memory recordKey, Memory fieldKey ) const
        { auto & itr = find( recordKey ); return itr != end( ) ? itr->second.get( fieldKey ) : Memory::Null; }
    inline void RecordMap::set( Memory key, Memory value )
        { auto keys = splitKey( key ); set( keys.first, keys.second, value ); }
    inline void RecordMap::set( Memory recordKey, Memory fieldKey, Memory value )
        { operator[]( recordKey ).insert_or_assign( fieldKey, value ); }
    inline void RecordMap::remove( Memory key )
        { auto keys = splitKey( key ); remove( keys.first, keys.second ); }
    inline void RecordMap::remove( Memory recordKey, Memory fieldKey )
        { if ( auto itr = find( recordKey ); itr != end( ) ) { itr->second.erase( fieldKey ); } }

    inline DataMap RecordMap::getRecord( Memory recordKey ) const
        { auto itr = find( recordKey ); return itr != end( ) ? DataMap{ itr->second } : DataMap{}; }
    inline void RecordMap::setRecord( Memory recordKey, DataMap data )
        { operator[]( recordKey ) = data; }
    inline void RecordMap::removeRecord( Memory recordKey )
        { erase( recordKey ); }

    inline std::pair<Memory, Memory> RecordMap::splitKey( Memory key )
    {
        auto pos = key.rfind( '.' );
        if ( pos == Memory::npos )
            { return { "", key }; }
        else
            { return { key.substr( 0, pos ), key.substr( pos + 1 ) }; }
    }

    class ReadBuffer;
    void encodeBinary( ReadBuffer & buffer, const cpp::DataMap & value, ByteOrder byteOrder = ByteOrder::Host );
    void decodeBinary( ReadBuffer & buffer, cpp::DataMap & value, ByteOrder byteOrder = ByteOrder::Host );

}

/*

Key key{"item", 10};
auto key = cpp::text("item[%]", 10);

DataMap config;
config["key"] = "10"; // : key(2)='10'

DataMap data;
data["item"]

*/
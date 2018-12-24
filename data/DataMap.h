#pragma once

#include <map>

#include <cpp/String.h>


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
        { auto & itr = find( key ); return itr != end( ) ? itr->second : nullptr; }
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
        { auto & itr = find( key ); return itr != end( ) ? itr->second : nullptr; }
    inline void StringMap::set( Memory key, Memory value )
        { insert_or_assign( key, value ); }
    inline void StringMap::remove( Memory key )
        { erase( key ); }


    class ReadBuffer;
    void encodeBinary( ReadBuffer & buffer, const cpp::DataMap & value, ByteOrder byteOrder = ByteOrder::Host );
    void decodeBinary( ReadBuffer & buffer, cpp::DataMap & value, ByteOrder byteOrder = ByteOrder::Host );

}

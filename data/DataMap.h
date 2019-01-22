#pragma once

/*

	DataMap and StringMap are extensions of std::map.

	(1) Objects of each can convert to/from DataMap & StringMap.
	(2) Extends the interface of std::map, except operator[] which is overridden as read-only.  
	(3) Adds isEmpty(), notEmpty(), get(), set(), remove().
	(4) get() and operator[] return a Memory object which may be null.
	(5) toText() to encode the map as a text string, a constructor to decode the map from EncodedText (e.g. DataMap map = data.asText();). 
	(6) toBinary() to encode the map to a DataBuffer, fromBinary() to decode the map from a DataBuffer. 

*/

#include <map>

#include "String.h"


namespace cpp
{

	class DataBuffer;
    struct StringMap;

    struct DataMap
		: public std::map<Memory, Memory>
    {
		typedef std::map<Memory, Memory>	Map;

		static DataMap		fromBinary( DataBuffer & buffer );

							DataMap( );
							DataMap( DataMap && move );
							DataMap( const DataMap & copy );
							DataMap( Map && move );
							DataMap( const Map & copy );
							DataMap( const StringMap & copy );
							DataMap( std::initializer_list<Map::value_type> init );
							DataMap( const EncodedText & encodedText );

        DataMap &			operator=( DataMap && move );
        DataMap &			operator=( const DataMap & copy );
		DataMap &			operator=( Map && move );
		DataMap &			operator=( const Map & copy );
		DataMap &			operator=( const StringMap & copy );

        Memory				operator[]( Memory key ) const;

        bool				isEmpty( ) const;
        bool				notEmpty( ) const;

        Memory				get( Memory key ) const;
		void				set( Memory key, Memory value );
		void 				remove( Memory key );

		String				toString( ) const;
		Memory				toBinary( DataBuffer & buffer, ByteOrder byteOrder = ByteOrder::Host ) const;
    };



    struct StringMap
		: public std::map<String, String>
    {
		typedef std::map<String, String> Map;

		static StringMap	fromBinary( DataBuffer & buffer );

							StringMap( );
							StringMap( StringMap && move );
							StringMap( const StringMap & copy );
							StringMap( Map && move );
							StringMap( const Map & copy );
							StringMap( const DataMap & copy );
							StringMap( std::initializer_list<Map::value_type> init );
							StringMap( const EncodedText & encodedText );

        StringMap &			operator=( StringMap && move );
        StringMap &			operator=( const StringMap & copy );
		StringMap &			operator=( Map && move );
		StringMap &			operator=( const Map & copy );
		StringMap &			operator=( const DataMap & copy );

        Memory				operator[]( Memory key ) const;

        bool				isEmpty( ) const;
        bool				notEmpty( ) const;

        Memory				get( Memory key ) const;
        void				set( Memory key, Memory value );
		void				remove( Memory key );

		String				toString( ) const;
		Memory				toBinary( DataBuffer & buffer, ByteOrder byteOrder = ByteOrder::Host ) const;
	};



    inline DataMap::DataMap( )
        : Map() {}

    inline DataMap::DataMap( DataMap && move )
        : Map( std::move( move ) ) {}

    inline DataMap::DataMap( const DataMap & copy )
        : Map( copy ) {}

    inline DataMap::DataMap( const StringMap & copy )
        : Map( ) { for ( auto & itr : copy ) { insert_or_assign( itr.first, itr.second ); } }

    inline DataMap::DataMap( std::initializer_list<std::map<Memory, Memory>::value_type> init )
        : Map( init ) {}

    inline DataMap & DataMap::operator=( DataMap && move )
        { Map::operator=( std::move( move ) ); return *this; }

    inline DataMap & DataMap::operator=( const DataMap & copy )
        { Map::operator=( copy ); return *this; }

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
        : Map( ) {}

    inline StringMap::StringMap( StringMap && move )
        : Map( std::move( move ) ) {}

    inline StringMap::StringMap( const StringMap & copy )
        : Map( copy ) {}

    inline StringMap::StringMap( const DataMap & copy )
        : Map( ) { for ( auto & itr : copy ) { insert_or_assign( itr.first, itr.second ); } }

    inline StringMap::StringMap( std::initializer_list<Map::value_type> init )
        : Map( init ) {}

    inline StringMap & StringMap::operator=( StringMap && move )
        { Map::operator=( std::move( move ) ); return *this; }

    inline StringMap & StringMap::operator=( const StringMap & copy )
        { Map::operator=( copy );  return *this; }

    inline StringMap & StringMap::operator=( const DataMap & copy )
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

}

#pragma once

/*

	DataArray and StringArray are extensions of std::vector.

	(1) Objects of each can convert to/from DataArray & StringArray.
	(2) Extends the interface of std::vector, except operator[] which is overridden as read-only.
	(3) Adds isEmpty(), notEmpty(), get(), set(), remove().
	(4) get() and operator[] return a Memory object which may be null.
	(5) toText() to encode the vector as a text string, a constructor to decode the vector from EncodedText (e.g. DataArray array = data.asText();).
	(6) toBinary() to encode the vector to a DataBuffer, fromBinary() to decode the vector from a DataBuffer.

*/

#include <vector>

#include <cpp/data/String.h>


namespace cpp
{

	class DataBuffer;
	struct StringArray;

	struct DataArray
		: public std::vector<Memory>
	{
		typedef std::vector<Memory>	Array;

		static DataArray	fromBinary( DataBuffer & buffer );

		DataArray( );
		explicit DataArray( size_t count );
		DataArray( DataArray && move );
		DataArray( const DataArray & copy );
		DataArray( Array && move );
		DataArray( const Array & copy );
		DataArray( const StringArray & copy );
		DataArray( std::initializer_list<Memory> init );
		DataArray( const EncodedText & encodedText );	//	from comma-separated list e.g. "one, two, three"

		DataArray &			operator=( DataArray && move );
		DataArray &			operator=( const DataArray & copy );
		DataArray &			operator=( Array && move );
		DataArray &			operator=( const Array & copy );
		DataArray &			operator=( const StringArray & copy );

		Memory				operator[]( size_t index ) const;

		bool				isEmpty( ) const;
		bool				notEmpty( ) const;

		Memory				get( size_t index ) const;
		void				set( size_t index, Memory value );
		Memory				remove( size_t index );

		String				toString( ) const;	//	comma-separated list
		Memory				toBinary( DataBuffer & buffer, ByteOrder byteOrder = ByteOrder::Host ) const;
	};



	struct StringArray
		: std::vector<String>
	{
		typedef std::vector<String>	Array;

		static StringArray	fromBinary( DataBuffer & buffer );

							StringArray( );
							explicit StringArray( size_t count );
							StringArray( StringArray && move );
							StringArray( const StringArray & copy );
							StringArray( Array && move );
							StringArray( const Array & copy );
							StringArray( const DataArray & copy );
							StringArray( std::initializer_list<String> init );
							StringArray( const EncodedText & encodedText );

		StringArray &		operator=( StringArray && move );
		StringArray &		operator=( const StringArray & copy );
		StringArray &		operator=( Array && move );
		StringArray &		operator=( const Array & copy );
		StringArray &		operator=( const DataArray & copy );

		Memory				operator[]( size_t index ) const;

		bool				isEmpty( ) const;
		bool				notEmpty( ) const;

		Memory				get( size_t index ) const;
		void				set( size_t index, String value );
		Memory				remove( size_t index );

		String				toString( ) const;
		Memory				toBinary( DataBuffer & buffer, ByteOrder byteOrder = ByteOrder::Host ) const;
	};

}
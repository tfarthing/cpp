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

#include "String.h"


namespace cpp
{

	class DataBuffer;
	struct StringArray;

	struct DataArray
	{
											DataArray( );
											explicit DataArray( size_t count );
											DataArray( DataArray && move ) noexcept;
											DataArray( const DataArray & copy );
											DataArray( std::vector<Memory> && move ) noexcept;
											DataArray( const std::vector<Memory> & copy );
											DataArray( const StringArray & copy );
											DataArray( std::initializer_list<Memory> init );

											DataArray( const EncodedText & encodedText );
											DataArray( const EncodedBinary & encodedBinary );	//	from comma-separated list e.g. "one, two, three"

		DataArray &							operator=( DataArray && move ) noexcept;
		DataArray &							operator=( const DataArray & copy );
		DataArray &							operator=( std::vector<Memory> && move ) noexcept;
		DataArray &							operator=( const std::vector<Memory> & copy );
		DataArray &							operator=( const StringArray & copy );

		Memory								operator[]( size_t index ) const;

		bool								isEmpty( ) const;
		bool								notEmpty( ) const;

		void								add( const Memory & value );
		Memory								get( size_t index ) const;
		void								set( size_t index, const Memory & value );
		Memory								remove( size_t index );

        Memory								toText( DataBuffer & buffer );	//	comma-separated list
        Memory								toBinary( DataBuffer & buffer, ByteOrder byteOrder = ByteOrder::Host );

		std::vector<Memory>					data;
	};



	struct StringArray
	{
							                StringArray( );
							                explicit StringArray( size_t count );
							                StringArray( StringArray && move ) noexcept;
							                StringArray( const StringArray & copy );
							                StringArray( std::vector<String> && move ) noexcept;
							                StringArray( const std::vector<String> & copy );
							                StringArray( const DataArray & copy );
							                StringArray( std::initializer_list<String> init );

                                            StringArray( const EncodedText & encodedText );
                                            StringArray( const EncodedBinary & encodedBinary );

		StringArray &		                operator=( StringArray && move ) noexcept;
		StringArray &		                operator=( const StringArray & copy );
		StringArray &		                operator=( std::vector<String> && move ) noexcept;
		StringArray &		                operator=( const std::vector<String> & copy );
		StringArray &		                operator=( const DataArray & copy );

		Memory				                operator[]( size_t index ) const;

		bool				                isEmpty( ) const;
		bool				                notEmpty( ) const;

        void								add( String value );
        Memory				                get( size_t index ) const;
		void				                set( size_t index, String value );
		String				                remove( size_t index );

        Memory				                toString( DataBuffer & buffer ) const;
		Memory				                toBinary( DataBuffer & buffer, ByteOrder byteOrder = ByteOrder::Host ) const;
    
        std::vector<String>                 data;
    };

}
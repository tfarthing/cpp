#pragma once

/*

	ByteOrder is used to detect the byte order for the current 
	program and also specify byte orderind for encode and decode 
	operations.

*/

#include <cpp/data/Comparable.h>

namespace cpp
{

    class ByteOrder
        : public Comparable<ByteOrder>
    {
    public:
        static const ByteOrder		LittleEndian;
        static const ByteOrder		BigEndian;
        static const ByteOrder		Network;
        static const ByteOrder		Host;

		static int					compare( ByteOrder lhs, ByteOrder rhs );

    private:
		enum Endianess { Little, Big };
		
									ByteOrder( Endianess endianness );
		static ByteOrder			getHostByteOrder( );

	private:
        Endianess					value;
    };


	inline ByteOrder::ByteOrder( Endianess endianness )
		: value( endianness ) 
	{ 
	}


	inline int ByteOrder::compare( ByteOrder lhs, ByteOrder rhs )
	{
		if ( lhs.value == rhs.value )
			{ return 0; }
		else
			{ return lhs.value < rhs.value ? -1 : 1; }
	}

}
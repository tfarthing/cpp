#pragma once

/*

	ByteOrder is used to detect the byte order for the current 
	program and also specify byte ordering for binaryencode and decode 
	operations.

*/

namespace cpp
{

    class ByteOrder
    {
    public:
        static const ByteOrder		LittleEndian;
        static const ByteOrder		BigEndian;
        static const ByteOrder		Network;
        static const ByteOrder		Host;

		bool operator==( ByteOrder other );
		bool operator!=( ByteOrder other );

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


	inline bool ByteOrder::operator==( ByteOrder other )
	{
		return value == other.value;
	}

	inline bool ByteOrder::operator!=( ByteOrder other )
	{
		return value != other.value;
	}

}
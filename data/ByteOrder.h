#pragma once

#include <cpp/data/Comparable.h>

namespace cpp
{

    class ByteOrder
        : public Comparable<ByteOrder>
    {
    public:
        static const ByteOrder LittleEndian;
        static const ByteOrder BigEndian;
        static const ByteOrder Network;
        static const ByteOrder Host;

		static int compare( ByteOrder lhs, ByteOrder rhs );

    private:
        static ByteOrder getHostByteOrder( );

        enum Endianess { Little, Big };
        ByteOrder( Endianess endianness )
            : value( endianness ) { }
        Endianess value;
    };


	int ByteOrder::compare( ByteOrder lhs, ByteOrder rhs )
	{
		if ( lhs.value == rhs.value )
		{
			return 0;
		}
		else
		{
			return lhs.value < rhs.value ? -1 : 1;
		}
	}

}
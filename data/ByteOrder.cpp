#include "ByteOrder.h"

namespace cpp
{

    ByteOrder ByteOrder::getHostByteOrder( )
    {
        static int x = 0x12345678;
        static ByteOrder byteOrder = ( *( reinterpret_cast<char *>( &x ) ) == 0x12 )
            ? ByteOrder::BigEndian
            : ByteOrder::LittleEndian;
        return byteOrder;
    }

    const ByteOrder ByteOrder::LittleEndian = ByteOrder::Endianess::Little;
    const ByteOrder ByteOrder::BigEndian = ByteOrder::Endianess::Big;
    const ByteOrder ByteOrder::Network = ByteOrder::Endianess::Big;
    const ByteOrder ByteOrder::Host = getHostByteOrder( );

}
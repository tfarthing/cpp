#include <cpp/Exception.h>
#include <cpp/io/Output.h>

namespace cpp
{

    Memory Output::put( const Memory & src )
    {
        size_t bytes = 0;
        while ( isOpen( ) && bytes < src.length( ) )
        {
            bytes += write( src.substr( bytes ) ).length( );
        }
        if ( bytes < src.length( ) )
        {
            throw EOFException( "Output::put() : output was closed before all data could be written." );
        }
        return src;
    }

}

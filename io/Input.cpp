#include "../process/Exception.h"
#include "Input.h"
#include "../data/DataBuffer.h"

namespace cpp
{

    //  
    String Input::getAll( )
    {
        StringBuffer result{ 64 * 1024 };
        while ( read( result ).length( ) > 0 )
        {
            if ( result.putable( ).isEmpty( ) )
            {
                result.resize( result.size( ) * 2 );
            }
        }
        return result.getable( );
    }

    Memory Input::read( DataBuffer & dst, Duration timeout )
    {
        Memory bytes = read( dst.putable( ), timeout );
        dst.put( bytes.length( ) );
        return bytes;
    }

    //  block until all is read
    Memory Input::get( Memory dst )
    {
        size_t bytes = 0;
        while ( isOpen( ) && bytes < dst.length( ) )
        {
            bytes += read( dst.substr( bytes ) ).length( );
        }
        if ( bytes < dst.length( ) )
        {
            throw EOFException( "Input::get() : input was closed before all data could be read." );
        }
        return dst;
    }

    Memory Input::get( DataBuffer & dst )
    {
        Memory bytes = get( dst.putable( ) );
        dst.put( bytes.length() );
        return bytes;
    }

}

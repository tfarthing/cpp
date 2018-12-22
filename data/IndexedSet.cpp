#ifndef TEST

#else

#include <cpp/meta/Unittest.h>
#include "IndexedSet.h"

SUITE( IndexedSet )
{
    TEST( test1 )
    {
        cpp::IndexedSet<cpp::String> index;

        index.add( "apple" );
        index.add( "orange" );
        index.add( "avocado" );


        CHECK( index.getAt( 0 ) == "apple" );
        CHECK( index.rgetAt( 0 ) == "avocado" );
        CHECK( index.getAt( 0 ) == index.rgetAt(2) );
    }
}

#endif


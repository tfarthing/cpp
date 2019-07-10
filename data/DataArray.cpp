#ifndef TEST

#else

#include <cpp/meta/Test.h>
#include <cpp/data/DataArray.h>



TEST_CASE( "DataArray" )
{
    using namespace cpp;

    SECTION( "ctor" )
    {
        MemoryArray a = { "a", "b", "c" };
        MemoryArray b = std::move( a );

        StringArray c = b;

        a = c;

        StringArray d = std::move( a );

        String buffer = a.toText( );
        REQUIRE( buffer == "a,b,c" );
    }

}

#endif
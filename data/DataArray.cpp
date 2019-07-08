#ifndef TEST

#else

#include <cpp/data/DataArray.h>
#include <cpp/meta/Test.h>



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

    SECTION( "split" )
    {
        /*
        DataArray lhs = String{ "one, two, three" }.split( "," );
        DataArray rhs = StringArray{ "one", "two", "three" };
        REQUIRE( lhs == rhs );
        */
    }

}

#endif
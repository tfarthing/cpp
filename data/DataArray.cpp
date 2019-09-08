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
        CHECK( a.isEmpty( ) );

        StringArray c = b;
        CHECK( c == b );

        a = c;
        CHECK( a == b );
        CHECK( a == c );

        CHECK( a.size( ) == 3 );
        CHECK( a[0].data( ) == c[0].data( ) );
        CHECK( a[0].data( ) != b[0].data( ) );

        StringArray d = std::move( a );
        CHECK( a.isEmpty( ) );
        CHECK( d == c );

        String buffer = d.toText( );
        CHECK( buffer == "a,b,c" );

        a = Memory{ "d,e,f" }.asText( );
        buffer = a.toText( );
        CHECK( buffer == "d,e,f" );
    }

}

#endif
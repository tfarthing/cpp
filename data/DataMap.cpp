#ifndef TEST


#else

#include <cpp/meta/Test.h>

#include "DataMap.h"
#include "String.h"

cpp::String returnString( )
{
    return "string";
}

cpp::MemoryMap createMap( cpp::MemoryMap input )
{
    return input;
}

TEST_CASE( "DataMap" )
{
    using namespace cpp;

    SECTION( "test1" )
    {

        StringMap stringMap =
        {
            { "key1", String{ "value1" } },
            { "key2", returnString( ) }
        };

        MemoryMap memoryMap = stringMap;
        memoryMap.set( "key3", "value x" );

        CHECK( memoryMap["key1"] == "value1" );
        CHECK( memoryMap["key2"] == "string" );
        CHECK( memoryMap["key3"] == "value x" );
    }
}

#endif

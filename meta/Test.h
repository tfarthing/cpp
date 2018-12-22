#pragma once

/*

EXAMPLE:

#include <cpp/meta/Test.h>

SUITE( SuiteName )
{
    TEST( test1 )
    {
        CHECK( true );
    }

    TEST( test2 )
    {
        CHECK( false );
    }
}
*/

#ifdef TEST

#include <lib/UnitTest++/src/UnitTest++.h>

#include <cpp/util/Log.h>

namespace test
{
	inline void log( cpp::String text )
	{
		cpp::xlog( cpp::LogLevel::Alert, "test", std::move( text ) );
	}
}

#endif

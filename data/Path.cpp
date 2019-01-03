
#ifndef TEST

#include "Path.h"

namespace cpp
{


}

#else

#include <cpp/util/Path.h>
#include <cpp/meta/Unittest.h>

SUITE( String )
{
    using namespace cpp;

    TEST( depth )
    {
        CHECK( Path<'.'>::depth( "" ) == 1 );
        CHECK( Path<'.'>::depth( "1.2.3.4" ) == 4 );
        CHECK( Path<'.'>::depth( "1.2.3.4." ) == 5 );
        CHECK( Path<'.'>::depth( "1...4" ) == 4 );
        CHECK( Path<'.'>::depth( ".2.3.4" ) == 4 );
    }

    TEST( node )
    {
        CHECK( Path<'.'>::node( "", 0 ) == "" );
        CHECK( Path<'.'>::node( "1.2.3.4", 2 ) == "3" );
        CHECK( Path<'.'>::node( "1.2.3.4.", 4 ) == "" );
        CHECK( Path<'.'>::node( "1...4", 2 ) == "" );
        CHECK( Path<'.'>::node( ".2.3.4", 0 ) == "" );

        CHECK_THROW( Path<'.'>::node( "1.2.3.4", 4 ), Exception );
    }

    TEST( contains )
    {
        CHECK( Path<'.'>::contains( "", "class.subclass" ) );
        CHECK( Path<'.'>::contains( "class", "class.subclass" ) );
        CHECK( Path<'.'>::contains( "class.subclass", "class.subclass" ) );
        CHECK( !Path<'.'>::contains( "class.sub", "class.subclass" ) );
        CHECK( !Path<'.'>::contains( "clas", "class.subclass" ) );
    }

    TEST( parent )
    {
        CHECK( Path<'/'>::parent( "root/dir/file.exe" ) == "root/dir" );
        CHECK( Path<'.'>::parent( "1.2.3.4.5" ) == "1.2.3.4" );

        CHECK( Path<'.'>::parent( "1.2.3.4.5", 3 ) == "1.2.3" );
        CHECK( Path<'.'>::parent( "1.2.3.4.5", 6 ) == "1.2.3.4.5" );
    }

    TEST( element )
    {
        CHECK( Path<'/'>::element( "root/dir/file.exe" ) == "file.exe" );
        CHECK( Path<'.'>::element( "1.2.3.4.5" ) == "5" );

        CHECK( Path<'.'>::element( "1.2.3.4.5", 3 ) == "4.5" );
        CHECK( Path<'.'>::element( "1.2.3.4.5", 6 ) == "" );
    }

}

#endif

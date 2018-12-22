#ifndef TEST

#else

#include <cpp/meta/Unittest.h>
#include <cpp/math/Rect.h>

SUITE( Rect )
{
    using namespace cpp;

    TEST( attachTo )
    {
        Rect<f32_t> a{ 0, 0, 100, 100 };
        Rect<f32_t> b{ 0, 0, 10, 10 };

        b.alignWith( a, Alignment::TopLeft, Alignment::TopLeft, { 0,0 } );
        CHECK( b == Rect<f32_t>(0, 0, 10, 10) );

        b.alignWith( a, Alignment::TopLeft, Alignment::TopRight, { 0,0 } );
        CHECK( b == Rect<f32_t>( -10, 0, 10, 10 ) );

        b.alignWith( a, Alignment::TopLeft, Alignment::Center, { 0,0 } );
        CHECK( b == Rect<f32_t>( -5, -5, 10, 10 ) );

        b.alignWith( a, Alignment::Center, Alignment::Center, { 0,0 } );
        CHECK( b == Rect<f32_t>( 45, 45, 10, 10 ) );

        b.alignWith( a, Alignment::CenterLeft, Alignment::CenterLeft, { 0, 0 } );
        CHECK( b == Rect<f32_t>( 0, 45, 10, 10 ) );

    }
}

#endif



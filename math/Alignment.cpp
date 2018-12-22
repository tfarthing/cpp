#ifndef TEST

#include <cpp/Exception.h>
#include <cpp/math/Alignment.h>

namespace cpp
{

    Alignment Alignment::TopLeft        = Alignment{ 0, "TopLeft" } ;
    Alignment Alignment::TopCenter      = Alignment{ 1, "TopCenter" };
    Alignment Alignment::TopRight       = Alignment{ 2, "TopRight" };
    Alignment Alignment::CenterLeft     = Alignment{ 3, "CenterLeft" };
    Alignment Alignment::Center         = Alignment{ 4, "Center" };
    Alignment Alignment::CenterRight    = Alignment{ 5, "CenterRight" };
    Alignment Alignment::BottomLeft     = Alignment{ 6, "BottomLeft" };
    Alignment Alignment::BottomCenter   = Alignment{ 7, "BottomCenter" };
    Alignment Alignment::BottomRight    = Alignment{ 8, "BottomRight" };

    Alignment Alignment::fromValue( int value )
    {
        switch ( value )
        {
        case 0:
            return TopLeft;
        case 1:
            return TopCenter;
        case 2:
            return TopRight;
        case 3:
            return CenterLeft;
        case 4:
            return Center;
        case 5:
            return CenterRight;
        case 6:
            return BottomLeft;
        case 7:
            return BottomCenter;
        case 8:
            return BottomRight;
        default:
            throw Exception( "Invalid Alignment value" );
        }
    }

    Alignment Alignment::fromText( const Memory & text )
    {
        if ( text == TopLeft.text( ) ) 
            { return TopLeft; }
        if ( text == TopCenter.text( ) )
            { return TopCenter; }
        if ( text == TopRight.text( ) )
            { return TopRight; }
        if ( text == CenterLeft.text( ) )
            { return CenterLeft; }
        if ( text == Center.text( ) )
            { return Center; }
        if ( text == CenterRight.text( ) )
            { return CenterRight; }
        if ( text == BottomLeft.text( ) )
            { return BottomLeft; }
        if ( text == BottomCenter.text( ) )
            { return BottomCenter; }
        if ( text == BottomRight.text( ) )
            { return BottomRight; }
        throw Exception( "Invalid Alignment text" );
    }
    
}

#else

#include <cpp/math/Alignment.h>
#include <cpp/meta/Unittest.h>

SUITE( Alignment )
{
    using namespace cpp;

    TEST( ctor )
    {

        Alignment a = Alignment::TopCenter;
        Alignment b = Alignment::Center;
        Alignment c = a;

    }
}


#endif

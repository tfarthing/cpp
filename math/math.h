#pragma once

#include <cmath>
#include <algorithm>

namespace cpp
{

    static const double pi = 3.14159;

    struct Math
    {
        template<class T> static T minmax( T min, T value, T max )
            { return ( value < min ) ? min : ( ( value > max ) ? max : value ); }

        inline static double toRadians( double degree )
            { return degree * pi / 180; }
    
        inline static double cosine( double degree )
            { return std::cos( toRadians( degree ) ); }
        inline static double sine( double degree )
            { return std::sin( toRadians( degree ) ); }
        inline static double tangent( double degree )
            { return std::tan( toRadians( degree ) ); }

    };

}
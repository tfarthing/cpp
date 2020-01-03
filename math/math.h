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

    };

}
#pragma once

#include <functional>

namespace cpp
{

    template<class T>
    int compare(const T & lhs, const T & rhs)
        { if ( lhs == rhs ) { return 0; } return (lhs < rhs) ? -1 : 1; }

    template <typename T, typename Y = T>
    class Comparable
    {
    public:
        bool operator<(const T & other) const
            { return T::compare( static_cast<const Y &>( *this ), other) < 0; }
        
        bool operator<=(const T & other) const
            { return T::compare( static_cast<const Y &>( *this ), other) <= 0; }
        
        bool operator>(const T & other) const
            { return T::compare( static_cast<const Y &>( *this ), other) > 0; }

        bool operator>=(const T & other) const
            { return T::compare( static_cast<const Y &>(*this), other) >= 0; }

        bool operator==(const T & other) const
            { return T::compare( static_cast<const Y &>( *this ), other) == 0; }

        bool operator!=(const T & other) const
            { return T::compare( static_cast<const Y &>( *this ), other) != 0; }
    };

}


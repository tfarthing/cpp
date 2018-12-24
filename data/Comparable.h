#pragma once

#include <functional>

namespace cpp
{

    template<class T>
    int compare(const T & lhs, const T & rhs)
        { if ( lhs == rhs ) { return 0; } return (lhs < rhs) ? -1 : 1; }

    template <typename LHS, typename RHS = LHS>
    class Comparable
    {
    public:
        bool operator<( const RHS & other ) const
            { return LHS::compare( (const LHS &)*this, other) < 0; }
        
        bool operator<=( const RHS & other ) const
            { return LHS::compare( (const LHS &)*this, other) <= 0; }
        
        bool operator>( const RHS & other ) const
            { return LHS::compare( (const LHS &)*this, other) > 0; }

        bool operator>=( const RHS & other ) const
            { return LHS::compare( (const LHS &)*this, other) >= 0; }

        bool operator==( const RHS & other ) const
            { return LHS::compare( (const LHS &)*this, other) == 0; }

        bool operator!=( const RHS & other ) const
            { return LHS::compare( (const LHS &)*this, other) != 0; }
    };

}


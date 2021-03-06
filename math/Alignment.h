#pragma once

#include "../data/Float.h"
#include "../data/String.h"
#include "XY.h"

namespace cpp
{

    class Alignment
    {
    public:
        static Alignment TopLeft;
        static Alignment TopCenter;
        static Alignment TopRight;
        static Alignment CenterLeft;
        static Alignment Center;
        static Alignment CenterRight;
        static Alignment BottomLeft;
        static Alignment BottomCenter;
        static Alignment BottomRight;

        static Alignment fromValue( int value );
        static Alignment fromText( const Memory & text );

        int value( ) const
            { return m_enum->m_value; }
        Memory text( ) const
            { return m_enum->m_text; }

        XY<float32> xy( ) const
            { return XY<float32>{ x( ), y( ) }; }
        float32 x( ) const
            { if ( xcenter( ) ) return 0.5f; return left( ) ? 0.0f : 1.0f; }
        float32 y( ) const
            { if ( ycenter( ) ) return 0.5f; return top( ) ? 0.0f : 1.0f; }
        
        bool top( ) const
            { return m_enum->m_value / 3 == 0; }
        bool ycenter( ) const
            { return m_enum->m_value / 3 == 1; }
        bool bottom( ) const
            { return m_enum->m_value / 3 == 2; }
        bool left( ) const
            { return ( m_enum->m_value % 3 ) == 0; }
        bool xcenter( ) const
            { return ( m_enum->m_value % 3 ) == 1; }
        bool right( ) const
            { return ( m_enum->m_value % 3 ) == 2; }

        bool operator==( const Alignment & other )
            { return m_enum == other.m_enum; }
        bool operator!=( const Alignment & other )
            { return m_enum != other.m_enum; }

    private:
        struct Enum
            { int m_value; cpp::Memory m_text; };
        Alignment( ) 
            : m_enum(nullptr) {}
        Alignment( int value, const char * text )
            : m_enum( new Enum{ value, text } ) { }

        Enum * m_enum;
    };

}
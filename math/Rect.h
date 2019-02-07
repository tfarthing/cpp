#pragma once

#include <algorithm>
#include "../data/Float.h"
#include "Alignment.h"

namespace cpp
{

    template<typename T>
    class Rect
    {
    public:
        Rect( )
            : m_pos( ), m_dim( ) {}
        Rect( XY<T> pos, XY<T> dim )
            : m_pos( pos ), m_dim( dim ) {}
        template<typename CX, typename CY> Rect( CX cx, CY cy )
            : m_pos( ), m_dim( cx, cy ) {}
        template<typename W, typename Z> Rect( XY<W> pos, XY<Z> dim )
            : m_pos( pos ), m_dim( dim ) {}
        template<typename X, typename Y, typename CX, typename CY> Rect( X x, Y y, CX cx, CY cy )
            : m_pos( x, y ), m_dim( cx, cy ) {}
        template<typename W> Rect( const Rect<W> & copy )
            : m_pos( copy.pos( ) ), m_dim( copy.dim( ) ) {}

        bool operator==( const Rect<T> & rhs ) const
            { return m_pos == rhs.m_pos && m_dim == rhs.m_dim; }
        bool operator!=( const Rect<T> & rhs ) const
            { return m_pos != rhs.m_pos || m_dim != rhs.m_dim; }

        T x( ) const
            { return m_pos.m_x; }
        T y( ) const
            { return m_pos.m_y; }
        T cx( ) const
            { return m_dim.m_x; }
        T width( ) const
            { return m_dim.m_x; }
        T cy( ) const
            { return m_dim.m_y; }
        T height( ) const
            { return m_dim.m_y; }

        void setX( T x )
            { m_pos.m_x = x; }
        void setY( T y )
            { m_pos.m_y = y; }
        void setWidth( T width )
            { m_dim.m_x = width; }
        void setHeight( T height )
            { m_dim.m_y = height; }

        T ref_x( f32_t p ) const
            { return x( ) + ref_cx( p ); }
        T ref_cx( f32_t p ) const
            { return (T)( cx( ) * p ); }
        T ref_y( f32_t p ) const
            { return y( ) + ref_cy( p ); }
        T ref_cy( f32_t p ) const
            { return (T)( cy( ) * p ); }

        T top( ) const
            { return y( ); }
        T center_y( ) const
            { return y( ) + cy( ) / 2; }
        T bottom( ) const
            { return y( ) + cy( );  }
        T left( ) const
            { return x( ); }
        T center_x( ) const
            { return x( ) + cx( ) / 2; }
        T right( ) const
            { return x( ) + cx( ); }

        const XY<T> & pos( ) const
            { return m_pos; }
        const XY<T> & top_left( ) const
            { return m_pos; }
        const XY<T> & dim( ) const
            { return m_dim; }
        const XY<T> & size( ) const
            { return m_dim; }
        XY<T> bottom_right( ) const
            { return m_pos + m_dim; }

        bool null( ) const
            { return m_dim.m_x <= 0.0f || m_dim.m_y <= 0.0f; }
        bool contains( const XY<T> & point ) const
            { return top() <= point.y() && left() <= point.x() && bottom( ) >= point.y() && right() >= point.x(); }
        bool contains( const Rect<T> & rect ) const
            { return contains( rect.top_left( ) ) && contains( rect.bottom_right( ) ); }

        bool intersects( const Rect<T> & rect ) const
            { return !( left( ) > rect.right( ) || right( ) < rect.left( ) || top( ) > rect.bottom( ) || bottom( ) < rect.top( ) ); }

        void resize( T cx, T cy )
            { m_dim.m_x = cx; m_dim.m_y = cy; }
        void resize( XY<T> dim )
            { m_dim = dim; }

        Rect<T> trunc( ) const
            { return Rect<T>{ m_pos.trunc( ), m_dim.trunc( ) }; }
        Rect<T> round( ) const
            { return Rect<T>{ m_pos.round( ), m_dim.round( ) }; }

        Rect<T> & adjustTop( T newTop )
            { m_dim.m_y = std::max<T>( 0, cy( ) - ( newTop - top( ) ) ); m_pos.m_y = newTop; return *this; }
        Rect<T> & adjustBottom( T newBottom )
            { m_dim.m_y = std::max<T>( 0, newBottom - top( ) ); return *this; }
        Rect<T> & adjustLeft( T newLeft )
            { m_dim.m_x = std::max<T>( 0, cx( ) - ( newLeft - left( ) ) ); m_pos.m_x = newLeft; return *this; }
        Rect<T> & adjustRight( T newRight )
            { m_dim.m_x = std::max<T>( 0, newRight - left( ) ); return *this; }

        Rect<T> & moveTo( const XY<T> & xy )
            { m_pos = xy; return *this; }
        Rect<T> & moveTo( T x, T y )
            { return moveTo( XY<T>( x, y ) ); }

        Rect<T> & clipTo( const Rect<T> & rect );
        Rect<T> & expandTo( const Rect<T> & rect );

        Rect<T> & alignWith( const Rect<T> & rect, Alignment alignTo, Alignment alignFrom, const XY<T> & offset )
            { return alignWith( rect, alignTo.xy(), alignFrom.xy( ), offset ); }
        Rect<T> & alignWith( const Rect<T> & rect, XY<f32_t> alignTo, XY<f32_t> alignFrom, const XY<T> & offset );

        Rect<T> & stretchWith( const Rect<T> & rect, Alignment stretchTo, Alignment stretcFrom, const XY<T> & offset )
            { return stretchWith( rect, stretchTo.xy( ), stretchFrom.value( ), offset.m_x, offset.m_y ); }
        Rect<T> & stretchWith( const Rect<T> & rect, XY<f32_t> stretchTo, XY<f32_t> stretchFrom, const XY<T> & offset );

        Rect<T> & shrink( T offset );
        Rect<T> & shrink( const XY<T> & offset );
        Rect<T> & expand( T offset );
        Rect<T> & expand( const XY<T> & offset );

        static Rect<T> shrink( Rect<T> rect, T offset );
        static Rect<T> expand( Rect<T> rect, T offset );
        static Rect<T> shrink( Rect<T> rect, const XY<T> & offset );
        static Rect<T> expand( Rect<T> rect, const XY<T> & offset );

    public:
        XY<T> m_pos;
        XY<T> m_dim;
    };

    template<typename T> Rect<T> & Rect<T>::shrink( T offset )
    {
        return shrink( XY<T>{offset, offset} );
    }

    template<typename T>
    Rect<T> & Rect<T>::shrink( const XY<T> & offset )
    {
        adjustTop( top( ) + offset.m_y );
        adjustBottom( bottom( ) - offset.m_y );
        adjustLeft( left( ) + offset.m_x );
        adjustRight( right( ) - offset.m_x );
        return *this;
    }

    template<typename T> Rect<T> & Rect<T>::expand( T offset )
    {
        return expand( XY<T>{offset, offset} );
    }

    template<typename T> Rect<T> & Rect<T>::expand( const XY<T> & offset )
    {
        adjustTop( top( ) - offset.m_y );
        adjustBottom( bottom( ) + offset.m_y );
        adjustLeft( left( ) - offset.m_x );
        adjustRight( right( ) + offset.m_x );
        return *this;
    }

    template<typename T> Rect<T> Rect<T>::shrink( Rect<T> rect, T offset )
    {
        return rect.shrink( offset );
    }
    template<typename T> Rect<T> Rect<T>::expand( Rect<T> rect, T offset )
    {
        return rect.expand( offset );
    }
    template<typename T> Rect<T> Rect<T>::shrink( Rect<T> rect, const XY<T> & offset )
    {
        return rect.shrink( offset );
    }
    template<typename T> Rect<T> Rect<T>::expand( Rect<T> rect, const XY<T> & offset )
    {
        return rect.expand( offset );
    }

    template<typename T>
    Rect<T> & Rect<T>::clipTo( const Rect<T> & rect )
    {
        if ( rect.null( ) )
        {
            m_pos = m_dim = XY<T>( ); return *this;
        }
        if ( null( ) )
        {
            return *this;
        }
        if ( left( ) < rect.left( ) )
        {
            adjustLeft( rect.left( ) );
        }
        if ( right( ) > rect.right( ) )
        {
            adjustRight( rect.right( ) );
        }
        if ( top( ) < rect.top( ) )
        {
            adjustTop( rect.top( ) );
        }
        if ( bottom( ) > rect.bottom( ) )
        {
            adjustBottom( rect.bottom( ) );
        }
        return *this;
    }

    template<typename T>
    Rect<T> & Rect<T>::expandTo( const Rect<T> & rect )
    {
        if ( rect.null( ) )
            { return *this; }
        if ( left( ) > rect.left( ) )
            { adjustLeft( rect.left( ) ); }
        if ( right( ) < rect.right( ) )
            { adjustRight( rect.right( ) ); }
        if ( top( ) > rect.top( ) )
            { adjustTop( rect.top( ) ); }
        if ( bottom( ) < rect.bottom( ) )
            { adjustBottom( rect.bottom( ) ); }
        return *this;
    }

    template<typename T>
    Rect<T> & Rect<T>::alignWith( const Rect<T> & rect, XY<f32_t> alignTo, XY<f32_t> alignFrom, const XY<T> & offset )
    {
        m_pos.m_x = rect.ref_x( alignTo.m_x ) - ref_cx( alignFrom.m_x ) + offset.m_x;
        m_pos.m_y = rect.ref_y( alignTo.m_y ) - ref_cy( alignFrom.m_y ) + offset.m_y;
        return *this;
    }

    template<typename T>
    Rect<T> & Rect<T>::stretchWith( const Rect<T> & rect, XY<f32_t> stretchTo, XY<f32_t> stretchFrom, const XY<T> & offset )
    {
        //  adjust cx
        T x = rect.ref_x( stretchTo.m_x ) + offset.m_x;
        if ( stretchFrom.m_x < 0.5f )
            { adjustLeft( x - ref_cx( stretchFrom.m_x ) ); }
        else
            { adjustRight( x + ref_cx( 1.0f - stretchFrom.m_x ) ); }

        //  adjust cy
        T y = rect.ref_y( stretchTo.m_y ) + offset.m_y;
        if ( stretchFrom.m_y < 0.5f )
            { adjustTop( y - ref_cy( stretchFrom.m_y ) ); }
        else
            { adjustBottom( y + ref_cy( 1.0f - stretchFrom.m_y ) ); }

        return *this;
    }

}
#pragma once

namespace cpp
{
    template<typename T>
    class XY
    {
    public:
        XY( )
            : m_x( 0 ), m_y( 0 ) { }
        XY( T x, T y )
            : m_x( x ), m_y( y ) { }
        template<typename X, typename Y> XY( X x, Y y )
            : m_x( (T)x ), m_y( (T)y ) { }
        template<typename W> XY( const XY<W> & copy )
            : m_x( (T)copy.m_x ), m_y( (T)copy.m_y ) { }

        const T & x( ) const
            { return m_x; }
        const T & y( ) const
            { return m_y; }
        void setX( T x )
            { m_x = x; }
        void setY( T y )
            { m_y = y; }

        bool operator==( const XY<T> & rhs ) const
            { return ( m_x == rhs.m_x && m_y == rhs.m_y ); }
        bool operator!=( const XY<T> & rhs ) const
            { return ( m_x != rhs.m_x || m_y != rhs.m_y ); }

        bool operator<( const XY<T> & rhs ) const
            { return m_y < rhs.m_y || ( m_y == rhs.m_y && m_x < rhs.m_x ); }

        XY<T> trunc( ) const
            { return XY<T>{ std::trunc( m_x ), std::trunc( m_y ) }; }
        XY<T> round( ) const
            { return XY<T>{ std::round( m_x ), std::round( m_y ) }; }

        XY<T> operator+( const XY<T> & rhs ) const
            { return XY<T>( m_x + rhs.m_x, m_y + rhs.m_y ); }
        XY<T> operator-( const XY<T> & rhs ) const
            { return XY<T>( m_x - rhs.m_x, m_y - rhs.m_y ); }
        XY<T> operator*( const XY<T> & rhs ) const
            { return XY<T>( m_x * rhs.m_x, m_y * rhs.m_y ); }
        XY<T> operator/( const XY<T> & rhs ) const
            { return XY<T>( m_x / rhs.m_x, m_y / rhs.m_y ); }
        XY<T> & operator+=( const XY<T> & rhs )
            { m_x += rhs.m_x; m_y += rhs.m_y; return *this; }
        XY<T> & operator-=( const XY<T> & rhs )
            { m_x -= rhs.m_x; m_y -= rhs.m_y; return *this; }
        XY<T> & operator*=( const XY<T> & rhs )
            { m_x *= rhs.m_x; m_y *= rhs.m_y; return *this; }
        XY<T> & operator/=( const XY<T> & rhs )
            { m_x /= rhs.m_x; m_y /= rhs.m_y; return *this; }

        bool isAbove( const XY<T> & rhs ) const
            { return m_y < rhs.m_y; }
        bool isBelow( const XY<T> & rhs ) const
            { return m_y > rhs.m_y; }
        bool isLeftOf( const XY<T> & rhs ) const
            { return m_x < rhs.m_x; }
        bool isRightOf( const XY<T> & rhs ) const
            { return m_x > rhs.m_x; }

        bool isAboveAndLeftOf( const XY<T> & rhs ) const
            { return isAbove( rhs ) && isLeftOf( rhs ); }
        bool isAboveAndRightOf( const XY<T> & rhs ) const
            { return isAbove( rhs ) && isRightOf( rhs ); }
        bool isBelowAndLeftOf( const XY<T> & rhs ) const
            { return isBelow( rhs ) && isLeftOf( rhs ); }
        bool isBelowAndRightOf( const XY<T> & rhs ) const
            { return isBelow( rhs ) && isRightOf( rhs ); }

        template<typename X> XY<T> operator*( X value ) const
            { return XY<T>( m_x * value, m_y * value ); }
        template<typename X> XY<T> operator/( X value ) const
            { return XY<T>( m_x / value, m_y / value ); }
        template<typename X> XY<T> operator*( XY<X> value ) const
            { return XY<T>( m_x * value.m_x, m_y * value.m_y ); }
        template<typename X> XY<T> operator/( XY<X> value ) const
            { return XY<T>( m_x / value.m_x, m_y / value.m_y ); }
    public:
        T m_x;
        T m_y;
    };

}
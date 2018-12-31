#pragma once

/*

	Duration is an abstraction of std::chrono::microseconds
	(1) Can also represent an infinite duration (MAX_INT64): isInfinite()
	(2) Can convert to a double in seconds using toSeconds().

*/

#include <cassert>
#include <chrono>
#include "../data/Integer.h"
#include "../process/Exception.h"



namespace cpp
{

	class Duration
	{
	public:
        static const Duration		Zero;
        static const Duration		Infinite;

		static Duration				ofDays( int64_t days );
		static Duration				ofHours( int64_t hours );
		static Duration				ofMinutes( int64_t minutes );
		static Duration				ofSeconds( int64_t seconds );
		static Duration				ofMillis( int64_t milliseconds );
		static Duration				ofMicros( int64_t microseconds );

									Duration( int64_t microseconds = 0 );
		
									template<typename T, typename Y> 
									Duration( const std::chrono::duration<T, Y> & duration );

		bool						isZero( ) const;
		bool						isNegative( ) const;
		bool						isInfinite( ) const;

		int64_t						days( ) const;
		int64_t						hours( ) const;
		int64_t						minutes( ) const;
		int64_t						seconds( ) const;
		int64_t						millis( ) const;
		int64_t						micros( ) const;

		double						toSeconds( ) const;
									operator std::chrono::microseconds( ) const;

		std::string					toString( ) const;

		static int					compare( const Duration & lhs, const Duration & rhs );

	private:
		int64_t						m_microseconds;
	};



	struct DurationException
		: public cpp::Exception
	{
		DurationException( std::string message )
			: cpp::Exception( std::move( message ) ) { }
	};



	inline Duration Duration::ofDays( int64_t days )
		{ return Duration( days * ( (int64_t)24 * 60 * 60 * 1000000 ) ); }

	inline Duration Duration::ofHours( int64_t hours )
		{ return Duration( hours * ( (int64_t)60 * 60 * 1000000 ) ); }

	inline Duration Duration::ofMinutes( int64_t minutes )
		{ return Duration( minutes * ( (int64_t)60 * 1000000 ) ); }

	inline Duration Duration::ofSeconds( int64_t seconds )
		{ return Duration( seconds * 1000000 ); }

	inline Duration Duration::ofMillis( int64_t milliseconds )
		{ return Duration( milliseconds * 1000 ); }

	inline Duration Duration::ofMicros( int64_t microseconds )
		{ return Duration( microseconds ); }

	inline Duration::Duration( int64_t microseconds )
		: m_microseconds( microseconds ) { }

	template<typename T, typename Y> Duration::Duration( const std::chrono::duration<T, Y> & duration )
		: m_microseconds( std::chrono::duration_cast<std::chrono::microseconds>( duration ).count( ) ) { }

	inline bool Duration::isZero( ) const
		{ return m_microseconds == 0; }

	inline bool Duration::isNegative( ) const
		{ return m_microseconds < 0; }

	inline bool Duration::isInfinite( ) const
		{ return m_microseconds == (int64_t)INT64_MAX; }

	inline int64_t Duration::days( ) const
		{ assert( !isInfinite( ) ); return m_microseconds / ( (int64_t)24 * 60 * 60 * 1000000 ); }

	inline int64_t Duration::hours( ) const
		{ assert( !isInfinite( ) ); return m_microseconds / ( (int64_t)60 * 60 * 1000000 ); }

	inline int64_t Duration::minutes( ) const
		{ assert( !isInfinite( ) ); return m_microseconds / ( (int64_t)60 * 1000000 ); }

	inline int64_t Duration::seconds( ) const
		{ assert( !isInfinite( ) ); return m_microseconds / 1000000; }

	inline int64_t Duration::millis( ) const
		{ assert( !isInfinite( ) ); return m_microseconds / 1000; }

	inline int64_t Duration::micros( ) const
		{ assert( !isInfinite( ) ); return m_microseconds; }

	inline double Duration::toSeconds( ) const
		{ assert( !isInfinite( ) ); return (double)m_microseconds / 1000000.0; }

	inline Duration::operator std::chrono::microseconds( ) const
		{ assert( !isInfinite( ) ); return std::chrono::microseconds{ m_microseconds }; }

	inline int Duration::compare( const Duration & lhs, const Duration & rhs )
		{ return Integer::compare( lhs.m_microseconds, rhs.m_microseconds ); }
}


inline bool operator==( const cpp::Duration & lhs, const cpp::Duration & rhs )
    { return cpp::Duration::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::Duration & lhs, const cpp::Duration & rhs )
    { return cpp::Duration::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::Duration & lhs, const cpp::Duration & rhs )
    { return cpp::Duration::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::Duration & lhs, const cpp::Duration & rhs )
    { return cpp::Duration::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::Duration & lhs, const cpp::Duration & rhs )
    { return cpp::Duration::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::Duration & lhs, const cpp::Duration & rhs )
    { return cpp::Duration::compare( lhs, rhs ) >= 0; }


inline cpp::Duration operator+( const cpp::Duration d1, const cpp::Duration d2 )
{ 
    if ( d1.isInfinite() || d2.isInfinite() )
        { return cpp::Duration::Infinite; }
    return cpp::Duration{ d1.micros() + d2.micros( ) };
}


inline cpp::Duration & operator+=( cpp::Duration & d1, const cpp::Duration d2 )
{ 
    return d1 = d1 + d2;
}


inline cpp::Duration operator-( const cpp::Duration d1, const cpp::Duration d2 )
{ 
    if ( d1.isInfinite( ) || d2.isInfinite( ) )
        { return cpp::Duration::Infinite; }
    return cpp::Duration{ d1.micros( ) - d2.micros( ) }; 
}


inline cpp::Duration & operator-=( cpp::Duration & d1, const cpp::Duration d2 )
{ 
    return d1 = d1 - d2;
}


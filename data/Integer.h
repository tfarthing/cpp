 #pragma once

#include <stdint.h>

#include "Memory.h"

namespace cpp
{

	struct Integer
	{
		static int64_t parse( Memory text, int radix = 10, bool checkEnding = true );
		static uint64_t parseUnsigned( Memory text, int radix = 10, bool checkEnding = true );

		static std::string toHex( uint64_t value, int width = 8, bool upper = false, bool zeroed = true, bool prefix = true );
		static std::string toDecimal( int64_t value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string toDecimal( uint64_t value, int width = 0, bool zeroed = false, bool sign = false );
    
        static std::string toDecimal( int32_t value, int width = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (int64_t)value, width, zeroed, sign); }
        static std::string toDecimal( int16_t value, int width = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (int64_t)value, width, zeroed, sign); }
        static std::string toDecimal( int8_t value, int width = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (int64_t)value, width, zeroed, sign); }

        static std::string toDecimal( uint32_t value, int width = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (uint64_t)value, width, zeroed, sign); }
        static std::string toDecimal( uint16_t value, int width = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (uint64_t)value, width, zeroed, sign); }
        static std::string toDecimal( uint8_t value, int width = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (uint64_t)value, width, zeroed, sign); }

		template<class T, class V, typename std::enable_if<std::is_integral<T>::value>::type>
		static T to( V value );

		template<class T, typename = std::enable_if_t<std::is_integral<T>::value>>
		static int compare( T lhs, T rhs );

    };


	template<class T, class V, typename std::enable_if<std::is_integral<T>::value>::type>
	static T Integer::to( V value )
	{
		bool isValid = CanTypeFitValue<T>( value );
		check<OutOfBoundsException>( isValid, "Integer::to() : value cast to integer type that cannot hold the value" );
		if ( value < 0 && value )

	}


	template<class T, typename>
	int Integer::compare( T lhs, T rhs )
	{
		if ( lhs < rhs )
			{ return -1; }
		return ( lhs > rhs ) ? 1 : 0;
	}


	
	// the upper bound must always be checked
	template <typename target_type, typename actual_type>
	bool test_upper_bound( const actual_type n )
	{
		typedef typename std::common_type<target_type, actual_type>::type common_type;
		const auto c_n = static_cast<common_type>( n );
		const auto t_max = static_cast<common_type>( std::numeric_limits<target_type>::max( ) );
		return ( c_n <= t_max );
	}

	// the lower bound is only needed to be checked explicitely in non-trivial cases, see the next two functions
	template <typename target_type, typename actual_type>
	typename std::enable_if<!( std::is_unsigned<target_type>::value ), bool>::type
		test_lower_bound( const actual_type n )
	{
		typedef typename std::common_type<target_type, actual_type>::type common_type;
		const auto c_n = static_cast<common_type>( n );
		const auto t_min = static_cast<common_type>( std::numeric_limits<target_type>::lowest( ) );
		return ( c_n >= t_min );
	}

	// for unsigned target types, the sign of n musn't be negative
	// but that's not an issue with unsigned actual_type
	template <typename target_type, typename actual_type>
	typename std::enable_if<std::is_integral<target_type>::value &&
		std::is_unsigned<target_type>::value &&
		std::is_integral<actual_type>::value &&
		std::is_unsigned<actual_type>::value, bool>::type
		test_lower_bound( const actual_type )
	{
		return true;
	}

	// for unsigned target types, the sign of n musn't be negative
	template <typename target_type, typename actual_type>
	typename std::enable_if<std::is_integral<target_type>::value &&
		std::is_unsigned<target_type>::value &&
		( !std::is_integral<actual_type>::value ||
			!std::is_unsigned<actual_type>::value ), bool>::type
		test_lower_bound( const actual_type n )
	{
		return ( n >= 0 );
	}

	// value may be integral if the target type is non-integral
	template <typename target_type, typename actual_type>
	typename std::enable_if<!std::is_integral<target_type>::value, bool>::type
		test_integrality( const actual_type )
	{
		return true;
	}

	// value must be integral if the target type is integral
	template <typename target_type, typename actual_type>
	typename std::enable_if<std::is_integral<target_type>::value, bool>::type
		test_integrality( const actual_type n )
	{
		return ( ( std::abs( n - std::floor( n ) ) < 1e-8 ) || ( std::abs( n - std::ceil( n ) ) < 1e-8 ) );
	}

	// perform check only if non-trivial
	template <typename target_type, typename actual_type>
	typename std::enable_if<!std::is_same<target_type, actual_type>::value, bool>::type
		CanTypeFitValue( const actual_type n )
	{
		return test_upper_bound<target_type>( n ) &&
			test_lower_bound<target_type>( n ) &&
			test_integrality<target_type>( n );
	}


	// trivial case: actual_type == target_type
	template <typename actual_type>
	bool CanTypeFitValue( const actual_type )
	{
		return true;
	}

}

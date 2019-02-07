 #pragma once

#include <stdint.h>

#include "Memory.h"



namespace cpp
{

	struct Integer
	{
		static int64_t					parse( Memory text, int radix = 10, bool checkEnding = true );
		static uint64_t					parseUnsigned( Memory text, int radix = 10, bool checkEnding = true );

		static std::string				toHex( uint64_t value, int width = 8, bool upper = false, bool zeroed = true, bool prefix = true );
		static std::string				toDecimal( int64_t value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( uint64_t value, int width = 0, bool zeroed = false, bool sign = false );
    
		static std::string				toDecimal( int32_t value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( int16_t value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( int8_t value, int width = 0, bool zeroed = false, bool sign = false );

		static std::string				toDecimal( uint32_t value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( uint16_t value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( uint8_t value, int width = 0, bool zeroed = false, bool sign = false );

		template<class T, typename = std::enable_if_t<std::is_integral<T>::value>>
		static int						compare( T lhs, T rhs );

		template<class T> 
		static T						to( uint64_t value );

		template<class T>
		static T						to( int64_t value );
	};


	
	inline std::string Integer::toDecimal( int32_t value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (int64_t)value, width, zeroed, sign );
	}
	
	
	inline std::string	Integer::toDecimal( int16_t value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (int64_t)value, width, zeroed, sign );
	}
	
	
	inline std::string	Integer::toDecimal( int8_t value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (int64_t)value, width, zeroed, sign );
	}
		
	
	inline std::string	Integer::toDecimal( uint32_t value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (uint64_t)value, width, zeroed, sign );
	}
	
	
	inline std::string	Integer::toDecimal( uint16_t value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (uint64_t)value, width, zeroed, sign );
	}
	
	
	inline std::string Integer::toDecimal( uint8_t value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (uint64_t)value, width, zeroed, sign );
	}


	template<class T, typename>
	int Integer::compare( T lhs, T rhs )
	{
		if ( lhs < rhs )
		{
			return -1;
		}
		return ( lhs > rhs ) ? 1 : 0;
	}


	template<class T>
	T Integer::to( uint64_t value )
	{
		assert( value <= std::numeric_limits<T>::max( ) );
		return (T)value;
	}


	template<class T>
	T Integer::to( int64_t value )
	{
		if ( value >= 0 )
			{ return to<T>( (uint64_t)value ); }

		assert( value >= std::numeric_limits<T>::min( ) );
		return (T)value;
	}


}

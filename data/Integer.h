 #pragma once

#include "../../cpp/data/Primitive.h"
#include "Memory.h"



namespace cpp
{

	struct Integer
	{
		static int64					parse( Memory text, int radix = 10, bool checkEnding = true );
		static uint64					parseUnsigned( Memory text, int radix = 10, bool checkEnding = true );

		static std::string				toHex( uint64 value, int width = 8, bool upper = false, bool zeroed = true, bool prefix = false );
		static std::string				toDecimal( int64 value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( uint64 value, int width = 0, bool zeroed = false, bool sign = false );
    
		static std::string				toDecimal( int32 value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( int16 value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( int8 value, int width = 0, bool zeroed = false, bool sign = false );

		static std::string				toDecimal( uint32 value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( uint16 value, int width = 0, bool zeroed = false, bool sign = false );
		static std::string				toDecimal( uint8 value, int width = 0, bool zeroed = false, bool sign = false );

		template<class T, typename = std::enable_if_t<std::is_integral<T>::value>>
		static int						compare( T lhs, T rhs );

        template<class T, class V, typename std::enable_if<std::is_integral<V>{} && std::is_unsigned<V>{}, int>::type = 0 >
        static T						to( V value );

		template<class T, class V, typename std::enable_if<std::is_integral<V>{} && std::is_signed<V>{}, int>::type = 0>
		static T						to( V value );
	};


	
	inline std::string Integer::toDecimal( int32 value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (int64)value, width, zeroed, sign );
	}
	
	
	inline std::string	Integer::toDecimal( int16 value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (int64)value, width, zeroed, sign );
	}
	
	
	inline std::string	Integer::toDecimal( int8 value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (int64)value, width, zeroed, sign );
	}
		
	
	inline std::string	Integer::toDecimal( uint32 value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (uint64)value, width, zeroed, sign );
	}
	
	
	inline std::string	Integer::toDecimal( uint16 value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (uint64)value, width, zeroed, sign );
	}
	
	
	inline std::string Integer::toDecimal( uint8 value, int width, bool zeroed, bool sign )
	{
		return toDecimal( (uint64)value, width, zeroed, sign );
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


	template<class T, class V, typename std::enable_if<std::is_integral<V>{} && std::is_unsigned<V>{}, int>::type >
	T Integer::to( V value )
	{
		assert( value <= std::numeric_limits<T>::max( ) );
		return (T)value;
	}


	template<class T, class V, typename std::enable_if<std::is_integral<V>{} && std::is_signed<V>{}, int>::type>
	T Integer::to( V value )
	{
		if ( value >= 0 )
			{ return to<T>( (uint64)value ); }

		assert( value >= std::numeric_limits<T>::min( ) );
		return (T)value;
	}


}

#include "../data/Integer.h"
#include "../data/String.h"
#include "Duration.h"



namespace cpp
{

    const Duration Duration::Zero{ 0 };
    const Duration Duration::Infinite{ INT64_MAX };


	//	4:57:45.765432
	std::string Duration::toString( ) const
	{
		String result;

		auto nyears = days( ) / 365;
		auto ndays = days( ) % 365;
		auto nhours = hours( ) % 24;
		auto nminutes = minutes( ) % 60;
		auto nseconds = seconds( ) % 60;
		auto nmicros = micros( ) % 1000000;

		if ( nyears != 0 )
		{
			result += format( "%y", nyears );
		}
		if ( result.notEmpty( ) || ndays != 0 )
		{
			if ( result.notEmpty( ) )
				{ result += ":"; }

			result += Integer::toDecimal( ndays, 2, true );
		}
		if ( result.notEmpty( ) || nhours != 0 )
		{
			if ( result.notEmpty( ) )
				{ result += ":"; }

			result += Integer::toDecimal( nhours, 2, true );
		}
		if ( result.notEmpty( ) || nminutes != 0 )
		{
			if ( result.notEmpty( ) )
				{ result += ":"; }

			result += Integer::toDecimal( nminutes, 2, true );
		}
		if ( result.notEmpty( ) )
			{ result += ":"; }
		result += Integer::toDecimal( nseconds, result.isEmpty( ) ? 0 : 2, true );
		
		if ( nmicros )
			{ result += "." + Integer::toDecimal( nmicros, 6, true ); }

		result.trim( "0" );

		return std::move( result.data );
	}

}
#pragma once

#include <cassert>
#include <chrono>



namespace cpp
{

    enum class Weekday
        { Sunday, Monday, Tuesday, Wednesday, Thurday, Friday, Saturday };
    enum class Month
        { January=1, February, March, April, May, June, July, August, September, October, November, December };


    class Date
    {
    public:
		Date( const std::tm & time, bool isLocalTime );

		int year( ) const;
		int month( ) const;
		int day( ) const;
		int hour( ) const;
		int minute( ) const;
		int second( ) const;
		bool isDST( ) const;

		int dayOfYear( ) const;
		Weekday dayOfWeek( ) const;

		bool isLocalTime( ) const;
		const std::tm & data( ) const;

		static int fromMonth( Month month );
		static Month toMonth( int monthIndex ); // one-based

		static int fromWeekday( Weekday month );
		static Weekday toWeekday( int weekdayIndex ); // zero-based

    private:
		bool m_isLocalTime;
        std::tm m_date;
    };


	Date::Date( const std::tm & time, bool isLocalTime )
        : m_date( time ), m_isLocalTime( isLocalTime ) { }

    int Date::year() const
        { return m_date.tm_year + 1900; }
    
	int Date::month() const
        { return m_date.tm_mon + 1; }
    
	int Date::day() const
        { return m_date.tm_mday; }
    
	int Date::hour() const
        { return m_date.tm_hour; }
    
	int Date::minute() const
        { return m_date.tm_min; }
    
	int Date::second() const
        { return m_date.tm_sec; }
    
	bool Date::isDST() const
        { return m_date.tm_isdst > 0; }

    int Date::dayOfYear() const
        { return m_date.tm_yday; }

	Weekday Date::dayOfWeek() const
        { return (Weekday)m_date.tm_wday; }

	bool Date::isLocalTime( ) const
		{ return m_isLocalTime; }

	const std::tm & Date::data( ) const
		{ return m_date; }


	int Date::fromMonth( Month month )
		{ return (int)month; }

	Month Date::toMonth( int monthIndex )
	{ 
		assert( monthIndex >= (int)Month::January && monthIndex <= (int)Month::December );
		return (Month)monthIndex; 
	}

	int Date::fromWeekday( Weekday weekday )
		{ return (int)weekday; }

	Weekday Date::toWeekday( int weekdayIndex )
	{
		assert( weekdayIndex >= (int)Weekday::Monday && weekdayIndex <= (int)Weekday::Monday );
		return (Weekday)weekdayIndex;
	}

}

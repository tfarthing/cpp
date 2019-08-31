#pragma once

#include <cassert>
#include <chrono>
#include <string>



namespace cpp
{

    enum class Weekday
        { Sunday, Monday, Tuesday, Wednesday, Thurday, Friday, Saturday };

    enum class Month
        { January=1, February, March, April, May, June, July, August, September, October, November, December };



    class Date
    {
    public:
		                                    Date( const std::tm & time, int micros, bool isLocalTime );

		int                                 year( ) const;
		int                                 month( ) const;
		int                                 day( ) const;
		int                                 hour( ) const;
		int                                 minute( ) const;
		int                                 second( ) const;
        int                                 micros( ) const;
        bool                                isDST( ) const;

		int                                 dayOfYear( ) const; // days since January 1 ( 0-365 )
		Weekday                             dayOfWeek( ) const;

		bool                                isLocalTime( ) const;
		const std::tm &                     data( ) const;

        std::string							toString( ) const;                      // %Y-%m-%d %H:%M:%S.<millis>
        std::string							toString( const char * format ) const;  // https://en.cppreference.com/w/cpp/chrono/c/strftime

		static int                          fromMonth( Month month );
		static Month                        toMonth( int monthIndex ); // one-based

		static int                          fromWeekday( Weekday month );
		static Weekday                      toWeekday( int weekdayIndex ); // zero-based

    private:
		bool                                m_isLocalTime;
        std::tm                             m_date;
        int                                 m_micros;
    };


	inline Date::Date( const std::tm & time, int micros, bool isLocalTime )
        : m_isLocalTime( isLocalTime ), m_date( time ), m_micros( micros ){ }


	inline int Date::year() const
        { return m_date.tm_year + 1900; }
    

	inline int Date::month() const
        { return m_date.tm_mon + 1; }
    

	inline int Date::day() const
        { return m_date.tm_mday; }
    

	inline int Date::hour() const
        { return m_date.tm_hour; }
    

	inline int Date::minute() const
        { return m_date.tm_min; }
    

	inline int Date::second() const
        { return m_date.tm_sec; }


    inline int Date::micros( ) const
        { return m_micros; }

    
	inline bool Date::isDST() const
        { return m_date.tm_isdst > 0; }


	inline int Date::dayOfYear() const
        { return m_date.tm_yday; }


	inline Weekday Date::dayOfWeek() const
        { return (Weekday)m_date.tm_wday; }


	inline bool Date::isLocalTime( ) const
		{ return m_isLocalTime; }


	inline const std::tm & Date::data( ) const
		{ return m_date; }


	inline int Date::fromMonth( Month month )
		{ return (int)month; }


	inline Month Date::toMonth( int monthIndex )
	{ 
		assert( monthIndex >= (int)Month::January && monthIndex <= (int)Month::December );
		return (Month)monthIndex; 
	}


	inline int Date::fromWeekday( Weekday weekday )
		{ return (int)weekday; }


	inline Weekday Date::toWeekday( int weekdayIndex )
	{
		assert( weekdayIndex >= (int)Weekday::Monday && weekdayIndex <= (int)Weekday::Monday );
		return (Weekday)weekdayIndex;
	}

}

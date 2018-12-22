#pragma once

#pragma warning (disable: 4996)

#include <chrono>
#include <cpp/chrono/DateTime.h>
#include <cpp/util/Comparable.h>

/*
    Someday wrap https://howardhinnant.github.io/date/tz.html
*/

namespace cpp
{

    enum class Weekday
        { Sunday, Monday, Tuesday, Wednesday, Thurday, Friday, Saturday };
    enum class Month
        { January, February, March, April, May, June, July, August, September, October, November, December };

    class Date
        : public Comparable<Date>
    {
    public:
        static Date fromTime( DateTime time )
            { time_t epochTime = time.toEpochTime( ); return Date{ *std::localtime( &epochTime ) }; }
        static Date now( )
            { return fromTime( DateTime::now() ); }
        static Date create( int year, int month, int day, int hour, int min, int sec )
        {
            std::tm input{ };
            input.tm_year = year - 1900;
            input.tm_mon = month - 1;
            input.tm_mday = day;
            input.tm_hour = hour;
            input.tm_min = min;
            input.tm_sec = sec;
            input.tm_isdst = -1;
            mktime( &input );
            return Date{ input };
        }
        static Duration utcDelta( );

        Date( )
            : m_date( ) { time_t now = time(0); m_date = *std::localtime( &now ); }
        Date( const std::tm & time )
            : m_date( time ) { }

        static int compare( const Date & lhs, const Date & rhs )
            { return cpp::compare( lhs.toEpochTime(), rhs.toEpochTime() ); }

        int year() const
            { return m_date.tm_year + 1900; }
        int month() const
            { return m_date.tm_mon + 1; }
        int day() const
            { return m_date.tm_mday; }
        int hour() const
            { return m_date.tm_hour; }
        int minute() const
            { return m_date.tm_min; }
        int second() const
            { return m_date.tm_sec; }
        bool isDST() const
            { return m_date.tm_isdst > 0; }

        int dayOfYear() const
            { return m_date.tm_yday; }
        Weekday dayOfWeek() const
            { return (Weekday)m_date.tm_wday; }

        const std::tm & toTM( ) const
            { return m_date; }
        time_t toEpochTime( ) const
            { std::tm date = m_date; return std::mktime( ( std::tm * )&date ); }
        DateTime toTime( ) const
            { return DateTime{ toEpochTime( ) }; }

        String toString( String format ) const
            { String result( 64, '\0' ); result.resize( strftime( (char *)result.c_str( ), result.length( ), format.c_str( ), &m_date ) ); return result; }
        String toString( ) const
            { return toString( "%F %X" ); }

    private:
        std::tm m_date;
    };

    /*
    inline Date operator+( const Date & date, const Duration duration )
        { return Date{ time.sinceEpoch( ) + duration }; }

    inline Date operator+( const Duration duration, const Date & date )
        { return Date{ time.sinceEpoch( ) + duration }; }

    inline Date operator-( const Date & date, const Duration duration )
        { return Date{ time.sinceEpoch( ) - duration }; }

    inline Date operator-( const Duration duration, const Date & date )
        { return Date{ time.sinceEpoch( ) - duration }; }
    */
}

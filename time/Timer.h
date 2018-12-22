#pragma once

#include <chrono>
#include <cpp/chrono/Duration.h>
#include <cpp/chrono/Time.h>

namespace cpp
{

    class Timer
    {
    public:
        typedef std::chrono::high_resolution_clock clock_t;

        Timer( )
            : m_timepoint( clock_t::now( ) ) { }

        Duration get( ) const
            { return Duration{ clock_t::now( ) - m_timepoint}; }
        
        Duration reset( )
            { auto now = clock_t::now( ); auto begin = m_timepoint; m_timepoint = now; return Duration{ now - begin }; }

        bool test( Duration duration ) const
            { return get() >= duration; }

        Duration until( Duration duration ) const
            { return duration - get(); }


    private:
        clock_t::time_point m_timepoint;
    };

}
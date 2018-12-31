#pragma once

#include <chrono>

#include "Duration.h"
#include "Time.h"

namespace cpp
{

    class Timer
    {
    public:
        typedef std::chrono::high_resolution_clock clock_t;

									Timer( );

		Duration					get( ) const;
		Duration					reset( );
		Duration					until( Duration duration ) const;
		bool						elapsed( Duration duration ) const;

    private:
        clock_t::time_point			m_timepoint;
    };


	Timer::Timer( )
		: m_timepoint( clock_t::now( ) ) 
	{ 
	}

	Duration Timer::get( ) const
	{
		return Duration{ clock_t::now( ) - m_timepoint };
	}

	Duration Timer::reset( )
	{
		auto now = clock_t::now( ); auto begin = m_timepoint; m_timepoint = now; return Duration{ now - begin };
	}

	Duration Timer::until( Duration duration ) const
	{
		return duration - get( );
	}

	bool Timer::elapsed( Duration duration ) const
	{
		return get( ) > duration;
	}

}
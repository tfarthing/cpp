#pragma once

/*

	Provides abstraction for std::mutex, std::condition_variable, and std::unique_lock
	which are compatible with interruptable Thread implementation.
	(1) Mutex is encapsulation of std::mutex and std::condition_variable pair.  Provides
		lock(), tryLock(), notifyOne(), and notifyAll() methods.
	(2) Lock is encapsulation of std::unique_lock and provides wait interaction with the
		std::condition_variable of the Mutex.
	(3) Interrupting a thread that has called Lock::wait() will result in this function 
		throwing an InterruptException.

*/


#include <mutex>
#include <condition_variable>

#include "../time/Time.h"



namespace cpp
{

    class Mutex;
    class Lock
    {
    public:
        Lock( Mutex & mutex, bool allowInterrupt = true );
        Lock( Mutex & mutex, std::try_to_lock_t t, bool allowInterrupt = true );

        bool hasLock( ) const;

        void lock( );
        void unlock( );

        void wait( );
		std::cv_status waitUntil( Time time );
		std::cv_status waitFor( Duration duration );

		template<class Predicate> void wait( Predicate p );
		template<class Predicate> bool waitUntil( Time time, Predicate p );
		template<class Predicate> bool waitFor( Duration duration, Predicate p );

        void notifyOne( );
        void notifyAll( );

    private:
        Mutex & m_mutex;
        std::unique_lock<std::mutex> m_lock;
        bool m_allowInterrupt;
    };

    class Mutex
    {
    public:
        Lock lock( bool allowInterrupt = true );
        Lock tryLock( bool allowInterrupt = true );

        void notifyOne( );
        void notifyAll( );

        std::mutex & getMutex( );
        std::condition_variable & getCvar( );

    private:
        std::mutex m_mutex;
        std::condition_variable m_cvar;
    };


    class RecursiveMutex;
    class RecursiveLock
    {
    public:
        RecursiveLock( RecursiveMutex & mutex, bool allowInterrupt = true );
        RecursiveLock( RecursiveMutex & mutex, std::try_to_lock_t t, bool allowInterrupt = true );

        bool hasLock( ) const;

        void lock( );
        void unlock( );

        void wait( );
		std::cv_status waitUntil( Time time );
		std::cv_status waitFor( Duration duration );

		template<class Predicate> void wait( Predicate p );
		template<class Predicate> bool waitUntil( Time time, Predicate p );
		template<class Predicate> bool waitFor( Duration duration, Predicate p );

        void notifyOne( );
        void notifyAll( );

    private:
        RecursiveMutex & m_mutex;
        std::unique_lock<std::recursive_mutex> m_lock;
        bool m_allowInterrupt;
    };

    class RecursiveMutex
    {
    public:
        RecursiveLock lock( bool allowInterrupt = true );
        RecursiveLock tryLock( bool allowInterrupt = true );

        void notifyOne( );
        void notifyAll( );

        std::recursive_mutex & getMutex( );
        std::condition_variable_any & getCvar( );

    private:
        std::recursive_mutex m_mutex;
        std::condition_variable_any m_cvar;
    };



	template<class Predicate> void Lock::wait( Predicate p )
	{
		while ( !p( ) ) { wait( ); }
	}


	template<class Predicate> bool Lock::waitUntil( Time time, Predicate p )
	{
		while ( !p( ) && waitUntil( time ) == std::cv_status::no_timeout );
		return p( );
	}


	std::cv_status Lock::waitFor( Duration duration )
	{
		if ( duration.isInfinite( ) )
		{
			wait( ); return std::cv_status::no_timeout;
		};
		return waitUntil( Time::now( ) + duration );
	}


	template<class Predicate> bool Lock::waitFor( Duration duration, Predicate p )
	{
		if ( duration.isInfinite( ) )
		{
			wait( p ); return true;
		}
		return waitUntil( Time::now( ) + duration, p );
	}



	template<class Predicate> void RecursiveLock::wait( Predicate p )
	{
		while ( !p( ) ) { wait( ); }
	}


	template<class Predicate> bool RecursiveLock::waitUntil( Time time, Predicate p )
	{
		while ( !p( ) && waitUntil( time ) == std::cv_status::no_timeout );
		return p( );
	}


	std::cv_status RecursiveLock::waitFor( Duration duration )
	{
		if ( duration.isInfinite( ) )
		{
			wait( ); return std::cv_status::no_timeout;
		};
		return waitUntil( Time::now( ) + duration );
	}


	template<class Predicate> bool RecursiveLock::waitFor( Duration duration, Predicate p )
	{
		if ( duration.isInfinite( ) )
		{
			wait( p ); return true;
		}
		return waitUntil( Time::now( ) + duration, p );
	}

}

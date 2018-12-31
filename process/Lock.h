#pragma once

#include <mutex>
#include <condition_variable>

#include "../time/Time.h"



namespace cpp
{

    struct LockEvent
    {
        virtual void notifyOne( ) = 0;
        virtual void notifyAll( ) = 0;
    };

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
        template<class Predicate> void wait( Predicate p )
            { while ( !p( ) ) { wait( ); } }

        std::cv_status waitUntil( Time time );
        template<class Predicate> bool waitUntil( Time time, Predicate p )
        { 
            while ( !p( ) && waitUntil( time ) == std::cv_status::no_timeout );
            return p( ); 
        }

        std::cv_status waitFor( Duration duration )
        {
            if ( duration.isInfinite( ) ) 
                { wait( ); return std::cv_status::no_timeout; }; 
            return waitUntil( Time::now( ) + duration );
        }
        template<class Predicate> bool waitFor( Duration duration, Predicate p )
        { 
            if ( duration.isInfinite() )
                { wait( p ); return true; }
            return waitUntil( Time::now( ) + duration, p ); 
        }

        void notifyOne( );
        void notifyAll( );

    private:
        Mutex & m_mutex;
        std::unique_lock<std::mutex> m_lock;
        bool m_allowInterrupt;
    };

    class Mutex
        : public LockEvent
    {
    public:
        Lock lock( bool allowInterrupt = true );
        Lock tryLock( bool allowInterrupt = true );

        void notifyOne( ) override;
        void notifyAll( ) override;

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
        template<class Predicate> void wait( Predicate p )
            { while ( !p( ) ) { wait( ); } }

        std::cv_status waitUntil( Time time );
        template<class Predicate> bool waitUntil( Time time, Predicate p )
        { 
            while ( !p( ) && waitUntil( time ) == std::cv_status::no_timeout );
            return p( ); 
        }

        std::cv_status waitFor( Duration duration )
        {
            if ( duration.isInfinite( ) ) 
                { wait( ); return std::cv_status::no_timeout; }; 
            return waitUntil( Time::now( ) + duration );
        }
        template<class Predicate> bool waitFor( Duration duration, Predicate p )
        { 
            if ( duration.isInfinite() )
                { wait( p ); return true; }
            return waitUntil( Time::now( ) + duration, p ); 
        }

        void notifyOne( );
        void notifyAll( );

    private:
        RecursiveMutex & m_mutex;
        std::unique_lock<std::recursive_mutex> m_lock;
        bool m_allowInterrupt;
    };

    class RecursiveMutex
        : public LockEvent
    {
    public:
        RecursiveLock lock( bool allowInterrupt = true );
        RecursiveLock tryLock( bool allowInterrupt = true );

        void notifyOne( ) override;
        void notifyAll( ) override;

        std::recursive_mutex & getMutex( );
        std::condition_variable_any & getCvar( );

    private:
        std::recursive_mutex m_mutex;
        std::condition_variable_any m_cvar;
    };

}

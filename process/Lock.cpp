#include "Lock.h"
#include "Thread.h"

namespace cpp
{

    Lock::Lock( Mutex & mutex, bool allowInterrupt )
        : m_mutex( mutex ), m_lock( m_mutex.getMutex( ) ), m_allowInterrupt( allowInterrupt ) { }
    Lock::Lock( Mutex & mutex, std::try_to_lock_t t, bool allowInterrupt )
        : m_mutex( mutex ), m_lock( m_mutex.getMutex( ), t ), m_allowInterrupt( allowInterrupt ) { }

    bool Lock::hasLock( ) const
        { return m_lock.owns_lock( );  }

    void Lock::lock( )
        { m_lock.lock( ); }
    void Lock::unlock( )
        { m_lock.unlock( ); }

    void Lock::wait( )
    { 
        if ( m_allowInterrupt )
            { Thread::enterWait( [=]( ) { m_mutex.notifyAll( ); } ); }
        m_mutex.getCvar( ).wait( m_lock ); 
        if ( m_allowInterrupt )
            { Thread::leaveWait( ); }
    }

    std::cv_status Lock::waitUntil( Time time )
    {
        if ( m_allowInterrupt )
			{ Thread::enterWait( [=]( ) { m_mutex.notifyAll( ); } ); }
        std::cv_status status = m_mutex.getCvar( ).wait_until( m_lock, (std::chrono::steady_clock::time_point)time );
        if ( m_allowInterrupt )
            { Thread::leaveWait( ); }
        return status;
    }

    void Lock::notifyOne( )
        { m_mutex.notifyOne( ); }
    void Lock::notifyAll( )
        { m_mutex.notifyAll( ); }



    Lock Mutex::lock( bool allowInterrupt )
        { return Lock( *this, allowInterrupt ); }
    Lock Mutex::tryLock( bool allowInterrupt )
        { return Lock( *this, std::try_to_lock_t{ }, allowInterrupt ); }

    void Mutex::notifyOne( )
        { m_cvar.notify_one( ); }
    void Mutex::notifyAll( )
        { m_cvar.notify_all( ); }

    std::mutex & Mutex::getMutex( )
        { return m_mutex; }
    std::condition_variable & Mutex::getCvar( )
        { return m_cvar; }




    RecursiveLock::RecursiveLock( RecursiveMutex & mutex, bool allowInterrupt )
        : m_mutex( mutex ), m_lock( m_mutex.getMutex( ) ), m_allowInterrupt( allowInterrupt ) { }
    RecursiveLock::RecursiveLock( RecursiveMutex & mutex, std::try_to_lock_t t, bool allowInterrupt )
        : m_mutex( mutex ), m_lock( m_mutex.getMutex( ), t ), m_allowInterrupt( allowInterrupt ) { }

    bool RecursiveLock::hasLock( ) const
        { return m_lock.owns_lock( );  }

    void RecursiveLock::lock( )
        { m_lock.lock( ); }
    void RecursiveLock::unlock( )
        { m_lock.unlock( ); }

    void RecursiveLock::wait( )
    { 
        if ( m_allowInterrupt )
            { Thread::enterWait( [=]( ) { m_mutex.notifyAll( ); } ); }
        m_mutex.getCvar( ).wait( m_lock ); 
        if ( m_allowInterrupt )
            { Thread::leaveWait( ); }
    }

    std::cv_status RecursiveLock::waitUntil( Time time )
    {
        if ( m_allowInterrupt )
            { Thread::enterWait( [=]( ) { m_mutex.notifyAll( ); } ); }
        std::cv_status status = m_mutex.getCvar( ).wait_until( m_lock, (std::chrono::steady_clock::time_point)time );
        if ( m_allowInterrupt )
            { Thread::leaveWait( ); }
        return status;
    }

    void RecursiveLock::notifyOne( )
        { m_mutex.notifyOne( ); }
    void RecursiveLock::notifyAll( )
        { m_mutex.notifyAll( ); }



    RecursiveLock RecursiveMutex::lock( bool allowInterrupt )
        { return RecursiveLock( *this, allowInterrupt ); }
    RecursiveLock RecursiveMutex::tryLock( bool allowInterrupt )
        { return RecursiveLock( *this, std::try_to_lock_t{ }, allowInterrupt ); }

    void RecursiveMutex::notifyOne( )
        { m_cvar.notify_one( ); }
    void RecursiveMutex::notifyAll( )
        { m_cvar.notify_all( ); }

    std::recursive_mutex & RecursiveMutex::getMutex( )
        { return m_mutex; }
    std::condition_variable_any & RecursiveMutex::getCvar( )
        { return m_cvar; }

}

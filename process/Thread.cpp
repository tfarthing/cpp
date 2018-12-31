#ifndef TEST

#include "Platform.h"
#include "Thread.h"

namespace cpp
{
    thread_local Thread::Info::ptr_t Thread::s_info = std::make_shared<Info>( );

    void setThreadName( const char * threadName )
    {
        struct THREADNAME_INFO
        {
            DWORD dwType; // must be 0x1000
            LPCSTR szName; // pointer to name (in user addr space)
            DWORD dwThreadID; // thread ID (-1=caller thread)
            DWORD dwFlags; // reserved for future use, must be zero
        };

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = threadName;
        info.dwThreadID = -1;
        info.dwFlags = 0;

        __try
        {
            RaiseException( 0x406D1388, 0, sizeof( info ) / sizeof( DWORD ), (ULONG_PTR*)&info );
        }
        __except ( EXCEPTION_CONTINUE_EXECUTION )
        {
        }
    }

    String Thread::name( )
    {
        return s_info->m_name;
    }

    void Thread::setName( String name )
    {
        s_info->m_name = name;
        setThreadName( name.c_str( ) );
    }

}

#else

#include <functional>
#include <cpp/Exception.h>
#include <cpp/chrono/Duration.h>
#include <cpp/process/Thread.h>
#include <cpp/meta/Unittest.h>

SUITE(Thread)
{
	using namespace cpp;

	TEST(waitFor)
	{
        bool flag = false;
		Mutex mutex;

		std::function<void()> fn = [ &mutex, &flag ]( )
		{
			auto lock = mutex.lock();
            lock.waitFor( Duration::ofSeconds( 1 ), [ &flag ]( ) { return flag; } );
		};

        //  acquire the lock which will block the thread at its start
		auto lock = mutex.lock();
        //  start the thread
        Thread t{ fn };
        //  set the flag and notify the thread
        flag = true;
        lock.unlock( );
        lock.notifyAll( );
        //  wait for the thread to complete
        t.join( );
	}

    TEST( interruptWait )
    {
        bool flag = false;
        Mutex mutex;

        std::function<void( )> fn = [&mutex, &flag]( )
        {
            auto lock = mutex.lock( );
            lock.waitFor( Duration::ofSeconds( 1 ), [&flag]( ) { return flag; } );
        };

        //  acquire the lock which will block the thread at its start
        auto lock = mutex.lock( );
        //  start the thread
        Thread t{ fn };
        t.interrupt( );
        //  set the flag and notify the thread
        //  flag = true;
        lock.unlock( );
        lock.notifyAll( );
        //  wait for the thread to complete
        bool result = false;
        try
            { t.join( ); }
        catch ( InterruptException & )
            { result = true; }
        CHECK( result == true );
    }

    TEST( interruptSleep )
    {
        bool flag = false;
        Mutex mutex;

        std::function<void( )> fn = [&mutex, &flag]( )
        {
            Thread::sleep( Duration::ofSeconds( 5 ) );
        };

        //  acquire the lock which will block the thread at its start
        auto lock = mutex.lock( );
        //  start the thread
        Thread t{ fn };
        t.interrupt( );
        //  set the flag and notify the thread
        //  flag = true;
        lock.unlock( );
        lock.notifyAll( );
        //  wait for the thread to complete
        bool result = false;
        try
            { t.join( ); }
        catch ( InterruptException & )
            { result = true; }
        CHECK( result == true );
    }

}

#endif
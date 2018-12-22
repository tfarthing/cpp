#ifndef TEST

#else


#include <functional>
#include <atomic>
#include <cpp/process/ThreadGroup.h>
#include <cpp/meta/Unittest.h>

SUITE( ThreadGroup )
{
    using namespace cpp;

    TEST( ctor )
    {
        Mutex mutex;
        auto lock = mutex.lock( );
        std::atomic_int count = 0;
        ThreadGroup threads( 10, [&count, &mutex]() 
            { 
                auto lock2 = mutex.lock( );
                count++; 
                lock2.unlock( );
                lock2.notifyAll( );

                Thread::sleep( Duration::ofSeconds( 10 ) );
            } );
        lock.waitFor( Duration::ofSeconds(1), [&count]( ) { return count == 10; } );
        threads.setCount(0);

        CHECK( count == 10 );
    }
}

#endif
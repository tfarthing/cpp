#pragma once

#include <list>
#include <set>
#include <map>
#include <memory>
#include <functional>
#include <cpp/chrono/Duration.h>
#include <cpp/chrono/Time.h>
#include <cpp/process/Future.h>
#include <cpp/process/ThreadGroup.h>

namespace cpp
{

    struct SyncTask_t
        { virtual bool update( ) = 0; };
    typedef std::shared_ptr<SyncTask_t> SyncTask;

    class Bus
    {
    public:
        Bus( nullptr_t );
        Bus( int threadCount = 0, String threadName = "Bus" );

        typedef std::function<void( )> Call;
        typedef std::list<Call> CallList;

        void update( );

        void threads( int threadCount );

        bool isIdle( );
        bool hasSync( );
        bool hasAsync( );

        void setErrorHandler( std::function<void( const std::exception & )> fn );

        void sync( Call fn, Duration delay = 0 );
        Future<void> async( Call fn, Duration delay = 0 );
        template<class T> Future<T> async( std::function<T( )> fn, Duration delay = 0 );

        //  block caller but call synchronously with the Bus::update() call.
        void invoke( Call fn );
        //  block caller but call synchronously with the Bus::update() call and return result to caller.
        template<class T, class Function, class ...Args> T invoke( Function fn, Args && ...args );

        void syncTask( SyncTask task );

    private:
        class Detail;
        std::shared_ptr<Detail> m_detail;
    };

    template<class T> 
    Future<T> Bus::async( std::function<T( )> fn, Duration delay )
    {
        Promise<T> promise = Promise<T>::create();

        async( [=]( ) mutable
            { 
                try
                    { promise.setResult( fn( ) );  }
                catch (...)
                    { promise.setException( std::current_exception( ) ); }
            }, delay );
        
        return promise.getFuture( );
    }

    template<class T, class Function, class ...Args> 
    T Bus::invoke( Function fn, Args && ...args )
    {
        Mutex mutex;
        auto lock = mutex.lock( );
        
        T result;
        bool isComplete = false;

        sync( [&mutex, &result, &isComplete, &fn, &args...]( )
            { 
                result = fn( args... );

                auto lock = mutex.lock( );
                isComplete = true;
                lock.unlock( );
                lock.notifyAll( ); 
            } );
        
        while ( !isComplete )
            { lock.wait( ); }

        return result;
    }

}
#ifndef TEST

#include <cpp/process/Bus.h>

namespace cpp
{

    class Bus::Detail
    {
    public:
        Detail( int threadCount, String threadName );
        ~Detail( );

        bool poll( );
        void update( );

        void threads( int threadCount );

        bool isIdle( );
        bool hasSync( );
        bool hasAsync( );

        void setErrorHandler( std::function<void( const std::exception & )> fn );

        void sync( Call fn, Duration delay );
        Future<void> async( Call fn, Duration delay );
        template<class T> Future<T> async( std::function<T( )> fn, Duration delay );

        void invoke( Call fn );
        template<class T, class Function, class ...Args> T invoke( Function fn, Args && ...args );

        void syncTask( SyncTask task );

        void worker( );
        void setBusy( );
        void setIdle( );

    private:
        ThreadGroup m_threads;
        Mutex m_asyncMutex;
        Mutex m_syncMutex;
        int m_asyncIdle;
        int m_asyncBusy;
        std::map<Time, CallList> m_asyncMap;
        std::map<Time, CallList> m_syncMap;
        std::set<SyncTask> m_syncTasks;
        std::function<void( const std::exception & )> m_errorFn;
    };

    Bus::Detail::Detail( int threadCount, String threadName )
        : m_threads( 0, [this]( ) { worker( ); }, threadName ), m_asyncIdle( 0 ), m_asyncBusy( threadCount ) 
    { 
        threads( threadCount ); 
    }

    Bus::Detail::~Detail( )
    {
        threads( 0 );
    }

    void Bus::Detail::update( )
    {
        auto lock = m_syncMutex.lock( );

        std::vector<SyncTask> tasks{ m_syncTasks.begin(), m_syncTasks.end() };
        std::vector<CallList> callLists;
        while ( !m_syncMap.empty() && m_syncMap.begin()->first <= Time::now() )
            { callLists.push_back( std::move( m_syncMap.begin( )->second ) ); m_syncMap.erase( m_syncMap.begin( ) ); }

        lock.unlock();

        std::vector<SyncTask> completeTasks;
        for ( auto & task : tasks )
        { 
            try
            {
                if ( task->update( ) ) 
                    { completeTasks.push_back( task ); } 
            }
            catch ( std::exception & e )
            {
                if ( m_errorFn )
                    { m_errorFn( e ); }
                completeTasks.push_back( task );
            }
        }
        for ( auto & callList : callLists )
        {
            for ( auto & call : callList )
            { 
                try
                    { call();  }
                catch ( std::exception & e )
                    { if ( m_errorFn ) { m_errorFn( e ); } }
            }
        }

        lock.lock();
        for ( auto & task : completeTasks )
            { m_syncTasks.erase( task ); }
    }

    void Bus::Detail::threads( int threadCount )
    {
        m_threads.setCount( threadCount );
    }

    bool Bus::Detail::isIdle( )
    {
        return !hasSync( ) && !hasAsync( );
    }
    bool Bus::Detail::hasSync( )
    {
        auto lock = m_syncMutex.lock( ); return !m_syncTasks.empty( ) || !m_syncMap.empty( );
    }
    bool Bus::Detail::hasAsync( )
    {
        auto lock = m_asyncMutex.lock( ); return m_asyncBusy > 0 || !m_asyncMap.empty( );
    }

    void Bus::Detail::setErrorHandler( std::function<void( const std::exception & )> fn )
    {
        m_threads.setErrorHandler( fn );
        m_errorFn = std::move( fn );
    }


    void Bus::Detail::sync( Call fn, Duration delay)
    {
        auto lock = m_syncMutex.lock( );
        m_syncMap[Time::now( ) + delay].push_back( std::move( fn ) );
    }

    Future<void> Bus::Detail::async( Call fn, Duration delay )
    {
        Promise<void> promise = Promise<void>::create();
        Call call = [=]() mutable
        { 
            try
                { fn( ); promise.setResult( );  }
            catch (...)
                { promise.setException( std::current_exception( ) ); }
        };

        auto lock = m_asyncMutex.lock( );
        m_asyncMap[Time::now( ) + delay].push_back( std::move( call ) );
        lock.unlock( );
        lock.notifyOne( );

        return promise.getFuture( );
    }

    void Bus::Detail::invoke( Call fn )
    {
        Mutex mutex;
        auto lock = mutex.lock( );

        bool isComplete = false;

        sync( [&mutex, &fn, &isComplete]( )
        {
            fn( );

            auto lock = mutex.lock( );
            isComplete = true;
            lock.unlock( );
            lock.notifyAll( );
        }, 0 );

        while ( !isComplete )
            { lock.wait( ); }
    }

    void Bus::Detail::syncTask( SyncTask task )
    {
        auto lock = m_syncMutex.lock( );
        m_syncTasks.insert( std::move( task ) );
    }

    void Bus::Detail::worker( )
    {
        auto lock = m_asyncMutex.lock( );

        while ( !Thread::isInterrupted() )
        {
            //  if no async ready, wait
            while ( m_asyncMap.empty() )
                { setIdle(); lock.wait(); setBusy(); }
            
            auto itr = m_asyncMap.begin();
            const Time & time = itr->first;

            //  if earliest async is in the future, wait until then
            if ( time > Time::now() )
                { setIdle(); lock.waitUntil( time ); setBusy( ); continue; }

            //  remove the Call from the callMap
            auto & callList = itr->second;
            Call call = std::move(callList.front()); callList.pop_front();
            if (callList.empty())
                { m_asyncMap.erase( itr ); }
            
            lock.unlock();

            //  make the call
            String threadName = Thread::name( );
            try
                { call(); }
            catch ( std::exception & e )
                { if ( m_errorFn ) { m_errorFn( e ); } }
            lock.lock( );

            if ( threadName != Thread::name( ) )
                { Thread::setName( threadName ); }
        }

        m_asyncBusy--;
    }

    void Bus::Detail::setBusy( )
        { m_asyncIdle--; m_asyncBusy++; }

    void Bus::Detail::setIdle( )
        { m_asyncBusy--; m_asyncIdle++; }




    Bus::Bus( nullptr_t )
        : m_detail( nullptr ) { }

    Bus::Bus( int threadCount, String threadName )
        : m_detail( std::make_shared<Detail>( threadCount, threadName ) ) { }

    void Bus::setErrorHandler( std::function<void( const std::exception & )> fn )
        { m_detail->setErrorHandler( std::move(fn) ); }

    void Bus::update( )
        { m_detail->update( ); }

    void Bus::threads( int threadCount )
        { m_detail->threads( threadCount ); }

    void Bus::sync( Call fn, Duration delay )
        { m_detail->sync( fn, delay ); }

    Future<void> Bus::async( Call fn, Duration delay )
        { return m_detail->async( fn, delay ); }

    void Bus::invoke( Call fn )
        { m_detail->invoke( fn ); }

    void Bus::syncTask( SyncTask task )
        { m_detail->syncTask( task ); }

}

#else

#include <cpp/process/Bus.h>
#include <cpp/util/Handle.h>
#include <cpp/meta/Unittest.h>

SUITE( Bus )
{
    using namespace cpp;

    TEST( async )
    {
        std::function<int( )> fn = []( )
        {
            return 10;
        };

        Bus bus{1};
        auto result1 = bus.async( fn );
        auto result2 = bus.async<int>( [=]() { return fn() * fn(); } );

        CHECK( result1.get() + result2.get() == 110 );
    }

    TEST( syncTask )
    {
        int value = 0;

        std::function<int( )> fn = [&value]( )
        {
            return value * 10;
        };

        struct ExponentTask_t
            : public SyncTask_t
        {
            ExponentTask_t( int value, int exponent )
                : m_value(value), m_exponent( exponent ) { }

            bool update( )
            {
                if ( m_exponent > 0 )
                    { m_result *= m_value; m_exponent--; }
                else if ( m_exponent < 0 )
                    { m_result /= m_value; m_exponent++; }
                return m_exponent == 0;
            }

            double result() const
                { return m_result; }

            double m_result = 1;
            int m_value = 0;
            int m_exponent = 0;
        };
        typedef Handle<ExponentTask_t> ExponentTask;

        ExponentTask task1{ 10, 3 };
        ExponentTask task2{ 10, -3 };
        ExponentTask task3{ 0, 3 };
        ExponentTask task4{ 0, -3 };

        Bus bus;
        bus.syncTask( task1 );
        bus.syncTask( task2 );
        bus.syncTask( task3 );
        bus.syncTask( task4 );

        while ( bus.hasSync() )
            { bus.update(); }

        CHECK( task1->result() == 1000.0 );
        CHECK( task2->result( ) == 0.001 );
        CHECK( task3->result( ) == 0.0 );
        CHECK( isinf( task4->result( ) ) );

        CHECK( bus.exceptions().empty() );
    }

    TEST( invoke )
    {

        Bus bus;
        std::atomic_int result = 0;

        Thread thread( [&]() 
            {
                result = bus.invoke<int>( []()->int { return 10 + 20; } );
            });
        while ( !bus.hasSync() )
            { Thread::sleep( Duration::ofMillis(50) ); }

        while ( bus.hasSync() )
            { bus.update(); }

        CHECK ( result == 30 );
    }

}

#endif
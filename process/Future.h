#pragma once

#include <memory>
#include <future>
#include <cpp/Exception.h>
#include <cpp/process/Lock.h>

namespace cpp
{

    struct FutureException 
        : public Exception
    {
        FutureException( std::future_errc error )
            : Exception( "Thread execution was interrupted." ), m_error( error ) { }

        std::future_errc error( ) const
            { return m_error; }

        std::future_errc m_error;
    };

    template<class T>
    class PromisedState
    {
    public:
        typedef std::shared_ptr<PromisedState> ptr_t;

        PromisedState( )
            {}
        PromisedState( std::function<bool( )> cancelHandler )
            : m_cancelHandler( std::move( cancelHandler ) ) { }

        //  called by future
        T & get( )
            { auto lock = m_mutex.lock( ); lock.wait( predicate( ) ); if ( m_exception ) { std::rethrow_exception( m_exception ); } return *m_result; }
        bool cancel( )
            { auto lock = m_mutex.lock( ); return ( m_cancelHandler ) ? m_cancelHandler() : false; }
        bool isDone( )
            { auto lock = m_mutex.lock( ); return hasResult( ); }

        void wait( )
            { auto lock = m_mutex.lock( ); lock.wait( predicate( ) ); }
        bool waitFor( Duration duration )
            { auto lock = m_mutex.lock( ); return lock.waitFor( duration, predicate( ) ); }
        bool waitUntil( Time time )
            { auto lock = m_mutex.lock( ); return lock.waitUntil( time, predicate( ) ); }

        //  called by promise
        void setResult( const T & value )
            { auto lock = m_mutex.lock( ); m_result = std::make_unique<T>( value ); lock.unlock( ); lock.notifyAll( ); }
        void setResult( T && value )
            { auto lock = m_mutex.lock( ); m_result = std::make_unique<T>( std::forward( value ) ); lock.unlock( ); lock.notifyAll( ); }
        void setException( std::exception_ptr e )
            { auto lock = m_mutex.lock( ); m_exception = e; lock.unlock( ); lock.notifyAll( ); }

    private:
        bool hasResult( )
            { return m_result != nullptr || m_exception != nullptr; }
        std::function<bool()> predicate()
            { return [this]( ) { return hasResult( ); }; }

    private:
        Mutex m_mutex;
        std::function<bool( )> m_cancelHandler;
        std::unique_ptr<T> m_result;
        std::exception_ptr m_exception;
    };

    template<>
    class PromisedState<void>
    {
    public:
        typedef std::shared_ptr<PromisedState> ptr_t;

        PromisedState( )
            : m_hasResult( false ) {}
        PromisedState( std::function<bool( )> cancelHandler )
            : m_cancelHandler( std::move( cancelHandler ) ), m_hasResult(false) { }

        //  called by future
        void get( )
            { auto lock = m_mutex.lock( ); lock.wait( predicate( ) ); if ( m_exception ) { std::rethrow_exception( m_exception ); } }
        bool cancel( )
            { auto lock = m_mutex.lock( ); return ( m_cancelHandler ) ? m_cancelHandler() : false; }
        bool isDone( )
            { auto lock = m_mutex.lock( ); return hasResult( ); }

        void wait( )
            { auto lock = m_mutex.lock( ); lock.wait( predicate( ) ); }
        bool waitFor( Duration duration )
            { auto lock = m_mutex.lock( ); return lock.waitFor( duration, predicate( ) ); }
        bool waitUntil( Time time )
            { auto lock = m_mutex.lock( ); return lock.waitUntil( time, predicate( ) ); }

        //  called by promise
        void setResult( )
            { auto lock = m_mutex.lock( ); m_hasResult = true; lock.unlock( ); lock.notifyAll( ); }
        void setException( std::exception_ptr e )
            { auto lock = m_mutex.lock( ); m_exception = e; lock.unlock( ); lock.notifyAll( ); }

    private:
        bool hasResult( )
            { return m_hasResult || m_exception != nullptr; }
        std::function<bool()> predicate()
            { return [this]( ) -> bool { return hasResult( ); }; }

    private:
        Mutex m_mutex;
        std::function<bool( )> m_cancelHandler;
        bool m_hasResult;
        std::exception_ptr m_exception;
    };

    template<class T> class Promise;

    template<class T>
    class Future
    {
    public:
        Future( )
            : m_state( ) { }
        Future( Future<T> && move)
            : m_state( std::move( move.m_state ) ) { }
        Future & operator=( Future<T> && move )
            { m_state = std::move(move.m_state); return *this; }

        Future( const Future<T> & copy ) = delete;
        Future & operator=( const Future<T> & copy ) =delete;

        bool isValid( ) const
            { return m_state != nullptr; }

        T & get()
            { checkIsValid( ); return m_state->get( ); }

        bool isDone( )
            { checkIsValid( ); return m_state->isDone(); }
        void wait( )
            { checkIsValid( ); m_state->wait( ); }
        bool waitFor( Duration duration )
            { checkIsValid( ); return m_state->waitFor( duration ); }
        bool waitUntil( Time time )
            { checkIsValid( ); return m_state->waitUntil( time ); }

        void reset( )
            { m_state.reset( ); }

    private:
        friend class Promise<T>;
        typedef typename PromisedState<T>::ptr_t state_t;
        Future( state_t state )
            : m_state( std::move(state) ) { }

        void checkIsValid( )
            { if ( !isValid( ) ) { throw FutureException{ std::future_errc::no_state }; } }

    private:
        state_t m_state;
    };

    template<>
    class Future<void>
    {
    public:
        Future()
            : m_state( ) { }
        Future( Future<void> && move)
            : m_state( std::move( move.m_state ) ) { }
        Future & operator=( Future<void> && move )
            { m_state = std::move(move.m_state); return *this; }

        Future( const Future<void> & copy ) = delete;
        Future & operator=( const Future<void> & copy ) =delete;

        bool isValid( ) const
            { return m_state != nullptr; }

        void get()
            { checkIsValid( ); m_state->get( ); }

        bool isDone( )
            { checkIsValid( ); return m_state->isDone(); }
        void wait( )
            { checkIsValid( ); m_state->wait( ); }
        bool waitFor( Duration duration )
            { checkIsValid( ); return m_state->waitFor( duration ); }
        bool waitUntil( Time time )
            { checkIsValid( ); return m_state->waitUntil( time ); }

        void reset( )
            { m_state.reset( ); }

    private:
        friend class Promise<void>;
        typedef PromisedState<void>::ptr_t state_t;

        Future( state_t state )
            : m_state( std::move( state ) ) { }

         void checkIsValid( )
            { if ( !isValid( ) ) { throw FutureException{ std::future_errc::no_state }; } }

    private:
        state_t m_state;
    };

    template<class T>
    class Promise
    {
    public:
        static Promise<T> create( )
            { return Promise{ std::make_shared< PromisedState<T> >( ) }; }
        static Promise<T> create( std::function<bool()> cancelHandler )
            { return Promise{ std::make_shared< PromisedState<T> >( std::move( cancelHandler ) ) }; }

        Promise( )
            : m_state( ) { }

        Future<T> getFuture( )
            { return Future<T>{ m_state }; }

        void setResult( const T & data )
            { m_state->setResult( data ); }
        void setResult( T&& data )
            { m_state->setResult( data ); }
        void setException( std::exception_ptr e )
            { m_state->setException( e ); }

        void reset( )
            { m_state.reset( ); }

    private:
        Promise( typename PromisedState<T>::ptr_t && state)
            : m_state( std::move(state) ) { }

    private:
        typedef typename PromisedState<T>::ptr_t state_t;
        state_t m_state;
    };

    template<>
    class Promise<void>
    {
    public:
        static Promise<void> create()
            { return Promise{ std::make_shared< PromisedState<void> >( ) }; }
        static Promise<void> create( std::function<bool( )> cancelHandler )
            { return Promise{ std::make_shared< PromisedState<void> >( std::move( cancelHandler ) ) }; }

        Promise()
            : m_state( ) { }

        Future<void> getFuture( )
            { return Future<void>{ m_state }; }

        void setResult( )
            { m_state->setResult( ); }
        void setException( std::exception_ptr e )
            { m_state->setException( e ); }

        void reset( )
            { m_state.reset( ); }

    private:
        Promise( PromisedState<void>::ptr_t && state )
            : m_state( std::move( state ) ) { }

    private:
        typedef PromisedState<void>::ptr_t state_t;
        state_t m_state;
    };

}

#pragma once

#include <cassert>
#include <thread>
#include <future>
#include <chrono>
#include <atomic>
#include <cpp/Exception.h>
#include <cpp/chrono/Duration.h>
#include <cpp/process/Lock.h>
    
namespace cpp
{

	class Thread
	{
	public:
        Thread( )
            : m_thread(), m_info(nullptr) { }
		
        template< class Function, class... Args > 
		Thread(Function&& f, Args&&... args);
        
        Thread( Thread && move )
            : m_thread( std::move( move.m_thread ) ), m_info( move.m_info ) { }
        
        Thread & operator=( Thread && move )
        {
            reset( );

            m_thread = std::move( move.m_thread );
            m_info = std::move( move.m_info );
            return *this;
        }

		~Thread()
			{ reset(); }
        bool isRunning() const
            { return m_thread.joinable(); }
		void join( bool checkFlag = false )
            { if ( isRunning( ) ) { m_thread.join( ); } if ( checkFlag ) { check( ); } }
		void interrupt()
            { if ( m_info ) { m_info->interrupt(); } }
        void check()
            { if ( m_info ) { m_info->checkException(); } }
        void detach( )
            { if ( isRunning( ) ) { m_thread.detach( ); m_info = nullptr; } }
        void reset()
        { 
            if ( isRunning( ) )
            {
                interrupt( );
                if ( Thread::id() != m_thread.get_id() )
                    { m_thread.join( ); }
                else
                    { m_thread.detach( ); }
                m_info = nullptr;
            }
        }

    public:
        typedef std::thread::id id_t;

        static id_t id( )
            { return std::this_thread::get_id( ); }
        static void yield( )
            { checkInterrupt( ); std::this_thread::yield( ); checkInterrupt( ); }
        static void sleep( const Duration & duration )
            { Mutex mutex; auto lock = mutex.lock( ); lock.waitFor( duration ); }
        static bool isInterrupted( )
            { return s_info->m_isInterrupted; }
        static void checkInterrupt( )
            { s_info->checkInterrupt( ); }
        static void clearInterrupt( )
            { s_info->m_isInterrupted = false; }
        static String name( );
        static void setName( String name );

    private:
        friend class Lock;
        friend class RecursiveLock;
        static void enterWait( LockEvent * waitEvent )
            { assert( s_info->m_waitEvent == nullptr ); s_info->m_waitEvent = waitEvent; checkInterrupt( ); }
        static void leaveWait()
            { assert( s_info->m_waitEvent != nullptr ); s_info->m_waitEvent = nullptr; checkInterrupt( ); }

    private:
        struct Info
        {
            typedef std::shared_ptr<Info> ptr_t;

            Info( )
                : m_exception(nullptr), m_isInterrupted( false ), m_waitEvent( nullptr ) { }
            void interrupt( )
                { m_isInterrupted.store( true ); if ( m_waitEvent ) { m_waitEvent->notifyAll( ); } }
            void checkInterrupt( )
                { if ( m_isInterrupted ) { m_waitEvent = nullptr; throw InterruptException{ }; } }
            void checkException() 
                { if ( m_exception ) { auto e = std::move( m_exception ); m_exception = nullptr; std::rethrow_exception( e ); } }
            String m_name;
            std::exception_ptr m_exception;
            std::atomic_bool m_isInterrupted;
            LockEvent * m_waitEvent;
        };

	private:
		std::thread m_thread;
        Info::ptr_t m_info;
        static thread_local Info::ptr_t s_info;
	};

	template< class Function, class... Args >
	Thread::Thread(Function&& f, Args&&... args)
		: m_info(nullptr)
	{
        Mutex mutex;
		auto lock = mutex.lock( false );
        m_thread = std::thread{ [&mutex, this, f, args...]( )
        {
            auto lock = mutex.lock( false );
            m_info = s_info; 
            lock.unlock( ); 
            lock.notifyAll( );

            try
                { f( args... ); }
            catch ( ... )
                { s_info->m_exception = std::current_exception( ); }
        } };
        while ( !m_info )
            { lock.wait( ); }
	}

}
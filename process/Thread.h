#pragma once

/*

	Thread is an encapsulation of std::thread which also provides the
	ability to interrupt a thread.
	(1) Can interrupt a thread which will cause an InterruptException if
		the interrupted thread calls yield(), sleep(), or locks a Mutex.
	(2) Can check for an unhandled exception in the thread using check();
	(3) Can name the thread for debugging purposes.

*/

#include <cassert>
#include <thread>
#include <future>
#include <chrono>
#include <atomic>

#include "../time/Time.h"
#include "Exception.h"
#include "Lock.h"
    
namespace cpp
{

	class Thread
	{
	public:
												template< class Function, class... Args >
												Thread( Function&& f, Args&&... args );
		
												Thread( );
												Thread( Thread && move );
												~Thread( );

		Thread &								operator=( Thread && move );

		bool									isRunning( ) const;
		void									join( );
		void									interrupt( );
		void									check( );
		void									detach( );
		void									reset( );

        typedef std::thread::id					id_t;

		static id_t								id( );
		static void								yield( );
		static void								sleep( const Duration & duration );
		static bool								isInterrupted( );
		static void								checkInterrupt( );
		static void								clearInterrupt( );
        static std::string						name( );
        static void								setName( std::string name );

    private:
        friend class Lock;
        friend class RecursiveLock;
		static void								enterWait( std::function<void( )> waitEvent );
		static void								leaveWait( );

    private:
        struct Info
        {
            typedef std::shared_ptr<Info>		ptr_t;

												Info( );
			void								interrupt( );
			void								checkInterrupt( );
			void								checkException( );

			std::string							m_name;
            std::exception_ptr					m_exception;
            std::atomic_bool					m_isInterrupted;
            std::function<void()>				m_waitEvent;
        };

	private:
		std::thread								m_thread;
        Info::ptr_t								m_info;
        static thread_local Info::ptr_t			s_info;
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


	inline Thread::Thread( )
		: m_thread( ), m_info( nullptr ) { }


	inline Thread::Thread( Thread && move )
		: m_thread( std::move( move.m_thread ) ), m_info( move.m_info ) { }


	inline Thread & Thread::operator=( Thread && move )
	{
		reset( );

		m_thread = std::move( move.m_thread );
		m_info = std::move( move.m_info );
		return *this;
	}


	inline Thread::~Thread( )
	{
		reset( );
	}


	inline bool Thread::isRunning( ) const
	{
		return m_thread.joinable( );
	}


	inline void Thread::join( )
	{
		if ( isRunning( ) ) 
			{ m_thread.join( ); } 
	}


	inline void Thread::interrupt( )
	{
		if ( m_info ) 
			{ m_info->interrupt( ); }
	}


	inline void Thread::check( )
	{
		if ( m_info ) 
			{ m_info->checkException( ); }
	}


	inline void Thread::detach( )
	{
		if ( isRunning( ) ) 
			{ m_thread.detach( ); m_info = nullptr; }
	}


	inline void Thread::reset( )
	{
		if ( isRunning( ) )
		{
			interrupt( );
			if ( Thread::id( ) != m_thread.get_id( ) )
			{
				m_thread.join( );
			}
			else
			{
				m_thread.detach( );
			}
			m_info = nullptr;
		}
	}


	inline Thread::id_t Thread::id( )
		{ return std::this_thread::get_id( ); }


	inline void Thread::yield( )
		{ checkInterrupt( ); std::this_thread::yield( ); checkInterrupt( ); }


	inline void Thread::sleep( const Duration & duration )
		{ Mutex mutex; auto lock = mutex.lock( ); lock.waitFor( duration ); }


	inline bool Thread::isInterrupted( )
		{ return s_info->m_isInterrupted; }


	inline void Thread::checkInterrupt( )
		{ s_info->checkInterrupt( ); }


	inline void Thread::clearInterrupt( )
		{ s_info->m_isInterrupted = false; }


	inline void Thread::enterWait( std::function<void( )> waitEvent )
	{ 
		assert( s_info->m_waitEvent == nullptr ); 
		s_info->m_waitEvent = std::move( waitEvent ); 
		checkInterrupt( ); 
	}


	inline void Thread::leaveWait( )
	{
		assert( s_info->m_waitEvent != nullptr ); 
		s_info->m_waitEvent = nullptr; 
		checkInterrupt( );
	}


	inline Thread::Info::Info( )
		: m_exception( nullptr ), m_isInterrupted( false ), m_waitEvent( nullptr ) 
	{ 
	}


	inline void Thread::Info::interrupt( )
	{
		m_isInterrupted.store( true ); 
		if ( m_waitEvent ) 
			{ m_waitEvent( ); }
	}


	inline void Thread::Info::checkInterrupt( )
	{
		if ( m_isInterrupted ) 
			{ m_waitEvent = nullptr; throw InterruptException{ }; }
	}


	inline void Thread::Info::checkException( )
	{
		if ( m_exception ) 
		{ 
			auto e = std::move( m_exception ); 
			m_exception = nullptr; 
			std::rethrow_exception( e ); 
		}
	}

}
#pragma once

/*

	Provides abstraction for asio executor model.  
	
	AsyncTimer is an encapsulated asio::steady_timer
	(1) Can be cancelled or go out of scope without user-handler being subsequently called.

	AsyncIO is a thin wrapper for asio::io_context
	(1) Allocates io_context with shared_ptr so that users can optionally own the context or 
		not.  The value is copyable instead of a reference.
	(2) Provides AsyncIO::Timer abstraction of steady_timer, using shared_ptr to simplify 
		ownership of the timer context and protect against deallocation before event callback.

*/

#include <chrono>
#include <functional>

#include "Lock.h"

#include "Platform.h"
#define ASIO_STANDALONE
#include <asio/asio.hpp>

namespace cpp
{

	class AsyncTimer
	{
	public:
		typedef std::chrono::milliseconds duration_t;
		typedef std::chrono::steady_clock::time_point time_point_t;

		static AsyncTimer					waitFor( 
												asio::io_context * context, 
												duration_t timeout,
												std::function<void( )> handler );
		static AsyncTimer					waitUntil( 
												asio::io_context * context, 
												time_point_t timeout,
												std::function<void( )> handler );

											AsyncTimer( );
											AsyncTimer( AsyncTimer && move );
											~AsyncTimer( );

		AsyncTimer &						operator=( AsyncTimer && move);

		void								cancel( );

	private:
		void								start( 
												asio::io_context * context, 
												time_point_t & timeout,
												std::function<void( )> handler );

	private:
		struct Detail;
		std::shared_ptr<Detail>				detail;
	};



	class AsyncIO
	{
	public:
											AsyncIO( );
											~AsyncIO( );

		asio::io_context &					context( );

		AsyncTimer							waitFor( std::chrono::milliseconds timeout, std::function<void( )> handler );
		AsyncTimer							waitUntil( std::chrono::steady_clock::time_point timeout, std::function<void( )> handler );

		//  block caller but call synchronously with the AsyncIO::run() call.
		void								invoke( std::function<void()> fn );
		
		template<class T, class Fn, class ...Args> 
		T									invoke( Fn fn, Args && ...args );

	private:
		std::shared_ptr<asio::io_context>	io;
	};



	AsyncTimer AsyncTimer::waitFor( asio::io_context * context, duration_t timeout, std::function<void( )> handler )
	{
		return waitUntil( context, std::chrono::steady_clock::now( ) + timeout, std::move( handler ) );
	}


	AsyncTimer AsyncTimer::waitUntil( asio::io_context * context, time_point_t timeout, std::function<void( )> handler )
	{
		AsyncTimer timer;
		timer.start( context, timeout, std::move( handler ) );
		return timer;
	}
	


	void AsyncIO::invoke( std::function<void( )> fn )
	{
        Mutex mutex;
        auto lock = mutex.lock( );

        bool isComplete = false;

        io->post( [&mutex, &fn, &isComplete]( )
        {
            fn( );

            auto lock = mutex.lock( );
            isComplete = true;
            lock.unlock( );
            lock.notifyAll( );
        } );

        while ( !isComplete )
            { lock.wait( ); }
	}


	template<class T, class Function, class ...Args> T AsyncIO::invoke( Function fn, Args && ...args )
	{
		Mutex mutex;
		auto lock = mutex.lock( );

		T result;
		bool isComplete = false;

		io->post( [&mutex, &result, &isComplete, &fn, &args...]( )
		{
			result = fn( args... );

			auto lock = mutex.lock( );
			isComplete = true;
			lock.unlock( );
			lock.notifyAll( );
		} );

		while ( !isComplete )
		{
			lock.wait( );
		}

		return result;
	}

}
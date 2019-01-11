#pragma once

/*

	Provides abstraction for asio executor model.  
	
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
		: public std::enable_shared_from_this<AsyncTimer>
	{
	public:
		void start( asio::io_context * context, std::chrono::steady_clock::time_point & timeout, std::function<void( )> handler );
		void cancel( );

	private:
		std::shared_ptr<asio::steady_timer> timer;
	};




	class AsyncIO
	{
	public:
		AsyncIO( );
		~AsyncIO( );

		asio::io_context & context( );

		//  block caller but call synchronously with the AsyncIO::run() call.
		void invoke( std::function<void()> fn );
		template<class T, class Function, class ...Args> T invoke( Function fn, Args && ...args );

		AsyncTimer timer( std::chrono::milliseconds timeout, std::function<void( )> handler );
		AsyncTimer timer( std::chrono::steady_clock::time_point timeout, std::function<void( )> handler );

	private:
		std::shared_ptr<asio::io_context> io;
	};



	void AsyncTimer::start( asio::io_context * context, std::chrono::steady_clock::time_point & timeout, std::function<void( )> handler )
	{
		auto self = shared_from_this( );
		timer = std::make_shared<asio::steady_timer>( *context, timeout );
		timer->async_wait( [this, self, handler]( std::error_code error )
			{
				if ( !error )
					{ handler( ); }
			} );
	}

	void AsyncTimer::cancel( )
	{
		timer->cancel( );
		timer.reset( );
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
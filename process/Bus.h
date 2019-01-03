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

#include <asio/asio.hpp>

namespace cpp
{

	class AsyncIO
	{
	public:
		AsyncIO( );
		~AsyncIO( );

		asio::io_context & context( );
		
		class Timer
			: public std::enable_shared_from_this<AsyncIO::Timer>
		{
		public:
			Timer( );

			void cancel( );
		private:
			void start( AsyncIO & asyncIO );
			void onEvent( std::error_code error );

		private:
			std::shared_ptr<asio::steady_timer> timer;
		};

		Timer timer( std::chrono::milliseconds delay, std::function<void( )> handler );

	private:
		std::shared_ptr<asio::io_context> io;
	};



	AsyncIO::Timer::Timer( )
	{

	}

	void AsyncIO::Timer::cancel( )
	{

	}

	void AsyncIO::Timer::onEvent( std::error_code error )
	{

	}

}
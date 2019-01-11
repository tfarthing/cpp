#pragma once

#include <functional>
#include <memory>
#include <chrono>

#include "AsyncIO.h"
#include "../time/Duration.h"


namespace cpp
{

	class AsyncProcess
	{
	public:
		typedef std::function<void( std::string & recvBuffer )> RecvHandler;
		typedef std::function<void( int32_t exitValue )> ExitHandler;
		typedef std::chrono::milliseconds Duration;
		typedef std::chrono::steady_clock::time_point Time;

		static AsyncProcess run(
			asio::io_context & io,
			std::string cmdline,
			std::string workingPath,
			RecvHandler onStdout,
			RecvHandler onStderr,
			ExitHandler onExit );

		static AsyncProcess runDetached(
			asio::io_context & io,
			std::string cmdline,
			std::string workingPath,
			ExitHandler onExit );

		AsyncProcess( );
		AsyncProcess( AsyncProcess && move );
		~AsyncProcess( );

		AsyncProcess & operator=( AsyncProcess && move );

		bool isRunning( ) const;
		void send( std::string msg );
		void detach( );
		void close( );

		int32_t exitValue( ) const;

	private:
		struct Detail;
		AsyncProcess( std::shared_ptr<Detail> && detail );

		std::shared_ptr<Detail> detail = nullptr;
	};


}

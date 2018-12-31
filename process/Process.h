#pragma once

#include <functional>
#include <memory>
#include <chrono>

#include "lib/asio/asio.hpp"



class Process
{
public:
	typedef std::function<void( std::string & recvBuffer )> RecvHandler;
	typedef std::function<void( int32_t exitValue )> ExitHandler;
	typedef std::chrono::milliseconds Duration;
	typedef std::chrono::steady_clock::time_point Time;

	static Process run( 
		asio::io_context & io, 
		std::string cmdline, 
		std::string workingPath, 
		RecvHandler onStdout, 
		RecvHandler onStderr,
		ExitHandler onExit );
	
	static Process runDetached( 
		asio::io_context & io, 
		std::string cmdline, 
		std::string workingPath,
		ExitHandler onExit);

	Process( );
	Process( Process && move );
	~Process( );

	Process & operator=( Process && move );

	bool isRunning( ) const;
	void send( std::string msg );
	void detach( );
	void close( );

	void wait( );
	bool waitFor( Duration timeout );
	bool waitUntil( Time time );

	int32_t exitValue( ) const;

private:
	struct Detail;
	Process( std::shared_ptr<Detail> && detail );

	std::shared_ptr<Detail> detail = nullptr;
};
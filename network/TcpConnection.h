#pragma once

#if __has_include(<openssl/ssl.h>)

#include <chrono>
#include <functional>
#include <map>

#include "../process/Exception.h"
#include "../process/AsyncIO.h"


namespace cpp
{

	class TcpConnection
	{
	public:
		typedef std::function<void( std::error_code connectResult )> ConnectHandler;
		typedef std::function<void( std::string & recvBuffer )> RecvHandler;
		typedef std::function<void( std::error_code reason )> DisconnectHandler;

		TcpConnection( );
		TcpConnection(
			asio::io_context & io,
			std::string address,
			ConnectHandler onConnect,
			RecvHandler onRecv,
			DisconnectHandler onDisconnect,
			std::string caFilename = "" );

		void connect(
			asio::io_context & io,
			std::string address,
			ConnectHandler onConnect,
			RecvHandler onRecv,
			DisconnectHandler onDisconnect,
			std::string caFilename = "" );
		void send( std::string msg );
		void disconnect( );

	private:
		class Detail;
		std::shared_ptr<Detail> detail = nullptr;
	};

}

#endif

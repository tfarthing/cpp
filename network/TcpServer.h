#pragma once

#if __has_include(<openssl/ssl.h>)

#include <string>
#include <map>

#include "../process/AsyncIO.h"
#define ASIO_STANDALONE
#include <asio/ssl.hpp>



namespace cpp
{

	class TcpServer
	{
	public:
		typedef std::function<void( std::error_code acceptError, const std::string & addr )> ConnectHandler;
		typedef std::function<void( const std::string & addr, std::string & recvBuffer )> RecvHandler;
		typedef std::function<void( const std::string & addr, std::error_code reason )> DisconnectHandler;
		enum class TcpVersion { v4, v6 };

		TcpServer( );
		TcpServer(
			asio::io_context & io,
			uint16_t port,
			ConnectHandler connectHandler,
			RecvHandler recvHandler,
			DisconnectHandler disconnectHandler,
			const std::string & address = "localhost",
			TcpVersion tcpVersion = TcpVersion::v6,
			std::string keyPEM = "",
			std::string certificatePEM = "",
			std::string dhParamsPEM = "" );
		~TcpServer( );

		void open(
			asio::io_context & io,
			uint16_t port,
			ConnectHandler connectHandler,
			RecvHandler recvHandler,
			DisconnectHandler disconnectHandler,
			const std::string & address = "localhost",
			TcpVersion tcpVersion = TcpVersion::v6,
			std::string keyPEM = "",
			std::string certificatePEM = "",
			std::string dhParamsPEM = "" );
		bool isOpen( ) const;
		void close( );

		void send( const std::string & clientAddr, std::string data );
		void disconnect( const std::string & clientAddr, std::error_code reason = {} );

	private:
		typedef asio::ip::tcp::socket TcpSocket;
		typedef std::unique_ptr<TcpSocket> TcpSocketPtr;
		typedef asio::ssl::stream<TcpSocket> TlsStream;
		typedef std::unique_ptr<TlsStream> TlsStreamPtr;

		void onAccept( std::error_code error, TlsStreamPtr stream, TcpSocketPtr socket );
		void onConnectionRecv( const std::string & clientAddr, std::string & recvBuffer );
		void onConnectionClose( const std::string & clientAddr, std::error_code errorCode );

	private:
		static const size_t RecvSize = 1 * 1024;
		typedef asio::ssl::context TlsContext;
		typedef std::unique_ptr<TlsContext> TlsContextPtr;
		typedef asio::ip::tcp::acceptor TcpAcceptor;

		class Session;
		typedef std::shared_ptr<Session> ConnectionPtr;

		class Listener;
		std::shared_ptr<Listener> listener;

		ConnectHandler connectHandler;
		RecvHandler recvHandler;
		DisconnectHandler disconnectHandler;
		bool isAcceptPending = false;
		std::map<std::string, ConnectionPtr> connections;
	};

}

#endif

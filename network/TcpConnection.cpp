#include <deque>

#if __has_include(<openssl/ssl.h>)

#include "../process/Exception.h"
#include "../process/Platform.h"
#include "TcpConnection.h"

#define ASIO_STANDALONE
#include <asio/ssl.hpp>


namespace cpp
{

	class TcpConnection::Detail
		: public std::enable_shared_from_this<TcpConnection::Detail>
	{
	public:
		Detail(
			asio::io_context & io,
			std::string address,
			ConnectHandler onConnect,
			RecvHandler onRecv,
			DisconnectHandler onDisconnect,
			std::string caFilename );
		~Detail( );

		void resolve( );

		bool isConnecting( ) const;
		bool isConnected( ) const;

		void send( std::string data );

		void disconnect( );
		void disconnect( std::error_code errorCode );

	private:
		typedef asio::ip::tcp::resolver TcpResolver;
		typedef asio::ip::tcp::endpoint TcpEndpoint;

		void connect( const TcpResolver::results_type & endpoints );
		void handshake( );
		void recv( );
		void send( );

		void onResolve( std::shared_ptr<Detail> self, std::error_code error, const TcpResolver::results_type & results );
		void onConnect( std::shared_ptr<Detail> self, std::error_code error );
		void onHandshake( std::shared_ptr<Detail> self, std::error_code error );
		void onRecv( std::shared_ptr<Detail> self, std::error_code error, std::size_t bytes );
		void onSend( std::shared_ptr<Detail> self, std::error_code error, std::size_t bytes );

		void notifyConnect( const std::error_code & error );
		void notifyRecv( );
		void notifyDisconnect( const std::error_code & reason );

	public:
		static const size_t RecvSize = 1 * 1024;
		typedef asio::ip::tcp::socket TcpSocket;
		typedef std::unique_ptr<TcpSocket> TcpSocketPtr;
		typedef asio::ssl::stream<TcpSocket> TlsStream;
		typedef std::unique_ptr<TlsStream> TlsStreamPtr;
		typedef asio::ssl::context TlsContext;

		std::string address;
		ConnectHandler connectHandler;
		RecvHandler recvHandler;
		DisconnectHandler disconnectHandler;
		std::string certificateAuthority;

		TcpResolver resolver;
		TlsContext tlsContext;
		TlsStreamPtr stream;
		TcpSocketPtr socket;

		size_t recvBytes = 0;
		std::string recvBuffer;
		std::deque<std::string> sendBuffers;
		bool isConnectPending = true;
		bool isSending = false;
		std::error_code errorCode;
	};



	TcpConnection::TcpConnection( )
	{
	}

	TcpConnection::TcpConnection( asio::io_context & io, std::string address, ConnectHandler onConnect, RecvHandler onRecv, DisconnectHandler onDisconnect, std::string caFilename )
		: detail( std::make_shared<Detail>( io, address, onConnect, onRecv, onDisconnect, caFilename ) )
	{
		detail->resolve( );
	}


	void TcpConnection::connect( asio::io_context & io, std::string address, ConnectHandler onConnect, RecvHandler onRecv, DisconnectHandler onDisconnect, std::string caFilename )
	{
		detail = std::make_shared<Detail>( io, address, onConnect, onRecv, onDisconnect, caFilename );
		detail->resolve( );
	}

	void TcpConnection::send( std::string msg )
	{
		check<std::exception>( detail != nullptr, "TcpClient::send() : client is uninitialized" );
		detail->send( std::move( msg ) );
	}

	void TcpConnection::disconnect( )
	{
		if ( detail )
		{
			detail.reset( );
		}
	}



	TcpConnection::Detail::Detail( asio::io_context & io, std::string addr, ConnectHandler onConnect, RecvHandler onNotify, DisconnectHandler onDisconnect, std::string caPEM )
		: address( addr ), connectHandler( onConnect ), recvHandler( onNotify ), disconnectHandler( onDisconnect ), resolver( io ), tlsContext( asio::ssl::context::tlsv12 ), certificateAuthority( caPEM )
	{
		if ( !certificateAuthority.empty( ) )
		{
			tlsContext.load_verify_file( certificateAuthority );
			stream = std::make_unique<TlsStream>( io, tlsContext );
		}
		else
		{
			socket = std::make_unique<TcpSocket>( io );
		}
		printf( "TcpConnection::Detail::Detail( )\n" );
	}


	TcpConnection::Detail::~Detail( )
	{
		disconnect( );
		printf( "TcpConnection::Detail::~Detail( )\n" );
	}


	bool TcpConnection::Detail::isConnecting( ) const
	{
		return ( socket || stream ) && isConnectPending;
	}


	bool TcpConnection::Detail::isConnected( ) const
	{
		return ( socket || stream ) && !isConnectPending;
	}


	void TcpConnection::Detail::resolve( )
	{
		using namespace std::placeholders;

		size_t pos = address.find_last_of( ':' );
		check<std::invalid_argument>( pos != std::string::npos,
			"TcpConnection - address should be in the format [ip]:[port]" );

		std::string hostname = address.substr( 0, pos );
		std::string port = address.substr( pos + 1 );

		resolver.async_resolve( hostname, port, std::bind( &TcpConnection::Detail::onResolve, this, shared_from_this( ), _1, _2 ) );
	}


	void TcpConnection::Detail::connect( const TcpResolver::results_type & endpoints )
	{
		using namespace std::placeholders;

		auto & s = ( stream != nullptr )
			? stream->lowest_layer( )
			: *socket;
		async_connect( s, endpoints, std::bind( &TcpConnection::Detail::onConnect, this, shared_from_this( ), _1 ) );
	}


	void TcpConnection::Detail::handshake( )
	{
		using namespace std::placeholders;

		stream->async_handshake( asio::ssl::stream_base::client, std::bind( &TcpConnection::Detail::onHandshake, this, shared_from_this( ), _1 ) );
	}


	void TcpConnection::Detail::recv( )
	{
		using namespace std::placeholders;

		if ( !stream && !socket )
		{
			return;
		}

		recvBytes = recvBuffer.length( );
		if ( recvBuffer.length( ) - recvBytes < RecvSize )
		{
			recvBuffer.resize( recvBuffer.length( ) + RecvSize );
		}

		asio::mutable_buffer buf{ (char *)recvBuffer.data( ) + recvBytes, recvBuffer.length( ) - recvBytes };
		if ( stream )
		{
			stream->async_read_some( buf, std::bind( &TcpConnection::Detail::onRecv, this, shared_from_this( ), _1, _2 ) );
		}
		else
		{
			socket->async_read_some( buf, std::bind( &TcpConnection::Detail::onRecv, this, shared_from_this( ), _1, _2 ) );
		}
	}


	void TcpConnection::Detail::send( std::string data )
	{
		sendBuffers.push_back( std::move( data ) );
		send( );
	}


	void TcpConnection::Detail::send( )
	{
		using namespace std::placeholders;

		if ( isSending || sendBuffers.empty( ) )
		{
			return;
		}

		isSending = true;
		asio::const_buffer buf{ (const char *)sendBuffers[0].data( ), sendBuffers[0].length( ) };
		if ( stream )
		{
			stream->async_write_some( buf, std::bind( &TcpConnection::Detail::onSend, this, shared_from_this( ), _1, _2 ) );
		}
		else
		{
			socket->async_write_some( buf, std::bind( &TcpConnection::Detail::onSend, this, shared_from_this( ), _1, _2 ) );
		}
	}

	void TcpConnection::Detail::disconnect( )
	{
		stream.reset( );
		socket.reset( );

		if ( disconnectHandler )
		{
			disconnectHandler( errorCode );
		}

		disconnectHandler = nullptr;
		recvHandler = nullptr;
		connectHandler = nullptr;
	}


	void TcpConnection::Detail::disconnect( std::error_code error )
	{
		errorCode = error;
		disconnect( );
	}


	void TcpConnection::Detail::onResolve( std::shared_ptr<Detail> self, std::error_code error, const TcpResolver::results_type & endpoints )
	{
		if ( error )
		{
			notifyConnect( error );
		}
		else
		{
			connect( endpoints );
		}
	}


	void TcpConnection::Detail::onConnect( std::shared_ptr<Detail> self, std::error_code error )
	{
		notifyConnect( error );
		if ( !error )
		{
			if ( stream )
			{
				handshake( );
			}
			else
			{
				recv( );
			}
		}
	}


	void TcpConnection::Detail::onHandshake( std::shared_ptr<Detail> self, std::error_code error )
	{
		if ( error )
		{
			disconnect( error );
		}
		else
		{
			recv( );
		}
	}


	void TcpConnection::Detail::onRecv( std::shared_ptr<Detail> self, std::error_code error, std::size_t bytes )
	{
		if ( error )
		{
			disconnect( error );
		}
		else
		{
			recvBuffer.resize( recvBytes + bytes );

			if ( recvHandler )
			{
				recvHandler( recvBuffer );
			}

			recv( );
		}
	}


	void TcpConnection::Detail::onSend( std::shared_ptr<Detail> self, std::error_code error, std::size_t bytes )
	{
		isSending = false;
		if ( !errorCode )
		{
			if ( bytes == sendBuffers[0].length( ) )
			{
				sendBuffers.pop_front( );
			}
			else
			{
				sendBuffers[0].erase( 0, bytes );
			}
			send( );
		}
	}


	void TcpConnection::Detail::notifyConnect( const std::error_code & error )
	{
		try
		{
			isConnectPending = false;
			connectHandler( error );
		}
		catch ( ... )
		{
			disconnect( ); fprintf( stderr, "TcpConnection::notifyConnect() : exception thrown from handler\n" );
		}
	}


	void TcpConnection::Detail::notifyRecv( )
	{
		try
		{
			recvHandler( recvBuffer );
		}
		catch ( ... )
		{
			disconnect( ); fprintf( stderr, "TcpConnection::notifyRecv() : exception thrown from handler\n" );
		}
	}


	void TcpConnection::Detail::notifyDisconnect( const std::error_code & reason )
	{
		try
		{
			disconnectHandler( reason );
		}
		catch ( ... )
		{
			disconnect( ); fprintf( stderr, "TcpConnection::notifyDisconnect() : exception thrown from handler\n" );
		}
	}

}

#endif

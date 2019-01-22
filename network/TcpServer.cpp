#if __has_include(<openssl/ssl.h>)

#include <cassert>
#include <string>
#include <deque>
#include <map>

#include "TcpServer.h"



namespace cpp
{
	class TcpServer::Listener
		: public std::enable_shared_from_this<TcpServer::Listener>
	{
		struct passkey {};
		typedef asio::ip::tcp::resolver TcpResolver;
	public:
		typedef std::function<void( std::error_code error, TlsStreamPtr stream, TcpSocketPtr socket )> AcceptHandler;

		static std::shared_ptr<Listener> listen(
			asio::io_context & io,
			const std::string & address,
			uint16_t port,
			TcpVersion tcpVersion,
			TlsContextPtr tlsContext,
			AcceptHandler onAccept );

		Listener(
			passkey,
			asio::io_context & io,
			std::string address,
			uint16_t port,
			TcpVersion tcpVersion,
			TlsContextPtr tls,
			AcceptHandler onAccept );
		~Listener( );
		void close( );

	private:
		void accept( );

	private:
		AcceptHandler acceptHandler;
		TcpAcceptor acceptor;
		TlsContextPtr tlsContext;
	};



	class TcpServer::Session
		: public std::enable_shared_from_this<TcpServer::Session>
	{
		struct passkey {};
	public:
		typedef std::function<void( const std::string & addr, std::string & recvBuffer )> RecvHandler;
		typedef std::function<void( const std::string & addr, std::error_code reason )> CloseHandler;

		static std::shared_ptr<TcpServer::Session> connect(
			TlsStreamPtr stream,
			TcpSocketPtr socket,
			RecvHandler recvHandler,
			CloseHandler closeHandler );
		Session( passkey, TlsStreamPtr stream, TcpSocketPtr socket, RecvHandler recvHandler, CloseHandler closeHandler );
		~Session( );

		const std::string & addr( ) const;

		void disconnect( );
		void disconnect( std::error_code errorCode );
		void send( std::string msg );

	private:
		void handshake( );
		void send( );
		void recv( );

	private:
		TlsStreamPtr stream;
		TcpSocketPtr socket;
		RecvHandler recvHandler;
		CloseHandler closeHandler;

		std::string remoteAddress;
		std::string recvBuffer;
		std::deque<std::string> sendBuffers;
		bool isSending = false;
		std::error_code errorCode;
	};



	TcpServer::TcpServer( )
	{
	}


	TcpServer::TcpServer(
		asio::io_context & io,
		uint16_t port,
		ConnectHandler onConnect,
		RecvHandler onMessage,
		DisconnectHandler onDisconnect,
		const std::string & address,
		TcpVersion tcpVersion,
		std::string keyPEM,
		std::string certificatePEM,
		std::string dhParamsPEM )
	{
		open( io, port, onConnect, onMessage, onDisconnect, address, tcpVersion, keyPEM, certificatePEM, dhParamsPEM );
	}


	TcpServer::~TcpServer( )
	{
		close( );
	}


	void TcpServer::open(
		asio::io_context & io,
		uint16_t port,
		ConnectHandler onConnect,
		RecvHandler onMessage,
		DisconnectHandler onDisconnect,
		const std::string & address,
		TcpVersion tcpVersion,
		std::string keyPEM,
		std::string certificatePEM,
		std::string dhParamsPEM )
	{
		close( );

		connectHandler = onConnect;
		recvHandler = onMessage;
		disconnectHandler = onDisconnect;

		TlsContextPtr tlsContext;
		if ( !certificatePEM.empty( ) )
		{
			tlsContext = std::make_unique<TlsContext>( asio::ssl::context::tlsv12_server );
			tlsContext->set_options(
				asio::ssl::context::default_workarounds |
				asio::ssl::context::no_sslv2 |
				asio::ssl::context::single_dh_use );
			tlsContext->use_certificate_chain( asio::const_buffer{ certificatePEM.c_str( ), certificatePEM.size( ) } );
			tlsContext->use_private_key( asio::const_buffer{ keyPEM.c_str( ), keyPEM.size( ) }, asio::ssl::context::pem );
			tlsContext->use_tmp_dh( asio::const_buffer{ dhParamsPEM.c_str( ), dhParamsPEM.size( ) } );
		}

		using namespace std::placeholders;
		listener = Listener::listen(
			io,
			address,
			port,
			tcpVersion,
			std::move( tlsContext ),
			std::bind( &TcpServer::onAccept, this, _1, _2, _3 ) );
	}


	bool TcpServer::isOpen( ) const
	{
		return listener != nullptr;
	}


	void TcpServer::close( )
	{
		if ( listener )
		{
			listener->close( );
			listener.reset( );
		}

		while ( !connections.empty( ) )
		{
			auto connection = connections.begin( )->second;
			connection->disconnect( );
		}

		connectHandler = nullptr;
		recvHandler = nullptr;
		disconnectHandler = nullptr;
	}


	void TcpServer::send( const std::string & clientAddr, std::string data )
	{
		auto itr = connections.find( clientAddr );
		if ( itr != connections.end( ) )
		{
			itr->second->send( data );
		}
	}


	void TcpServer::disconnect( const std::string & clientAddr, std::error_code reason )
	{
		auto itr = connections.find( clientAddr );
		if ( itr != connections.end( ) )
		{
			itr->second->disconnect( reason );
		}
	}


	void TcpServer::onAccept( std::error_code error, TlsStreamPtr stream, TcpSocketPtr socket )
	{
		using namespace std::placeholders;

		std::string addr;
		if ( !error )
		{
			auto connection = Session::connect(
				std::move( stream ),
				std::move( socket ),
				std::bind( &TcpServer::onConnectionRecv, this, _1, _2 ),
				std::bind( &TcpServer::onConnectionClose, this, _1, _2 ) );

			addr = connection->addr( );
			connections.insert_or_assign( addr, std::move( connection ) );
		}
		// call the user's disconnect handler
		if ( connectHandler )
		{
			connectHandler( error, addr );
		}
	}


	void TcpServer::onConnectionRecv( const std::string & clientAddr, std::string & recvBuffer )
	{
		if ( recvHandler )
		{
			recvHandler( clientAddr, recvBuffer );
		}
	}


	void TcpServer::onConnectionClose( const std::string & clientAddr, std::error_code reason )
	{
		if ( disconnectHandler )
		{
			disconnectHandler( clientAddr, reason );
		}
		connections.erase( clientAddr );
	}



	std::shared_ptr<TcpServer::Listener> TcpServer::Listener::listen( asio::io_context & io, const std::string & address, uint16_t port, TcpVersion tcpVersion, TlsContextPtr tlsContext, AcceptHandler onAccept )
	{
		std::shared_ptr<Listener> listener = std::make_shared<Listener>( passkey{}, io, address, port, tcpVersion, std::move( tlsContext ), std::move( onAccept ) );
		listener->accept( );
		return listener;
	}


	TcpServer::Listener::Listener( passkey, asio::io_context & io, std::string address, uint16_t port, TcpVersion tcpVersion, TlsContextPtr tls, AcceptHandler onAccept )
		: acceptHandler( std::move( onAccept ) ), acceptor( io ), tlsContext( std::move( tls ) )
	{
		asio::ip::tcp tcpType = ( tcpVersion == TcpVersion::v6 )
			? asio::ip::tcp::v6( )
			: asio::ip::tcp::v4( );

		TcpResolver resolver{ acceptor.get_io_context( ) };
		auto endpoint = *resolver.resolve( tcpType, address, std::to_string( port ) ).begin( );

		acceptor.open( tcpType );
		acceptor.bind( endpoint );
		acceptor.listen( );
	}


	TcpServer::Listener::~Listener( )
	{
		close( );
	}


	void TcpServer::Listener::close( )
	{
		acceptor.close( );
		acceptHandler = nullptr;
	}

	void TcpServer::Listener::accept( )
	{
		auto self{ shared_from_this( ) };
		acceptor.async_accept( [this, self]( std::error_code error, TcpSocket socket )
			{
				TlsStreamPtr tlsStream;
				TcpSocketPtr tcpSocket;

				if ( tlsContext )
				{
					tlsStream = std::make_unique<TlsStream>( std::move( socket ), *tlsContext );
				}
				else
				{
					tcpSocket = std::make_unique<TcpSocket>( std::move( socket ) );
				}

				if ( acceptHandler )
				{
					acceptHandler( error, std::move( tlsStream ), std::move( tcpSocket ) );
				}

				if ( acceptor.is_open( ) )
				{
					accept( );
				}
			} );
	}



	std::shared_ptr<TcpServer::Session> TcpServer::Session::connect(
		TlsStreamPtr stream,
		TcpSocketPtr socket,
		RecvHandler recvHandler,
		CloseHandler closeHandler )
	{
		std::shared_ptr<Session> connection = std::make_shared<Session>(
			passkey{},
			std::move( stream ),
			std::move( socket ),
			std::move( recvHandler ),
			std::move( closeHandler ) );
		connection->handshake( );
		return connection;
	}


	TcpServer::Session::Session( passkey, TlsStreamPtr streamPtr, TcpSocketPtr socketPtr, RecvHandler onMessage, CloseHandler onClose )
		: stream( std::move( streamPtr ) ), socket( std::move( socketPtr ) ), recvHandler( std::move( onMessage ) ), closeHandler( std::move( onClose ) )
	{
		auto remoteEndpoint = ( stream != nullptr )
			? stream->lowest_layer( ).remote_endpoint( )
			: socket->remote_endpoint( );
		remoteAddress = remoteEndpoint.address( ).to_string( ) + ":" + std::to_string( remoteEndpoint.port( ) );

		printf( "Session( ) : %s\n", addr( ).c_str( ) );
	}


	TcpServer::Session::~Session( )
	{
		disconnect( );
		printf( "~Session( ) : %s\n", addr( ).c_str( ) );
	}


	void TcpServer::Session::disconnect( )
	{
		stream.reset( );
		socket.reset( );

		if ( closeHandler )
		{
			closeHandler( addr( ), errorCode );
		}

		closeHandler = nullptr;
		recvHandler = nullptr;
	}


	void TcpServer::Session::disconnect( std::error_code error )
	{
		errorCode = error;
		disconnect( );
	}


	void TcpServer::Session::send( std::string msg )
	{
		sendBuffers.push_back( msg );
		send( );
	}


	const std::string & TcpServer::Session::addr( ) const
	{
		return remoteAddress;
	}


	void TcpServer::Session::handshake( )
	{
		if ( stream )
		{
			auto self{ shared_from_this( ) };
			stream->async_handshake( asio::ssl::stream_base::server, [this, self]( std::error_code error )
				{
					if ( error )
					{
						disconnect( error );
					}
					else
					{
						recv( );
					}
				} );
		}
		else
		{
			recv( );
		}
	}


	void TcpServer::Session::send( )
	{
		if ( isSending || sendBuffers.empty( ) )
		{
			return;
		}

		auto self{ shared_from_this( ) };
		auto onWrite = [this, self]( std::error_code errorCode, std::size_t bytes )
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
		};

		isSending = true;
		asio::const_buffer buf{ (const char *)sendBuffers[0].data( ), sendBuffers[0].length( ) };
		if ( stream )
		{
			stream->async_write_some( buf, std::move( onWrite ) );
		}
		else
		{
			socket->async_write_some( buf, std::move( onWrite ) );
		}
	}


	void TcpServer::Session::recv( )
	{
		using namespace std::placeholders;

		if ( !stream && !socket )
		{
			return;
		}

		size_t recvBytes = recvBuffer.length( );
		if ( recvBuffer.length( ) - recvBytes < RecvSize )
		{
			recvBuffer.resize( recvBuffer.length( ) + RecvSize );
		}

		auto self{ shared_from_this( ) };
		auto onRead = [this, self, recvBytes]( std::error_code errorCode, std::size_t bytes )
		{
			if ( errorCode )
			{
				disconnect( errorCode );
			}
			else
			{
				recvBuffer.resize( recvBytes + bytes );

				if ( recvHandler )
				{
					recvHandler( remoteAddress, recvBuffer );
				}

				recv( );
			}
		};

		asio::mutable_buffer buf{ (char *)recvBuffer.data( ) + recvBytes, recvBuffer.length( ) - recvBytes };
		if ( stream )
		{
			stream->async_read_some( buf, std::move( onRead ) );
		}
		else
		{
			socket->async_read_some( buf, std::move( onRead ) );
		}
	}

}

#endif

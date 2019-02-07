#ifndef TEST

#include "../data/Integer.h"
#include "../data/DataBuffer.h"
#include "../time/Timer.h"
#include "Tcp.h"

namespace cpp
{

    Tcp::Connection Tcp::connect( String address, Duration timeoutDuration, AsyncIO io, size_t buflen, Protocol protocol )
    {
        return Connection{ address, timeoutDuration, std::move(io), buflen, protocol };
    }

    Tcp::Connection Tcp::asyncConnect( String address, Duration timeoutDuration, Tcp::Connection::ConnectHandler callback, AsyncIO io, size_t buflen, Protocol protocol )
    {
        return Connection{ address, timeoutDuration, std::move( callback ), std::move(io), buflen, protocol };
    }

    Tcp::Listener Tcp::listen( int port, Tcp::Listener::AcceptHandler callback, AsyncIO io, size_t buflen )
    {
        return Tcp::Listener{ port, std::move(callback), std::move(io), buflen };
    }


    struct Tcp::Connection::Detail
        : public Input::Source_t, public Output::Sink_t
    {
        Detail( AsyncIO io, size_t recvBufferSize ) : 
            m_io( std::move(io) ), 
            m_connectionTimer( ), 
            m_resolver( m_io.context( ) ),
            m_socket( m_io.context( ) ),
            m_recvBuffer( recvBufferSize ), 
            m_sendBytes( 0 ), 
            m_hasConnectTimeout( false ),
            m_isConnecting( false ),
            m_isConnected( false), 
            m_isRecving( false ), 
            m_isSending( false ) 
        { 
        }

        Detail( AsyncIO io, asio::ip::tcp::socket && socket, size_t recvBufferSize ) : 
            m_io( std::move(io) ), 
            m_connectionTimer( ), 
            m_resolver( m_io.context() ),
            m_socket( std::move( socket ) ), 
            m_recvBuffer( recvBufferSize ), 
            m_sendBytes( 0 ), 
            m_hasConnectTimeout( false ),
            m_isConnecting( false ),
            m_isConnected( true ),
            m_isRecving(false), 
            m_isSending(false) 
        { 
        }

        ~Detail( )
            { close(); }
        asio::mutable_buffers_1 toBuffer( const Memory & buf )
            { return asio::buffer( (char *)buf.data( ), buf.length( ) ); }

        void setBufferSize( size_t size )
        {
            m_recvBuffer.resize( size );
        }

        void doConnect( cpp::String address, cpp::Duration timeoutDuration, Tcp::Connection & connection, ConnectHandler connectHandler, Protocol protocol )
        {
            auto onConnect = [=]( const std::error_code & error )
            {
                assert( m_isConnecting );

                m_isConnecting = false;
                m_connectionTimer.cancel( );

                if ( !m_hasConnectTimeout && !error )
                {
                    m_isConnected = true;

                    auto & localEndpoint = m_socket.local_endpoint( );
                    auto & remoteEndpoint = m_socket.remote_endpoint( );

                    m_localIP = localEndpoint.address( ).to_string( );
                    m_localPort = localEndpoint.port( );
                    m_remoteIP = remoteEndpoint.address( ).to_string( );
                    m_remotePort = remoteEndpoint.port( );

                    doRecv( );
                }
                if ( m_connectHandler )
                    { m_connectHandler( connection, m_hasConnectTimeout ? std::make_error_code( std::errc::timed_out ) : error ); }
            };

            auto onResolve = [=]( const std::error_code & error, asio::ip::tcp::resolver::iterator iterator )
            {
                assert( m_isConnecting );

                if ( !m_hasConnectTimeout && !error )
                {
                    auto endpoint = iterator->endpoint( );
                    m_socket.async_connect( endpoint, onConnect );
                }
                else
                {
                    m_isConnecting = false;
                    m_connectionTimer.cancel( );
                    if ( m_connectHandler )
                        { m_connectHandler( connection, m_hasConnectTimeout ? std::make_error_code( std::errc::timed_out ) : error ); }
                }
            };

            auto onTimeout = [=]( )
            {
                m_hasConnectTimeout = true;
                m_resolver.cancel( );

                std::error_code err;
                m_socket.close( err );
            };

            m_connectHandler = connectHandler;
            m_addr = address;
            size_t pos = m_addr.find_last_of( ":" );
            cpp::check<cpp::Exception>( pos != String::npos, "Invalid address: " + m_addr );

            String host = m_addr.substr( 0, pos );
            String port = m_addr.substr( pos + 1 );

            m_isConnecting = true;
            asio::ip::tcp::resolver::query query = { ( protocol == Protocol::IPv6 ) ? asio::ip::tcp::v6( ) : asio::ip::tcp::v4( ), host, port };
            //m_connectionTimer = m_io.asyncWait( timeoutDuration, onTimeout );
			m_connectionTimer = m_io.waitFor( timeoutDuration, onTimeout );
            m_resolver.async_resolve( query, onResolve );
        }

        void doRecv( )
        {
            Memory dst = m_recvBuffer.putable( );
            if ( !dst || m_isRecving )
                { return; }
            m_isRecving = true;
            m_socket.async_read_some( toBuffer(dst), [=]( const std::error_code & error, std::size_t bytes )
                {
                    m_isRecving = false;
                    if ( error )
                    {
                        std::error_code err;
                        if ( m_socket.is_open() )
                            { m_socket.close( err ); }
                        if ( m_isConnected )
                        {
                            m_isConnected = false;
                            if ( m_closeHandler )
                                { m_closeHandler( error ); }
                        }
                    }
                    else
                    {
                        m_recvBuffer.put( bytes );
                        if ( m_recvHandler )
                            { m_recvHandler( m_recvBuffer.getAll( ) ); }
                        doRecv( );
                    }
                } );
        }

        void doSend( )
        {
            if ( !m_isConnected || m_sendBuffers.empty( ) )
                { return; }
            Memory src = Memory{ m_sendBuffers.front( ) }.substr( m_sendBytes );
            if ( !src )
                { return; }
            m_isSending = true;
            m_socket.async_write_some( toBuffer(src), [=]( const std::error_code & error, std::size_t bytes )
                {
                    m_isSending = false;
                    if ( error )
                    {
                        std::error_code err;
                        if ( m_socket.is_open() )
                            { m_socket.close( err ); }
                        if ( m_isConnected )
                        {
                            m_isConnected = false;
                            if ( m_closeHandler )
                                { m_closeHandler( error ); }
                        }
                    }
                    else
                    {
                        m_sendBytes += bytes;
                        if ( m_sendBytes == m_sendBuffers.front( ).size( ) )
                            { m_sendBuffers.pop_front( ); m_sendBytes = 0; }
                        if ( !m_sendBuffers.empty( ) )
                            { doSend( ); }
                    }
                } );
        }

        void doDisconnect( )
        {
            if ( m_isConnecting || m_isConnected )
            {
                std::error_code err;
                m_connectionTimer.cancel( );
                m_resolver.cancel( );
                m_socket.close( err );
            }
        }

        bool isConnecting() const
            { return m_isConnecting; }
        bool isConnected() const
            { return m_isConnected; }
        bool isOpen( ) const override
            { return m_isConnecting || m_isConnected; }
        bool isReceiving( ) const
            { return m_isRecving; }
        bool isSending( ) const
            { return m_isSending; }

        Memory read( Memory dst, Duration timeout ) override
        {
            Timer timer;
            while ( m_socket.is_open() || !m_recvBuffer.getable().isEmpty() )
            {
                //  wait if the read should block
                if ( m_recvBuffer.getable( ).isEmpty( ) )
                {
                    if ( !m_isRecving )
                        { doRecv(); }

                    if ( timer.elapsed( timeout ) )
                        { break; }

                    if ( timeout.isInfinite() )
                        { m_io.runOne( ); }
                    else
                        { m_io.runOne( timer.until( timeout ) ); }

                    continue;
                }

                size_t len = std::min( dst.length( ), m_recvBuffer.getable( ).length( ) );
                return Memory::copy( dst, m_recvBuffer.get( len ) );
            }
            return nullptr;
        }

        Memory write( const Memory & src ) override
        {
            m_sendBuffers.push_back( src );
            if ( !m_isSending )
                { doSend(); }
            return src;
        }

        void flush( ) override
        { 
            while ( m_socket.is_open() && !m_sendBuffers.empty( ) ) 
                { m_io.runOne( ); } 
        }

        void close( ) override
        { 
            doDisconnect( );
            m_connectHandler = nullptr; 
            m_recvHandler = nullptr;
            m_closeHandler = nullptr;
            while ( m_isConnecting || m_isConnected || m_isRecving || m_isSending )
                { m_io.runOne( ); }
        }

        AsyncIO m_io;
        AsyncTimer m_connectionTimer;
        asio::ip::tcp::resolver m_resolver;
        asio::ip::tcp::socket m_socket;
        StringBuffer m_recvBuffer;
        std::list<String> m_sendBuffers;
        size_t m_sendBytes;
        bool m_hasConnectTimeout;
        bool m_isConnecting;
        bool m_isConnected;
        bool m_isRecving;
        bool m_isSending;
        String m_addr;
        String m_localIP;
        int m_localPort;
        String m_remoteIP;
        int m_remotePort;

        ConnectHandler m_connectHandler;
        RecvHandler m_recvHandler;
        CloseHandler m_closeHandler;
    };

    Tcp::Connection::Connection( )
        : Input( nullptr ), Output( nullptr ), m_detail( nullptr ) { }

    Tcp::Connection::Connection( String address, Duration timeoutDuration, AsyncIO io, size_t buflen, Protocol protocol )
        : Input( nullptr ), Output( nullptr ), m_detail( std::make_shared<Detail>( std::move(io), buflen ) )
    {
        Input::m_source = m_detail;
        Output::m_sink = m_detail;
        connect( address, timeoutDuration, protocol );
    }

    Tcp::Connection::Connection( String address, Duration timeoutDuration, Tcp::Connection::ConnectHandler handler, AsyncIO io, size_t buflen, Protocol protocol )
        : Input( nullptr ), Output( nullptr ), m_detail( std::make_shared<Detail>( std::move( io ), buflen ) )
    {
        Input::m_source = m_detail;
        Output::m_sink = m_detail;
        asyncConnect( std::move(address), timeoutDuration, std::move(handler), protocol );
    }

    Tcp::Connection::Connection( AsyncIO io, asio::ip::tcp::socket && socket, size_t buflen )
        : Input( nullptr ), Output( nullptr ), m_detail( std::make_shared<Detail>( std::move(io), std::move(socket), buflen ) )
    {
        auto & localEndpoint = m_detail->m_socket.local_endpoint( );
        auto & remoteEndpoint = m_detail->m_socket.remote_endpoint( );
        m_detail->m_localIP = localEndpoint.address().to_string();
        m_detail->m_localPort = localEndpoint.port();
        m_detail->m_remoteIP = remoteEndpoint.address().to_string();
        m_detail->m_remotePort = remoteEndpoint.port();

        Input::m_source = m_detail;
        Output::m_sink = m_detail;
        m_detail->doRecv( );
    }


    void Tcp::Connection::connect( String address, Duration timeoutDuration, Protocol protocol )
    {
        std::error_code result;
        asyncConnect( address, timeoutDuration, [&]( Connection connection, const std::error_code & error )
            {
                result = error;
            }, protocol );
        while ( isConnecting( ) )
            { m_detail->m_io.runOne(); }
        check( result );
    }

    void Tcp::Connection::asyncConnect( String address, Duration timeoutDuration, ConnectHandler connectHandler, Protocol protocol )
    {
        m_detail->doConnect( address, timeoutDuration, *this, connectHandler, protocol );
    }

    bool Tcp::Connection::isOpen( ) const
    {
        return m_detail && m_detail->isOpen( );
    }

    Tcp::Connection::operator bool( ) const
    {
        return isOpen( );
    }

    bool Tcp::Connection::isConnecting( ) const
    {
        return m_detail && m_detail->isConnecting( );
    }

    bool Tcp::Connection::isConnected( ) const
    {
        return m_detail && m_detail->isConnected( );
    }

    bool Tcp::Connection::isReceiving( ) const
    {
        return m_detail && m_detail->isReceiving( );
    }

    bool Tcp::Connection::isSending( ) const
    {
        return m_detail && m_detail->isSending( );
    }

    void Tcp::Connection::disconnect( )
    {
        if ( isOpen( ) )
        {
            m_detail->doDisconnect( );
        }
    }

    void Tcp::Connection::close( )
    {
        if ( isOpen() )
        { 
            m_detail->close( ); 
        }
    }

    cpp::String Tcp::Connection::localAddress( ) const
    {
        return m_detail ? cpp::String::format( "%:%", localIP( ), cpp::Integer::toDecimal( localPort() ) ) : "null";
    }

    cpp::String Tcp::Connection::localIP( ) const
    {
        return m_detail ? m_detail->m_localIP : "null";
    }

    int Tcp::Connection::localPort( ) const
    {
        return m_detail ? m_detail->m_localPort : 0;
    }

    cpp::String Tcp::Connection::remoteAddress( ) const
    {
        return m_detail ? cpp::String::format( "%:%", remoteIP( ), cpp::Integer::toDecimal( remotePort() ) ) : "null";
    }

    cpp::String Tcp::Connection::remoteIP( ) const
    {
        return m_detail ? m_detail->m_remoteIP : "null";
    }

    int Tcp::Connection::remotePort( ) const
    {
        return m_detail ? m_detail->m_remotePort : 0;
    }

    void Tcp::Connection::setBufferSize( size_t size )
    {
        if ( isOpen( ) )
            { m_detail->setBufferSize( size ); }
    }

    void Tcp::Connection::async( RecvHandler recvHandler, CloseHandler closeHandler )
    {
        cpp::check<cpp::Exception>( isConnected( ), "Tcp::Connection::async() : not connected" );
        assert( m_detail->m_recvHandler == nullptr );
        assert( m_detail->m_closeHandler == nullptr );

        m_detail->m_recvHandler = recvHandler;
        m_detail->m_closeHandler = closeHandler;

        //  trigger recv handler if any data has already been recvd
        if ( m_detail->m_recvBuffer.getable() && m_detail->m_recvHandler )
            { m_detail->m_recvHandler( m_detail->m_recvBuffer.getAll( ) ); }
        m_detail->doRecv( );
    }
    



    struct Tcp::Listener::Detail
    {
		Detail( AsyncIO & io, size_t buflen )
			: m_io( io ), m_buflen( buflen ), m_isAccepting( false ), m_acceptor( io.context( ) ), m_socket( io.context( ) ) { }
        ~Detail()
            { close(); }

        void close( );

        void accept( );
        void onAccept( std::error_code error );

        AsyncIO m_io;
        size_t m_buflen;
        bool m_isAccepting;
        asio::ip::tcp::acceptor m_acceptor;
        asio::ip::tcp::socket m_socket;
        std::function< void( std::error_code, Tcp::Connection ) > m_acceptHandler;
    };

    void Tcp::Listener::Detail::close( )
    {
        m_acceptHandler = nullptr;
        m_acceptor.close( );
        while ( m_isAccepting )
            { m_io.runOne( ); }
    }

    void Tcp::Listener::Detail::accept( )
    {
        if ( m_acceptor.is_open() )
        {
            m_isAccepting = true;
            m_acceptor.async_accept( m_socket, [=]( std::error_code error ) { onAccept( error ); } );
        }
    }

    void Tcp::Listener::Detail::onAccept( std::error_code error )
    {
        m_isAccepting = false;
        
		Connection connection = error ? Connection{} : Connection{ m_io, std::move( m_socket ), m_buflen };
        m_socket = std::move( asio::ip::tcp::socket{ m_io.context() } );
        if ( error )
            { m_acceptor.close(); }
        else
            { accept( ); }
        
        if ( m_acceptHandler )
            { m_acceptHandler( error, connection ); }
    }


    Tcp::Listener::Listener()
        : m_detail( nullptr ) { }

    Tcp::Listener::Listener( AsyncIO io, size_t buflen )
        : m_detail( std::make_shared<Detail>( std::move(io), buflen ) )
    {
    }

    Tcp::Listener::Listener( String bindAddress, int port, AcceptHandler acceptHandler, AsyncIO io, size_t buflen )
        : m_detail( std::make_shared<Detail>( std::move(io), buflen ) )
    {
        listen( bindAddress, port, std::move(acceptHandler) );
    }

    Tcp::Listener::Listener( int port, AcceptHandler callback, AsyncIO io, size_t buflen )
        : m_detail( std::make_shared<Detail>( std::move(io), buflen ) )
    {
        listen( port, callback );
    }

    void Tcp::Listener::listen( int port, AcceptHandler callback )
    {
        listen( "::", port, std::move(callback) );
    }

    void Tcp::Listener::listen( String bindAddress, int port, AcceptHandler acceptHandler )
    {
        auto & acceptor = m_detail->m_acceptor;

        std::error_code error;
        asio::ip::tcp::resolver resolver( m_detail->m_io.context() );
        asio::ip::tcp::resolver::query query{ asio::ip::tcp::v6(), bindAddress, Integer::toDecimal(port) };
        auto itr = resolver.resolve( query, error );
        check( error );

        auto endpoint = itr->endpoint();

        check( acceptor.open( endpoint.protocol( ), error ) );
        check( acceptor.set_option( asio::ip::tcp::acceptor::reuse_address( true ), error ) );
        check( acceptor.bind( endpoint, error ) );
        check( acceptor.listen( asio::socket_base::max_connections, error ) );

        m_detail->m_acceptHandler = std::move( acceptHandler );

        m_detail->accept( );
    }

    bool Tcp::Listener::isOpen( ) const
    {
        return m_detail && m_detail->m_acceptor.is_open( );
    }

    Tcp::Listener::operator bool( ) const
    {
        return isOpen( );
    }

    void Tcp::Listener::close( )
    {
        if ( isOpen( ) )
        {
            //  increment ref counter for duration of Detail::close() in case a callback 
            //  destroys the object by assigning this connection
            std::shared_ptr<Detail> detail = m_detail;
            detail->close( ); 
            //m_detail.reset( );
        }
    }

}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/io/network/Tcp.h>
#include <cpp/process/Thread.h>
#include <cpp/Program.h>
#include <cpp/io/reader/LineReader.h>
#include <cpp/io/StandardOutput.h>

SUITE( TcpConnection )
{
    using namespace cpp;

    TEST( asyncConnect )
    {
        Tcp::Connection connection = Tcp::asyncConnect( "google.com:80", Duration::ofSeconds( 5 ), nullptr, Program::asyncIO( ) );
        while ( connection.isConnecting() )
            { Program::update( ); }
        CHECK( connection.isOpen() );

        connection = Tcp::asyncConnect( "10.10.10.1:80", Duration::ofMillis( 100 ), nullptr, Program::asyncIO( ) );
        CHECK( !connection.isConnected( ) );
        while ( connection.isConnecting() )
            { Program::update( ); }
        CHECK( !connection.isOpen( ) );

        std::error_code result;
        connection = Tcp::asyncConnect(
            "google.com:80",
            Duration::ofMillis( 1 ),
            [&]( Tcp::Connection connection, const std::error_code & error )
                {
                    result = error;
                }, 
            Program::asyncIO( ) );
        CHECK( !connection.isConnected( ) );

        while ( connection.isConnecting() )
            { Program::update( ); }

        CHECK( connection.isOpen( ) == false );
        CHECK( result.value( ) == (int)std::errc::timed_out );
    }

    TEST( connect )
    {
        try
        {
            Tcp::Connection connection = Tcp::connect( "google.com:80", Duration::ofSeconds( 5 ) );
            CHECK( connection.isOpen( ) );
            connection.close( );
            CHECK( !connection.isOpen( ) );
        }
        catch ( Exception & )
        {
            CHECK( false );
        }

        try
        {
            Tcp::Connection connection = Tcp::connect( "10.10.10.1:80", Duration::ofMillis( 100 ) );
            CHECK( !connection.isOpen( ) );
        }
        catch ( Exception & )
        {
            CHECK( true );
        }
    }

    TEST( close )
    {
        bool handlerCalled = false;

        Tcp::Connection connection = Tcp::asyncConnect( 
            "some.fake.com:80", 
            Duration::ofMillis( 100 ), 
            [&]( Tcp::Connection connection, const std::error_code & error )
                { 
                    handlerCalled = true; 
                }, 
            Program::asyncIO( ) );

        CHECK( !connection.isConnected( ) );
        CHECK( connection.isConnecting( ) );
        connection.close( );

        CHECK( handlerCalled == false );

        Program::asyncIO( ).runFor( Duration::ofMillis( 100 ) );

        CHECK( handlerCalled == false );
    }

    TEST( read_write )
    {
        bool result = false;
        Tcp::Connection connection;
        auto acceptFn = [&connection, &result]( std::error_code error, Tcp::Connection c )
        {
            connection = c;
            connection.async( 
                [&]( const Memory & data ) 
                    { 
                        if ( data == "hello!\n" )
                            { connection.put( data ); }
                        else if ( data == "goodbye!\n" )
                            { connection.put( data ); }
                        else if ( data == "laterz!\n" )
                            { result = true; }
                    },
                []( const std::error_code & error ) 
                    { 
                    } );
        };

        AsyncIO io;
        Tcp::Listener listener = Tcp::listen( 5000, acceptFn, io );
        Tcp::Connection client = Tcp::connect( "::1:5000", Duration::ofMillis( 500 ), io );

        CHECK( listener.isOpen( ) );
        CHECK( client.isConnected( ) );

        String buffer{ 32, '\0' };
        client.put( "hello!\n" );
        CHECK( client.read( buffer ) == "hello!\n" );
        client.put( "goodbye!\n" );
        CHECK( client.read( buffer ) == "goodbye!\n" );
        client.put( "laterz!\n" );
        client.close( );
        CHECK( result == true );
    }


    TEST( write_before_read )
    {
        Tcp::Connection connection;
        auto acceptFn = [&connection]( std::error_code error, Tcp::Connection c )
        {
            if ( !error )
            {
                connection = c;
                connection.put( "data" );
            }
        };

        AsyncIO io;
        Tcp::Listener listener = Tcp::listen( 5000, acceptFn, io );
        Tcp::Connection client = Tcp::connect( "::1:5000", Duration::ofMillis( 500 ), io );

        io.poll( );

        cpp::String rdata;
        client.async(
            [&]( const cpp::Memory & data )
            { 
                rdata += data; 
            },
            []( const std::error_code & error )
            { 
            } );

        if ( rdata != "data" )
            { io.runOne( ); }

        CHECK( rdata == "data" );
    }

}

#endif

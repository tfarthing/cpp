#pragma once

#include "../data/String.h"
#include "../process/Platform.h"
#include "../process/Program.h"
#include "../time/Duration.h"
#include "../io/Input.h"
#include "../io/Output.h"

#include "../process/AsyncIO.h"



namespace cpp
{

    class Tcp
    {
    public:
        struct Exception
            : cpp::Exception
        {
            Exception( std::error_code error )
                : cpp::Exception( error.message() ), m_error(error) { }

            std::error_code m_error;
        };
        static void check( std::error_code error )
            { if ( error ) { throw Tcp::Exception( error ); } }

        static const size_t DefaultRecvBufferSize = 64 * 1024;

        enum Protocol { IPv4, IPv6 };

        class Connection
            : public Input, public Output
        {
        public:
            Connection( );

            using ConnectHandler = std::function< void( Connection connection, const std::error_code & error) >;
            using CloseHandler = std::function< void( const std::error_code & error ) >;
            using RecvHandler = std::function< void( const Memory & data ) >;

            void async( RecvHandler recvHandler, CloseHandler closeHandler );

            bool isOpen( ) const;
            operator bool( ) const;

            bool isConnecting( ) const;
            bool isConnected( ) const;
            bool isReceiving( ) const;
            bool isSending( ) const;

            void disconnect( );
            void close( );

            cpp::String localAddress( ) const;
            cpp::String localIP( ) const;
            int localPort( ) const;

            cpp::String remoteAddress( ) const;
            cpp::String remoteIP( ) const;
            int remotePort( ) const;

            void setBufferSize( size_t size );

        private:
            friend class Tcp;
            Connection( String address, Duration timeoutDuration, AsyncIO io, size_t buflen, Protocol protocol );
            Connection( String address, Duration timeoutDuration, Tcp::Connection::ConnectHandler handler, AsyncIO io, size_t buflen, Protocol protocol );
            Connection( AsyncIO io, asio::ip::tcp::socket && socket, size_t buflen );

            void connect( String address, Duration timeoutDuration = Duration::ofSeconds( 5 ), Protocol protocol = Protocol::IPv6);
            void asyncConnect( String address, Duration timeoutDuration, ConnectHandler callback, Protocol protocol = Protocol::IPv6 );

        private:
            struct Detail;
            std::shared_ptr<Detail> m_detail;
        };


        class Listener
        {
        public:
            using AcceptHandler = std::function< void( std::error_code, Connection ) >;

            Listener( );

            bool isOpen( ) const;
            operator bool( ) const;
            void close( );

        private:
            friend class Tcp;
            Listener( AsyncIO io, size_t buflen );
            Listener( String bindAddress, int port, AcceptHandler callback, AsyncIO io, size_t buflen );
            Listener( int port, AcceptHandler callback, AsyncIO io, size_t buflen );

            void listen( int port, AcceptHandler callback );
            void listen( String bindAddress, int port, AcceptHandler callback );

        private:
            struct Detail;
            std::shared_ptr<Detail> m_detail;
        };


        static Connection connect( 
            String address, 
            Duration timeoutDuration = Duration::ofSeconds( 5 ), 
            AsyncIO io = AsyncIO{ }, 
            size_t buflen = DefaultRecvBufferSize,
            Protocol protocol = Protocol::IPv6 );
        
        static Connection asyncConnect( 
            String address, 
            Duration timeoutDuration = Duration::ofSeconds( 5 ), 
            Connection::ConnectHandler callback = nullptr,
            AsyncIO io = AsyncIO{ },
            size_t buflen = DefaultRecvBufferSize,
            Protocol protocol = Protocol::IPv6 );

        static Listener listen( 
            int port, 
            Listener::AcceptHandler callback, 
            AsyncIO io = AsyncIO{ },
            size_t buflen = DefaultRecvBufferSize );

        static Listener listen( 
            String bindAddress, 
            int port, Listener::AcceptHandler callback, 
            AsyncIO io = AsyncIO{ },
            size_t buflen = DefaultRecvBufferSize );

    };

}
#ifndef TEST

#include <list>
#include <set>
#include <map>
#include <cpp/Exception.h>
#include <cpp/Platform.h>
#include <cpp/chrono/timer.h>
#include <cpp/io/network/Http.h>
#include <cpp/platform/windows/WindowsException.h>
#include <cpp/util/DataBuffer.h>
#include <cpp/util/Log.h>
#include <Wininet.h>

namespace cpp
{

    const char * METHOD_GET = "GET";
    const char * METHOD_POST = "POST";

    struct URL
    {
        URL( )
            : m_scheme(0), m_port(0) { }

        URL( String url );

        String id( ) const
        {
            return String::format( "%:%", toUtf8( m_host ), m_port );
        }

        String source( ) const
        {
            return m_urlSource;
        }

        cpp::String m_urlSource;
        uint32_t m_scheme;
        uint32_t m_port;
        std::wstring m_host;
        std::wstring m_path;
        std::wstring m_extra;
    };

    URL::URL( String url ) :
        m_urlSource( url ),
        m_scheme( 0 ),
        m_port( 0 ),
        m_host( 1024, 0 ),
        m_path( 1024, 0 ),
        m_extra( 1024, 0 )
    {
        URL_COMPONENTS urlComponents = { 0 };
        urlComponents.dwStructSize = sizeof( URL_COMPONENTS );
        urlComponents.lpszHostName = (LPWSTR)m_host.data( );
        urlComponents.dwHostNameLength = (DWORD)m_host.length( );
        urlComponents.lpszUrlPath = (LPWSTR)m_path.data( );
        urlComponents.dwUrlPathLength = (DWORD)m_path.length( );
        urlComponents.lpszExtraInfo = (LPWSTR)m_extra.data( );
        urlComponents.dwExtraInfoLength = (DWORD)m_extra.length( );

        Utf16::Text wurl = toUtf16( url );
        windows::check( InternetCrackUrl( wurl, (DWORD)wurl.size( ), ICU_DECODE | ICU_ESCAPE, &urlComponents ) != FALSE );
        m_host.resize( urlComponents.dwHostNameLength );
        m_path.resize( urlComponents.dwUrlPathLength );
        m_extra.resize( urlComponents.dwExtraInfoLength );
        m_scheme = urlComponents.nScheme;
        m_port = urlComponents.nPort;
    }


    struct Http::Request::Detail
        : public cpp::Input::Source_t, public cpp::Output::Sink_t
    {
    public:
        Detail( HINTERNET connection, const URL & url, String method, String headers, Duration timeout ) :
            m_connectionHandle( connection ),
            m_url( url ),
            m_method( method ),
            m_headers( cpp::toUtf16( headers ) ),
            m_timeout( timeout ),
            m_handle( nullptr ),
            m_isPending( false ),
            m_isRecving( false ),
            m_recvBytes( 0 ),
            m_recvBuffer( 64 * 1024 ),
            m_isSending( false ),
            m_sendBytes( 0 ),
            m_sentBytes( 0 ),
            m_isRequesting( true )
        {
            cpp::Timer timer;
            startRequest( timer.until( timeout ) );
            if ( m_method == METHOD_GET )
                { endRequest( timer.until( timeout ) ); }
        }

        ~Detail( )
        {
            close( );
        }

        void doStart( )
        {

        }

        void notify( DWORD err )
        {
            auto lock = m_mutex.lock( );
            m_isPending = false;
            if ( err )
            {
                setError( err );
                doClose( );
            }
            else
            {
                if ( m_isRecving )
                {
                    notifyRecv( );
                }
                else if ( m_isSending )
                {
                    notifySend( );
                }
            }
            lock.unlock( );
            lock.notifyAll( );
        }
        void notifyRecv( )
        {
            m_isRecving = false;
            if ( m_recvBytes )
            {
                m_recvBuffer.put( m_recvBytes );
            }
            else
            {
                doClose( );
            }
            doRecv( );
        }
        void notifySend( )
        {
            m_isSending = false;
            m_sentBytes += m_sendBytes;
            if ( m_sendBuffers.front( ).length( ) == m_sentBytes )
            {
                m_sendBuffers.pop_front( );
                m_sentBytes = 0;
            }
            doSend( );
        }
        void doRecv( )
        {
            Memory dst = m_recvBuffer.putable( );
            if ( !dst || !m_handle )
            {
                return;
            }

            m_isRecving = true;
            if ( InternetReadFile( m_handle, (LPVOID)dst.c_str( ), (DWORD)dst.length( ), &m_recvBytes ) == FALSE )
            {
                int err = GetLastError( );
                if ( err != ERROR_IO_PENDING )
                {
                    m_isRecving = false; setError( err ); doClose( );
                }
            }
            else
            {
                notifyRecv( );
            }
        }
        void doSend( )
        {
            if ( m_sendBuffers.empty( ) || !m_handle )
            {
                return;
            }

            String & sendItem = m_sendBuffers.front( );
            Memory src = Memory{ sendItem }.substr( (size_t)m_sentBytes );
            if ( !src )
            {
                return;
            }
            m_isSending = true;
            if ( InternetWriteFile( m_handle, (LPVOID)src.c_str( ), (DWORD)src.length( ), &m_sendBytes ) == FALSE )
            {
                int err = GetLastError( );
                if ( err != ERROR_IO_PENDING )
                {
                    m_isSending = false; doClose( );
                }
            }
        }
        void doClose( )
        {
            InternetCloseHandle( m_handle );
            m_handle = nullptr;
        }

        bool isOpen( ) const override
        {
            auto lock = m_mutex.lock( );
            return m_handle != nullptr || m_recvBuffer.getable( ).isEmpty( ) == false;
        }
        Memory read( const Memory & dst, Duration timeout ) override
        {
            auto lock = m_mutex.lock( );

            Timer timer;
            while ( m_handle || m_recvBuffer.getable( ) )
            {
                //  wait if the read should block
                if ( m_recvBuffer.getable( ).isEmpty( ) )
                {
                    if ( !m_isRecving )
                        { doRecv( ); continue; }

                    if ( timer.test( timeout ) )
                        { break; }

                    lock.waitFor( timer.until( timeout ) );
                    continue;
                }

                size_t len = std::min( dst.length( ), m_recvBuffer.getable( ).length( ) );
                return Memory::copy( dst, m_recvBuffer.get( len ) );
            }

            check( );
            return Memory::Null;
        }
        Memory write( const Memory & src ) override
        {
            auto lock = m_mutex.lock( );

            m_sendBuffers.push_back( src );
            if ( !m_isSending )
            {
                doSend( );
            }

            check( );
            return src;
        }
        void flush( ) override
        {
            auto lock = m_mutex.lock( );
            while ( m_handle != nullptr && !m_sendBuffers.empty( ) )
            {
                lock.wait( );
            }
        }
        void close( ) override
        {
            auto lock = m_mutex.lock( false );
            if ( m_handle )
            {
                doClose();
                while ( m_isPending )
                    { lock.wait( ); }
            }
            m_recvBuffer.getAll( );
        }

        static int toStatusCode( DWORD err )
        {
            switch ( err )
            {
            default:
                return 400;
            }
        }
        
        void setError( DWORD err )
        {
            m_error = std::make_shared<Exception>( windows::Exception::getErrorMessage( err ), toStatusCode( err ), m_url.source( ), toUtf8( m_headers ) );
        }

        void check( bool isSuccess )
        {
            if ( !isSuccess )
            {
                setError( GetLastError( ) ); check( );
            }
        }
        void check( )
        {
            if ( m_error )
            {
                throw *m_error;
            }
        }

        void startRequest( cpp::Duration timeout )
        {
            cpp::Timer timer;

            std::wstring objectName = m_url.m_path + m_url.m_extra;
            DWORD flags = INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;
            if ( m_url.m_scheme == INTERNET_SCHEME_HTTPS )
            {
                flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
            }

            m_handle = HttpOpenRequest(
                m_connectionHandle,                                     // InternetConnect handle
                toUtf16( m_method ).data( ),                                        // Method
                objectName.c_str( ),                                        // Object name
                HTTP_VERSION,                                               // Version
                L"",                                                        // Referrer
                NULL,                                                       // Extra headers
                flags,                                                      // Flags
                ( DWORD_PTR )this );
            check( m_handle != nullptr );

            if ( m_url.m_scheme == INTERNET_SCHEME_HTTPS )
            {
                DWORD dwFlags = 0;
                DWORD dwBuffLen = sizeof( dwFlags );
                InternetQueryOption( m_handle, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen );
                dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
                InternetSetOption( m_handle, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof( dwFlags ) );
            }

            try
            {
                auto lock = m_mutex.lock( );

                INTERNET_BUFFERS buffers{ 0 };
                buffers.dwStructSize = sizeof( INTERNET_BUFFERS );
                buffers.lpcszHeader = m_headers.c_str( );
                buffers.dwHeadersLength = (DWORD)m_headers.length( );

                if ( HttpSendRequestEx( m_handle, &buffers, NULL, 0, ( DWORD_PTR )this ) == FALSE )
                {
                    int err = GetLastError( );
                    check( err == ERROR_IO_PENDING );

                    m_isPending = true;
                    while ( m_isPending && !timer.test( timeout ) )
                    {
                        lock.waitFor( timer.until( timeout ) );
                    }
                    cpp::check<TimeoutException>( m_isPending == false );
                }
            }
            catch ( ... )
            {
                close( );
                throw;
            }
        }

        void endRequest( cpp::Duration timeout )
        {
            try
            {
                auto lock = m_mutex.lock( );
                
                if ( m_isRequesting == false )
                    { return; }
                
                cpp::Timer timer;
                if ( HttpEndRequest( m_handle, 0, 0, ( DWORD_PTR )this ) == FALSE )
                {
                    int err = GetLastError( );
                    check( err == ERROR_IO_PENDING );
                    m_isPending = true;

                    while ( m_isPending && !timer.test( timeout ) )
                        { lock.waitFor( timer.until( timeout ) ); }
                    cpp::check<TimeoutException>( m_isPending == false );
                }

                m_isRequesting = false;

                DWORD buflen = 0;
                m_headers.clear( );
                HttpQueryInfo( m_handle, HTTP_QUERY_RAW_HEADERS_CRLF, (LPWSTR)m_headers.c_str( ), &buflen, NULL );
                if ( buflen > 0 )
                {
                    m_headers.resize( buflen );
                    HttpQueryInfo( m_handle, HTTP_QUERY_RAW_HEADERS_CRLF, (LPWSTR)m_headers.c_str( ), &buflen, NULL );
                }

                String hdr = toUtf8( m_headers );
                auto statusResult = hdr.searchOne( R"(HTTP(\S+)\s(\S+)\s(.*)\r\n)" );
                if ( statusResult )
                {
                    m_statusCode = (int)Integer::parse( statusResult[2] );
                    if ( m_statusCode < 200 || m_statusCode >= 300 )
                    {
                        throw Exception{ statusResult[3], m_statusCode, m_url.source( ), hdr };
                    }
                }

                doRecv( );
                check( );
            }
            catch ( ... )
            {
                close( );
                throw;
            }
        }

        int statusCode() const
        {
            return m_statusCode;
        }

    private:
        HINTERNET m_connectionHandle;
        URL m_url;
        String m_method;
        std::wstring m_headers;
        Duration m_timeout;

        HINTERNET m_handle;
        bool m_isPending;
        bool m_isRecving;
        DWORD m_recvBytes;
        DataBuffer m_recvBuffer;
        bool m_isSending;
        DWORD m_sendBytes;
        uint64_t m_sentBytes;
        std::list<String> m_sendBuffers;
        mutable cpp::Mutex m_mutex;
        std::shared_ptr<Exception> m_error;
        int m_statusCode;
        bool m_isRequesting;
    };


    Http::Request::Request( std::shared_ptr<Detail> && detail )
        : m_detail( std::move( detail ) )
    {

    }

    Http::Request & Http::Request::writeRequest( String input )
    {
        m_detail->write( input );
        return *this;
    }

    Http::Request & Http::Request::writeRequest( Input input )
    {
        cpp::String buffer( 1024, '\0' );
        while ( input )
        {
            m_detail->write( input.read( buffer ) );
        }
        return *this;
    }

    Input Http::Request::getReply( cpp::Duration timeout )
    {
        m_detail->endRequest( timeout );
        return m_detail;
    }

    void Http::Request::close( cpp::Duration timeout )
    {
        m_detail->endRequest( timeout );
    }

    int Http::Request::getStatusCode( ) const
    {
        return m_detail->statusCode();
    }


    class CallbackHandler
    {
    public:
        CallbackHandler( )
            : m_fn() { }
        CallbackHandler( std::function<void( DWORD status )> fn )
            : m_fn( std::move( fn ) ) { }

        void notify( DWORD status )
            { if ( m_fn ) { m_fn( status ); } }

    private:
        std::function<void( DWORD status )> m_fn;
    };

    struct Http::Detail
    {
        Detail( )
            : m_inet( nullptr ) { }
        ~Detail( )
            { close( ); }

        static void CALLBACK callback( HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength );

        void open( String userAgent );
        HINTERNET connect( const URL & url );
        Request startRequest( const URL & url, String method, String headers, Duration timeout );

        Request get( String url, String headers, Duration timeout );
        Request post( String url, String headers, Duration timeout );

        String::Array connections( ) const;
        void disconnect( String hostname );
        void disconnect( );
        void close( );

        HINTERNET m_inet;
        std::map<String, HINTERNET> m_connections;
        mutable cpp::Mutex m_mutex;
    };


    void CALLBACK Http::Detail::callback( HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
    {
        HINTERNET connection = nullptr;
        Request::Detail * request = nullptr;

        try
        {
            switch ( dwInternetStatus )
            {
                case INTERNET_STATUS_HANDLE_CREATED:
                    //debug( "INTERNET_STATUS_HANDLE_CREATED" );
                    break;
                case INTERNET_STATUS_HANDLE_CLOSING:
                    //debug( "INTERNET_STATUS_HANDLE_CLOSING" );
                    break;

                case INTERNET_STATUS_RESOLVING_NAME:
                    //debug( "INTERNET_STATUS_RESOLVING_NAME" );
                    break;
                case INTERNET_STATUS_NAME_RESOLVED:
                    //debug( "INTERNET_STATUS_NAME_RESOLVED" );
                    break;

                case INTERNET_STATUS_CONNECTING_TO_SERVER:
                    //debug( "INTERNET_STATUS_CONNECTING_TO_SERVER" );
                    break;
                case INTERNET_STATUS_CONNECTED_TO_SERVER:
                    //debug( "INTERNET_STATUS_CONNECTED_TO_SERVER" );
                    break;

                case INTERNET_STATUS_CLOSING_CONNECTION:
                    //debug( "INTERNET_STATUS_CLOSING_CONNECTION" );
                    break;
                case INTERNET_STATUS_CONNECTION_CLOSED:
                    //debug( "INTERNET_STATUS_CONNECTION_CLOSED" );
                    break;

                case INTERNET_STATUS_SENDING_REQUEST:
                    //debug( "INTERNET_STATUS_SENDING_REQUEST" );
                    break;
                case INTERNET_STATUS_REQUEST_SENT:
                    //debug( "INTERNET_STATUS_REQUEST_SENT" );
                    break;

                case INTERNET_STATUS_RECEIVING_RESPONSE:
                    //debug( "INTERNET_STATUS_RECEIVING_RESPONSE" );
                    break;
                case INTERNET_STATUS_RESPONSE_RECEIVED:
                    //debug( "INTERNET_STATUS_RESPONSE_RECEIVED" );
                    break;
                case INTERNET_STATUS_REQUEST_COMPLETE:
                    INTERNET_ASYNC_RESULT * asyncResult;
                    Memory::copy( Memory::ofValue( asyncResult ), Memory::ofValue( lpvStatusInformation ) );
                    Memory::copy( Memory::ofValue( request ), Memory::ofValue( dwContext ) );
                    request->notify( asyncResult->dwError );
                    break;
                case INTERNET_STATUS_REDIRECT:
                    //debug( "INTERNET_STATUS_REDIRECT" );
                    break;
                case INTERNET_STATUS_STATE_CHANGE:
                    //debug( "INTERNET_STATUS_STATE_CHANGE" );
                    break;
                default:
                    break;
            }
        }
        catch ( std::exception & e )
        {
            log( LogLevel::Error, e.what( ) );
        }
    }


    void Http::Detail::open( String userAgent )
    {
        if ( m_inet == nullptr )
        {
            m_inet = InternetOpen( 
                toUtf16( userAgent ).data(), 
                INTERNET_OPEN_TYPE_DIRECT, 
                NULL, 
                NULL, 
                INTERNET_FLAG_ASYNC );
            windows::check( m_inet != nullptr );

            InternetSetStatusCallback( m_inet, callback );
        }
    }

    HINTERNET Http::Detail::connect( const URL & url )
    {
        auto lock = m_mutex.lock( );

        String connectionId = url.id( );
        HINTERNET connection = m_connections[connectionId];
        if ( connection == nullptr )
        {
            connection = InternetConnect(
                m_inet,
                url.m_host.c_str( ),
                url.m_port,
                L"",
                L"",
                INTERNET_SERVICE_HTTP,
                0,
                0 );
            if ( connection == NULL )
                { m_connections.erase( connectionId ); }
            windows::check( connection != nullptr );
            m_connections[connectionId] = connection;
        }
        return connection;
    }

    Http::Request Http::Detail::startRequest( const URL & url, String method, String headers, Duration timeout )
    {
        HINTERNET connection = connect( url );
        return Request{ std::make_shared<Request::Detail>( connection, url, method, headers, timeout ) };
    }
 
    Http::Request Http::Detail::get( String url, String headers, Duration timeout )
    {
        return startRequest( URL{ url }, METHOD_GET, headers, timeout );
    }

    Http::Request Http::Detail::post( String url, String headers, Duration timeout )
    {
        return startRequest( URL{ url }, METHOD_POST, headers, timeout );
    }

    String::Array Http::Detail::connections( ) const
    {
        auto lock = m_mutex.lock( );

        String::Array results;
        for ( auto itr : m_connections )
            { results.push_back( itr.first ); }
        return results;
    }

    void Http::Detail::disconnect( String hostname )
    {
        auto lock = m_mutex.lock( );

        auto itr = m_connections.find( hostname );
        if ( itr != m_connections.end( ) )
        {
            InternetCloseHandle( itr->second );
            m_connections.erase( itr );
        }
    }

    void Http::Detail::disconnect( )
    {
        auto lock = m_mutex.lock( );

        for ( auto itr : m_connections )
            { InternetCloseHandle( itr.second ); }
        m_connections.clear( );
    }

    void Http::Detail::close( )
    {
        disconnect( );
        if ( m_inet )
        { 
            InternetSetStatusCallback( m_inet, nullptr );
            InternetCloseHandle( m_inet ); 
            m_inet = nullptr; 
        }
    }




    Http::Http( )
        : m_detail( std::make_shared<Detail>() ) { }

    Http::Http( String userAgent )
        : m_detail( std::make_shared<Detail>( ) ) { open(userAgent); }

    void Http::open( String userAgent )
        { m_detail->open( userAgent ); }

    void Http::close( )
        { m_detail->close( ); }

    String::Array Http::connections( ) const
        { return m_detail->connections( ); }

    void Http::disconnect( String hostname )
        { m_detail->disconnect( hostname ); }

    void Http::disconnectAll( )
        { m_detail->disconnect( ); }

    Http::Request Http::get( String url, String headers, Duration timeout )
        { return m_detail->get( url, headers, timeout ); }

    Http::Request Http::post( String url, String headers, Duration timeout )
        { return m_detail->post( url, headers, timeout ); }




}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/io/network/Http.h>
#include <cpp/io/reader/LineReader.h>
#include <cpp/util/Log.h>

SUITE( Http )
{
    using namespace cpp;

    TEST( get )
    {
        int millis = 10;
        while ( true )
        {
            try
            {
                auto timeout = Duration::ofMillis( millis );

                Http http{ "unittest" };
                auto request = http.get( "http://thotrot.com/", "", timeout );
                auto result = request.getReply( ).getAll( );
                http.disconnectAll( );

                CHECK( !result.empty( ) );
                break;
            }
            catch ( cpp::TimeoutException & e )
            {
                millis += 10;
                continue;
            }
            catch ( Exception & e )
            {
                test::log( e.what( ) );
                CHECK( false );
                break;
            }
        }
        test::log( String::format( "HTTP request succeeded with timeout of % millis", millis ) );
    }
}

#endif
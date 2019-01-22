#pragma once

#include <memory>
#include <functional>

#include "../process/Exception.h"
#include "../data/String.h"
#include "../time/Duration.h"
#include "../io/Input.h"
#include "../io/Output.h"

namespace cpp
{

    class Http
    {
    public:
        class Exception
            : public cpp::Exception
        {
        public:
            Exception( String message, int statusCode, String url, String headers )
                : cpp::Exception( makeMessage( message, statusCode, url ) ), m_statusCode( statusCode ), m_url(url), m_headers( headers ) { }
            int getStatusCode( ) const
                { return m_statusCode; }
            String url( ) const
                { return m_url; }
            String headers( ) const
                { return m_headers; }
        private:
            static String makeMessage( String msg, int statusCode, String url )
                { return cpp::String::format("% : % (%)", url, msg, statusCode); }
        private:
            int m_statusCode;
            String m_url;
            String m_headers;
        };

        class Request
        {
        public:
            Request & writeRequest( String input );
            Request & writeRequest( Input input );

            Input getReply( cpp::Duration timeout = cpp::Duration::Infinite );

            void close( cpp::Duration timeout = cpp::Duration::Infinite );

            int getStatusCode( ) const;

        private:
            struct Detail;
            std::shared_ptr<Detail> m_detail;

            friend class Http;
            Request( std::shared_ptr<Detail> && detail );
        };

    public:
        Http( );
        Http( String userAgent );

        void open( String userAgent = "ACME" );
        void close( );

        String::Array connections( ) const;
        void disconnect( String hostname );
        void disconnectAll( );

        Request get(
            String url, 
            String headers = "", 
            Duration timeout = Duration::ofSeconds( 3 ) );

        Request post(
            String url, 
            String headers = "", 
            Duration timeout = Duration::ofSeconds( 3 ) );


    private:
        struct Detail;
        std::shared_ptr<Detail> m_detail;
    };

}
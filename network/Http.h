#pragma once

/*
    Http 

    (1) Supports GET: String data = Http::get( url ).readAll()
    (2) Supports POST: String data = Http::post( url, headers, output ).readAll()
*/

#include <memory>
#include <functional>

#include "../../cpp/process/Exception.h"
#include "../../cpp/data/String.h"
#include "../../cpp/time/Duration.h"
#include "../../cpp/io/Input.h"
#include "../../cpp/io/Output.h"



namespace cpp
{

    class HttpClient;
    struct Http
    {
        class Url;
        class Exception;
        typedef HttpClient Client;

        static Input                        get(
                                                const Memory & url,
                                                const Memory & headers,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );

        static Input                        post(
                                                const Memory & url,
                                                const Memory & headers,
                                                const Memory & body,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );

		/*
        static Input                        post(
                                                const Memory & url,
                                                const Memory & headers,
                                                Input body,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );

        static Input                        post(
                                                const Memory & url,
                                                const Memory & headers,
                                                std::vector<Input> parts,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );
		*/

        static Client &                     client( );
    };



    class HttpClient
    {
    public:
                                            HttpClient( );

        String::Array						connections( ) const;
        void								disconnect( String hostname );
        void								disconnectAll( );

        Input                               get(
                                                const Memory & url,
                                                const Memory & headers,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );

        Input                               post(
                                                const Memory & url,
                                                const Memory & headers,
                                                const Memory & body,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );
		/*
        Input                               post(
                                                const Memory & url,
                                                const Memory & headers,
                                                Input body,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );

        Input                               post(
                                                const Memory & url,
                                                const Memory & headers,
                                                std::vector<Input> parts,
                                                Duration connectTimeout = Duration::ofSeconds( 3 ) );
		*/
        void                                init( ); // explicitly initialize http context, otherwise performed on-demand

    private:
		class Detail;
        std::shared_ptr<Detail>				m_detail;
    };



	class Http::Exception
		: public cpp::Exception
	{
	public:
											Exception( 
												const Memory & message,
												int statusCode, 
												const Memory & url,
												const Memory & headers );

		int									getStatusCode( ) const;
		String								url( ) const;
		String								headers( ) const;

	private:
		static String						makeMessage( const Memory & msg, int statusCode, const Memory & url );

	private:
		int									m_statusCode;
		String								m_url;
		String								m_headers;
	};



	inline Http::Exception::Exception( const Memory & message, int statusCode, const Memory & url, const Memory & headers )
		: cpp::Exception( makeMessage( message, statusCode, url ).data ), m_statusCode( statusCode ), m_url( url ), m_headers( headers ) 
	{ 
	}


	inline int Http::Exception::getStatusCode( ) const
	{
		return m_statusCode;
	}


	inline String Http::Exception::url( ) const
	{
		return m_url;
	}


	inline String Http::Exception::headers( ) const
	{
		return m_headers;
	}


	inline String Http::Exception::makeMessage( const Memory & msg, int statusCode, const Memory & url )
	{
		return cpp::String::format( "% : % (%)", url, msg, statusCode );
	}

}

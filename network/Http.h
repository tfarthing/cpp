#pragma once

#include <memory>
#include <functional>

#include "../../cpp/process/Exception.h"
#include "../../cpp/data/String.h"
#include "../../cpp/time/Duration.h"
#include "../../cpp/io/Input.h"
#include "../../cpp/io/Output.h"

namespace cpp
{

    class Http
    {
    public:
											Http( );
											Http( String userAgent );

        void								open( String userAgent = "cpp" );
        void								close( );

        String::Array						connections( ) const;
        void								disconnect( String hostname );
        void								disconnectAll( );

		class Exception;
		class Request;

		Request								get(
												String url, 
												String headers = "", 
												Duration timeout = Duration::ofSeconds( 3 ) );

        Request								post(
												String url, 
												String headers = "", 
												Duration timeout = Duration::ofSeconds( 3 ) );


    private:
        struct Detail;
        std::shared_ptr<Detail>				m_detail;
    };



	class Http::Exception
		: public cpp::Exception
	{
	public:
											Exception( 
												String message, 
												int statusCode, 
												String url, 
												String headers );

		int									getStatusCode( ) const;
		String								url( ) const;
		String								headers( ) const;

	private:
		static String						makeMessage( String msg, int statusCode, String url );

	private:
		int									m_statusCode;
		String								m_url;
		String								m_headers;
	};



	class Http::Request
	{
	public:
		Request &							writeRequest( String input );
		Request &							writeRequest( Input input );

		Input								getReply( cpp::Duration timeout = cpp::Duration::Infinite );

		void								close( cpp::Duration timeout = cpp::Duration::Infinite );

		int									getStatusCode( ) const;

	private:
		struct Detail;
		std::shared_ptr<Detail>				m_detail;

		friend class Http;
											Request( std::shared_ptr<Detail> && detail );
	};



	Http::Exception::Exception( String message, int statusCode, String url, String headers )
		: cpp::Exception( makeMessage( message, statusCode, url ) ), m_statusCode( statusCode ), m_url( url ), m_headers( headers ) 
	{ 
	}


	int Http::Exception::getStatusCode( ) const
	{
		return m_statusCode;
	}


	String Http::Exception::url( ) const
	{
		return m_url;
	}


	String Http::Exception::headers( ) const
	{
		return m_headers;
	}


	String Http::Exception::makeMessage( String msg, int statusCode, String url )
	{
		return cpp::String::format( "% : % (%)", url, msg, statusCode );
	}

}
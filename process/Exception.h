#pragma once

#include <exception>
#include <string>

namespace cpp
{

    class Exception 
		: public std::exception
    {
    public:
		Exception( const char * message )
			: std::exception( message ) { }
		Exception( std::string message )
			: m_what( std::move( message ) ) { }
        Exception( std::error_code error )
            : m_what( error.message( ) ) { }

		const char * what( ) const override
			{ return m_what.empty( ) ? std::exception::what( ) : m_what.c_str( ); }

    protected:
        std::string m_what;
    };

    struct InterruptException : public Exception
    {
        InterruptException( )
            : Exception( "Thread execution was interrupted." ) { }
    };

    struct TimeoutException : public Exception
    {
        TimeoutException( )
            : Exception( "The operation timed out" ) { }
    };

    struct OutOfBoundsException : public Exception
    {
        OutOfBoundsException( std::string message )
            : Exception( message ) { }
    };

    struct DecodeException : public Exception
    {
        DecodeException( std::string message )
            : Exception( message ) { }
    };

    struct IOException : public Exception
    {
		IOException( std::string message )
			: Exception( message ) { }
    };

    struct NoDataException : public IOException
    {
        NoDataException( )
            : IOException( "No data available for input." ) {}
        NoDataException( std::string message )
			: IOException( message ) { }
    };

    template<typename T> void check( bool condition )
        { if (!condition) { throw T{ }; } }

    template<typename T, typename... Params> void check( bool condition, Params... parameters )
        { if (!condition) { throw T{ parameters... }; } }
}

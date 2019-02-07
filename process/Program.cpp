#ifndef TEST

#include <cmath>
#include "../data/DataMap.h"
#include "../text/Utf16.h"
#include "../file/Files.h"
#include "../util/Log.h"
#include "../platform/windows/WindowsApp.h"
#include "AsyncIO.h"
#include "Platform.h"
#include "Program.h"



namespace cpp
{

	Program & program( )
	{
		return Program::instance( );
	}

	struct Program::Detail
	{
		Detail( );

		StringMap				args;
		String					exePath;
		String					workingPath;
		Logger					logger;
		std::mt19937_64			rand;
		AsyncIO					io;
	};

	Program::Detail::Detail( )
		: rand( cpp::Time::now( ).sinceEpoch( ).micros( ) )
	{

	}



	Program * Program::globalProgram = nullptr;
	thread_local std::vector<Program *> Program::threadProgram;


	Program::Program( )
		: detail( std::make_unique<Detail>( ) ) { init( ); }


	Program::Program( const String::Array & args )
		: detail( std::make_unique<Detail>( ) ) { initArgs( args ); }


	Program::Program( const wchar_t * cmdline )
		: detail( std::make_unique<Detail>( ) ) { initArgs( toUtf8( cmdline ) ); }


	Program::Program( int argc, const char ** argv )
		: detail( std::make_unique<Detail>( ) ) { initArgs( argc, argv ); }


	Program::Program( int argc, const wchar_t ** argv )
		: detail( std::make_unique<Detail>( ) ) { initArgs( argc, argv ); }


	Program::~Program( )
	{
		removeInstance( this );
	}


	void Program::initArgs( int argc, const wchar_t ** argv )
	{
		String::Array args;
		for ( int i = 0; i < argc; i++ )
		{
			args.push_back( cpp::toUtf8( argv[i] ) );
		}
		initArgs( args );
	}


	void Program::initArgs( int argc, const char ** argv )
	{
		String::Array args;
		for ( int i = 0; i < argc; i++ )
		{
			args.push_back( argv[i] );
		}
		initArgs( args );
	}


	void Program::initArgs( const String & cmdline )
	{
		detail->args.set( "cmdline", cmdline );
		String::Array arguments;
		if constexpr ( Platform::isWindows( ) )
		{
			arguments = windows::App::parseCommandLine( cmdline );
		}
		initArgs( arguments );
	}


	void Program::initArgs( const String::Array & arguments )
	{
		detail->args.set( "argc", Integer::toDecimal( arguments.size( ) ) );

		for ( size_t i = 0; i < arguments.size( ); i++ )
		{
			auto parts = arguments[i].split( "=", Memory::WhitespaceList, false );
			detail->args.set( parts[0], parts.size( ) > 1 ? parts[1] : "" );
			detail->args.set( String::format( "arg[%]", i ), arguments[i] );
		}

		init( );
	}


	void Program::init( )
	{
		std::wstring path( 1024, L'\0' );
		int len = GetModuleFileName( NULL, (LPWSTR)path.c_str( ), (DWORD)path.size( ) );
		if ( len > 0 )
		{
			detail->exePath = cpp::toUtf8( path.substr( 0, len ) );
		}

		len = GetCurrentDirectory( (DWORD)path.size( ), (LPWSTR)path.c_str( ) );
		if ( len > 0 )
		{
			detail->workingPath = cpp::toUtf8( path.substr( 0, len ) );
		}

		addInstance( this );
	}


	const Memory Program::arg( Memory key ) const
	{
		return detail->args.at( key );
	}


	const StringMap & Program::args( ) const
	{
		return detail->args;
	}


	const String & Program::exePath( )
	{
		return detail->exePath;
	}


	const String & Program::workingPath( )
	{
		return detail->workingPath;
	}


	AsyncIO & Program::asyncIO( )
	{
		return detail->io;
	}


	Logger & Program::logger( )
	{
		return detail->logger;
	}


	uint64_t Program::rand( )
	{
		return detail->rand( );
	}


	double Program::frand( )
	{
		return std::generate_canonical<double, std::numeric_limits<double>::digits>( detail->rand );
	}


	std::mt19937_64 & Program::getRandom( )
	{
		return detail->rand;
	}

}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/Program.h>

SUITE( Program )
{
    using namespace cpp;

    TEST( parse )
    {
        String::Array args;

        args = Program::Standard::parse( R"("abc" d e)" );
        CHECK(
            args.size( ) == 3 &&
            args[0] == "abc" &&
            args[1] == "d" &&
            args[2] == "e" );

        args = Program::Standard::parse( R"(a\\b d"e f"g h)" );
        CHECK(
            args.size( ) == 3 &&
            args[0] == R"(a\\b)" &&
            args[1] == "de fg" &&
            args[2] == "h" );

        args = Program::Standard::parse( R"(a\\\"b c d)" );
        CHECK(
            args.size( ) == 3 &&
            args[0] == R"(a\"b)" &&
            args[1] == "c" &&
            args[2] == "d" );

        args = Program::Standard::parse( R"(a\\\\"b c" d e)" );
        CHECK(
            args.size( ) == 3 &&
            args[0] == R"(a\\b c)" &&
            args[1] == "d" &&
            args[2] == "e" );
    }

}

#endif

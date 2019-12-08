#ifndef TEST

#include <cmath>
#include "../data/DataMap.h"
#include "../text/Utf16.h"
#include "../file/FilePath.h"
#include "../util/Log.h"
#include "../network/Http.h"
#include "../platform/windows/WindowsApp.h"
#include "AsyncIO.h"
#include "Platform.h"
#include "Program.h"
#include "Random.h"



namespace cpp
{

	struct Program::Detail
	{
											Detail( );

		StringMap							args;
		FilePath							modulePath;
        AsyncIO								io;
        Logger								logger;
        Http::Client                        http;
		Random								rng;
	};

	Program::Detail::Detail( )
	{

	}



	Program * Program::globalProgram = nullptr;
	thread_local std::vector<Program *> Program::threadProgram;


	Program::Program( )
		: detail( std::make_unique<Detail>( ) ) { init( ); }


    Program::Program( int argc, const char ** argv )
        : detail( std::make_unique<Detail>( ) ) { initArgs( argc, argv ); }


    Program::Program( int argc, const wchar_t ** argv )
        : detail( std::make_unique<Detail>( ) ) { initArgs( argc, argv ); }


    Program::Program( const MemoryArray & args )
        : detail( std::make_unique<Detail>( ) ) { initArgs( args ); }


	Program::~Program( )
	{
		removeInstance( this );
	}


	void Program::initArgs( int argc, const wchar_t ** argv )
	{
		StringArray args;
		for ( int i = 0; i < argc; i++ )
		{
			args.add( cpp::toUtf8( argv[i] ) );
		}
		initArgs( args );
	}


	void Program::initArgs( int argc, const char ** argv )
	{
		StringArray args;
		for ( int i = 0; i < argc; i++ )
		{
			args.add( argv[i] );
		}
		initArgs( args );
	}


	void Program::initArgs( const std::string & cmdline )
	{
		detail->args.set( "cmdline", cmdline );
		StringArray arguments;
		if constexpr ( Platform::isWindows( ) )
		{
			for ( auto & arg : windows::App::parseCommandLine( cmdline ).data )
				{ arguments.add( arg.data ); }
		}
		initArgs( arguments );
	}


	void Program::initArgs( const MemoryArray & arguments )
	{
		detail->args.set( "argc", Integer::toDecimal( arguments.size( ) ) );

		for ( size_t i = 0; i < arguments.size( ); i++ )
		{
			Memory arg = arguments[i];
			auto parts = arg.split( "=", Memory::WhitespaceList, false );
			detail->args.set( parts[0], parts.size( ) > 1 ? parts[1] : "" );
			detail->args.set( cpp::format( "arg[%]", i ), arguments[i] );
		}
    
        init( );
    }


	void Program::init( )
	{
        initModulePath( );
		addInstance( this );
	}


    void Program::initModulePath( )
    {
        std::wstring path( 1024, L'\0' );
        int len = GetModuleFileName( NULL, (LPWSTR)path.c_str( ), (DWORD)path.size( ) );
        if ( len > 0 )
            { detail->modulePath = cpp::toUtf8( path.substr( 0, len ) ); }
    }

 
	const Memory Program::arg( Memory key )
	{
		return instance().detail->args.get( key );
	}


	const StringMap & Program::args( )
	{
		return instance( ).detail->args;
	}


    String Program::env( Memory key )
    {
		size_t retval;
		wchar_t buffer[1024];
		::_wgetenv_s( &retval, buffer, 1024, toUtf16( key ).c_str( ) );
		return toUtf8( buffer );
    }


	const FilePath & Program::modulePath( )
	{
		return instance( ).detail->modulePath;
	}


	Logger & Program::logger( )
	{
		return instance( ).detail->logger;
	}


    HttpClient & Program::http( )
    {
        return instance( ).detail->http;
    }


	Random & Program::rng( )
	{
		return instance( ).detail->rng;
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

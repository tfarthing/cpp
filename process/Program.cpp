#ifndef TEST

#include <cmath>
#include <cpp/Integer.h>
#include <cpp/Program.h>
#include <cpp/chrono/Timer.h>
#include <cpp/util/Utf16.h>

namespace cpp
{

    Program::Standard * Program::s_program = nullptr;
    thread_local Program::Standard * Program::t_program = nullptr;

	Program::Standard::Standard()
		: m_isRunning(true), m_rand(cpp::Time::now().sinceEpoch().micros()) { init(); }

	Program::Standard::Standard( const wchar_t * cmdline )
        : m_isRunning( true ), m_rand( cpp::Time::now( ).sinceEpoch( ).micros( ) ) { initArgs( toUtf8( cmdline ) ); }

	Program::Standard::Standard( int argc, const char ** argv )
        : m_isRunning( true ), m_rand( cpp::Time::now( ).sinceEpoch( ).micros( ) ) { initArgs( argc, argv ); }

	Program::Standard::Standard( int argc, const wchar_t ** argv )
        : m_isRunning( true ), m_rand( cpp::Time::now( ).sinceEpoch( ).micros( ) ) { initArgs( argc, argv ); }
	
	Program::Standard::~Standard()
	{
        m_asyncIO.stop( );
		clearInstance( this );
	}

    bool Program::Standard::update( )
    {
        m_bus.update( );
        m_asyncIO.poll( );
        return isRunning( );
    }
    
    bool Program::Standard::updateUntil( const cpp::Time & time )
    {
        while ( time > cpp::Time::now( ) )
        {
            m_bus.update( );
            cpp::Time nextTime = std::min( time, cpp::Time::now( ) + cpp::Duration::ofMillis( 10 ) );
            m_asyncIO.runUntil( nextTime );
        }
        return isRunning( );
    }

    void Program::Standard::initArgs( int argc, const wchar_t ** argv )
    {
        String::Array args;
        for ( int i = 0; i < argc; i++ )
        {
            args.push_back( cpp::toUtf8(argv[i]) );
        }
        initArgs( args );
    }

    void Program::Standard::initArgs( int argc, const char ** argv )
    {
        String::Array args;
        for ( int i = 0; i < argc; i++ )
            { args.push_back( argv[i] ); }
        initArgs( args );
    }

    void Program::Standard::initArgs(const String & cmdline)
	{
        m_args.set( "cmdline", cmdline );
        String::Array arguments = parse( cmdline );
        initArgs( arguments );
    }

	void Program::Standard::initArgs(const String::Array & arguments)
	{
        m_args.set( "argc", Integer::toDecimal( arguments.size() ) );
        for ( size_t i = 0; i < arguments.size( ); i++ )
        {
            const String & arg = arguments[i];
            m_args.addText( arg );
            m_args.set( String::format( "arg[%]", Integer::toDecimal(i) ), arg );
        }

        init( );
	}

	void Program::Standard::init()
	{
        std::wstring path( 1024, L'\0' );
        int len = GetModuleFileName( NULL, (LPWSTR)path.c_str( ), (DWORD)path.size( ) );
        if ( len > 0 )
            { m_exePath = toUtf8( path ).substr(0, len); }

        len = GetCurrentDirectory( (DWORD)path.size( ), (LPWSTR)path.c_str( ) );
        if ( len > 0 )
            { m_workingPath = toUtf8( path ).substr(0, len); }

		setInstance( this );
	}

    /*
    Windows cmdline parsing standards: https://msdn.microsoft.com/en-us/library/17w5ykft.aspx
    * Arguments are delimited by white space, which is either a space or a tab.
    * A string surrounded by double quotation marks ("string") is interpreted as a single 
      argument, regardless of white space contained within. A quoted string can be embedded 
      in an argument.
    * A double quotation mark preceded by a backslash (\") is interpreted as a literal double 
      quotation mark character (").
    * Backslashes are interpreted literally, unless they immediately precede a double 
      quotation mark.
    * If an even number of backslashes is followed by a double quotation mark, one backslash 
      is placed in the argv array for every pair of backslashes, and the double quotation 
      mark is interpreted as a string delimiter.
    * If an odd number of backslashes is followed by a double quotation mark, one backslash 
      is placed in the argv array for every pair of backslashes, and the double quotation 
      mark is "escaped" by the remaining backslash, causing a literal double quotation mark 
      (") to be placed in argv.
    */
    String::Array Program::Standard::parse( const String & cmdline )
    {
        String::Array arguments;

        size_t pos = 0;
        size_t argIndex = 0;
        bool isQuoted = false;
        size_t bslashCount = 0;
        String arg;

        while ( pos < cmdline.length( ) + 1 )
        {
            char ch = ( pos < cmdline.length( ) ) ? cmdline[pos] : 0;
            switch ( ch )
            {
            case ' ':
            case '\t':
            case '\0':
                //  quoted string ignores space and tab
                if ( isQuoted && ch != '\0')
                    { pos += 1; break; }

                if ( !arg.isEmpty() || argIndex < pos )
                {
                    arg += cmdline.substr( argIndex, pos - argIndex );
                    arguments.push_back( arg );
                    arg.clear( );
                }
                bslashCount = 0;
                pos += 1;
                argIndex = pos;
                break;

            case '\"':
                //  append everything upto quote or first backslash before quote
                arg += cmdline.substr( argIndex, pos - argIndex - bslashCount );

                //  append a backslash for every two before quote
                for ( size_t i = 0; i < bslashCount / 2; i++ )
                    { arg += "\\"; }

                // odd backslash count == literal quote
                if ( ( bslashCount % 2 ) != 0 ) 
                    { arg += "\""; }
                // else quote is a string delimiter
                else
                    { isQuoted ^= true; }

                bslashCount = 0;
                pos += 1;
                argIndex = pos;
                break;

            default:
                bslashCount = ( ch == '\\' ) ? bslashCount + 1 : 0;
                pos += 1;
                break;
            }
        }

        return arguments;
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

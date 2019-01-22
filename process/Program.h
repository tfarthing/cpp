#pragma once

#include <random>
#include <memory>

#include "../data/Memory.h"


namespace cpp
{

	struct StringMap;
	class Output;
	class Input;
	class Logger;
    class Program;
	class AsyncIO;



    Program & program( );

    class Program
    {
    public:
        Program( );
        Program( const String::Array & args );
        Program( const wchar_t * cmdline );
        Program( int argc, const char ** argv );
        Program( int argc, const wchar_t ** argv );
        ~Program( );

        Input &					in( );
        Output &				out( );
        Output &				error( );

        const Memory			arg( Memory key ) const;
        const StringMap &		args( ) const;

		const FilePath &		exePath( );
		const FilePath &		workingPath( );
        
		AsyncIO &				asyncIO( );
        
		Logger &				logger( );
        
		uint64_t				rand( );
        double					frand( );
        std::mt19937_64 &		getRandom( );

    protected:
        void initArgs( int argc, const wchar_t ** argv );
        void initArgs( int argc, const char ** argv );
        void initArgs( const String & cmdline );
        void initArgs( const String::Array & arguments );
        void init( );

    protected:
		struct Detail;
		std::unique_ptr<Detail> detail;

    private:
		friend Program & program( );
        static Program & instance( );
        static void addInstance( Program * instance );
        static void removeInstance( Program * instance );

    private:
        static Program * globalProgram;
        static thread_local std::vector<Program *> threadProgram;
    };


    Program & Program::instance( )
    {
		return threadProgram.empty( ) ? *globalProgram : *threadProgram.back( );
    }


    void Program::addInstance( Program * instance )
    {
        if ( !globalProgram )
        {
			globalProgram = instance;
        }
		threadProgram.push_back( instance );
    }


    void Program::removeInstance( Program * instance )
    {
        if ( globalProgram == instance )
        {
			globalProgram = nullptr;
        }
		threadProgram.erase( std::remove( threadProgram.begin(), threadProgram.end(), instance ) );
    }
	   	  
}
#pragma once

#include <random>
#include <memory>



namespace cpp
{

	struct StringMap;
	class Output;
	class Input;
	class Logger;
	class FilePath;
	class Random;



    class Program
    {
    public:
											Program( );
											Program( const String::Array & args );
											Program( const wchar_t * cmdline );
											Program( int argc, const char ** argv );
											Program( int argc, const wchar_t ** argv );
											~Program( );

        static Input &						in( );
		static Output &						out( );
		static Output &						error( );

		static const Memory					arg( Memory key );
		static const StringMap &			args( );

		static const FilePath &				modulePath( );
        
		static Logger &						logger( );
        
		static Random &						rng( );

    protected:
        void								initArgs( int argc, const wchar_t ** argv );
        void								initArgs( int argc, const char ** argv );
        void								initArgs( const String & cmdline );
        void								initArgs( const String::Array & arguments );
        void								init( );

    protected:
		struct Detail;
		std::unique_ptr<Detail> detail;

    private:
        static Program &					instance( );
        static void							addInstance( Program * instance );
        static void							removeInstance( Program * instance );

    private:
        static Program *					globalProgram;
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
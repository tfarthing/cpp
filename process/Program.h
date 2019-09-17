#pragma once

/*
    Program is an encapsulation of singletons that provide an interface in any cpp module. These include:
        * Program Arguments and Environment : argc, argv, env(), module path
        * Input & Output : i.e. stdin & stdout
        * Logging
        * Http
        * Random Number Generator
        * Also instance accessor of any platform specific singletons (e.g. console or application handles, font maps, ect.)

    In general, any interface that wants to provide static accessors will use the Program object for access so that the 
    lifetime of the singleton can be scoped inside of main().
*/

#include <../../cpp/data/DataArray.h>
#include <../../cpp/data/DataMap.h>



namespace cpp
{

    struct FilePath;
    class Output;
	class Input;
	class Logger;
	class Random;
    class HttpClient;



    class Program
    {
    public:
											Program( );
                                            Program( int argc, const char ** argv );
                                            Program( int argc, const wchar_t ** argv );
                                            Program( const MemoryArray & args );
                                            ~Program( );

		static const Memory					arg( Memory key );
		static const StringMap &			args( );

        static const Memory					env( Memory key );

		static const FilePath &				modulePath( );
        
		static Logger &						logger( );
        static HttpClient &                 http( );
		static Random &						rng( );

    protected:
        void                                initModulePath( );
        void								initArgs( int argc, const wchar_t ** argv );
        void								initArgs( int argc, const char ** argv );
        void								initArgs( const std::string & cmdline );
        void								initArgs( const MemoryArray & arguments );
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


	inline Program & Program::instance( )
    {
		return threadProgram.empty( ) ? *globalProgram : *threadProgram.back( );
    }


	inline void Program::addInstance( Program * instance )
    {
        if ( !globalProgram )
        {
			globalProgram = instance;
        }
		threadProgram.push_back( instance );
    }


	inline void Program::removeInstance( Program * instance )
    {
        if ( globalProgram == instance )
        {
			globalProgram = nullptr;
        }
		threadProgram.erase( std::remove( threadProgram.begin(), threadProgram.end(), instance ) );
    }
	   	  
}
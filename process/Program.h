#pragma once

#include <random>
#include <memory>
#include <string>
#include <vector>

#include <../../cpp/data/Memory.h>
#include <../../cpp/data/DataMap.h>



namespace cpp
{

	class Output;
	class Input;
	class Logger;
	class FilePath;
	class Random;



    class Program
    {
    public:
		typedef std::vector<std::string> StringArray_t;

											Program( );
											Program( const StringArray_t & args );
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
        void								initArgs( const std::string & cmdline );
        void								initArgs( const StringArray_t & arguments );
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
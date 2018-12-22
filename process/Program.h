#pragma once

#include <random>
#include <memory>

#include <cpp/Data.h>
#include <cpp/File.h>
#include <cpp/Log.h>

namespace cpp
{

	class DataMap;
	class Output;
	class Input;
	class Logger;
    class Program;



    Program & program( );

    class Program
    {
    public:
        Program( );
        Program( String::Array args );
        Program( const wchar_t * cmdline );
        Program( int argc, const char ** argv );
        Program( int argc, const wchar_t ** argv );
        ~Program( );

        Input &					in( );
        Output &				out( );
        Output &				error( );

        const Memory			arg( Memory key ) const;
        const DataMap &			args( ) const;

		FilePath				exePath( );
        FilePath				workingPath( );
        
		AsyncIO &				asyncIO( );
        
		Logger &				logger( );
        
		uint64_t				rand( );
        f64_t					frand( );
        std::mt19937_64 &		getRandom( );

        bool					poll( );
        void					run( );

    protected:
        void initArgs( int argc, const wchar_t ** argv );
        void initArgs( int argc, const char ** argv );
        void initArgs( const String & cmdline );
        void initArgs( const String::Array & arguments );
        void init( );

    protected:
		DataMap					args;
        FilePath				exePath;
        FilePath				workingPath;
        Logger					logger;
        std::mt19937_64			rand;
        AsyncIO					io;

    private:
        static Program & instance( );
        static void addInstance( Program * instance );
        static void removeInstance( Program * instance );

    private:
        static Program * globalProgram;
        static thread_local std::vector<Program *> threadProgram;
    };



    class Standard
    {
    public:
        Standard( );
        Standard( const wchar_t * cmdline );
        Standard( int argc, const char ** argv );
        Standard( int argc, const wchar_t ** argv );
        virtual ~Standard( );

        Memory arg( const String & key ) const
        {
            return m_args.get( key );
        }

        const Properties & args( ) const
        {
            return m_args;
        }

        FilePath exePath( ) const
        {
            return m_exePath;
        }

        FilePath workingPath( ) const
        {
            return m_workingPath;
        }

        AsyncIO & asyncIO( )
        {
            return m_asyncIO;
        }

        Bus & bus( )
        {
            return m_bus;
        }

        Logger & logger( )
        {
            return m_logger;
        }

        uint64_t rand( )
        {
            return m_rand( );
        }

        f64_t frand( )
        {
            return std::generate_canonical<f64_t, std::numeric_limits<f64_t>::digits>( m_rand );
        }

        std::mt19937_64 & getRandom( )
        {
            return m_rand;
        }

        bool isRunning( ) const
        {
            return m_isRunning;
        }

        virtual bool update( );
        virtual bool updateUntil( const cpp::Time & time );

        static Standard & program( )
        {
            return Program::program( );
        }

        static String::Array parse( const String & cmdline );

    protected:
        void initArgs( int argc, const wchar_t ** argv );
        void initArgs( int argc, const char ** argv );
        void initArgs( const String & cmdline );
        void initArgs( const String::Array & arguments );
        void init( );

    protected:
        bool m_isRunning;
        Properties m_args;
        FilePath m_exePath;
        FilePath m_workingPath;
        Logger m_logger;
        std::mt19937_64 m_rand;
        AsyncIO m_asyncIO;
        Bus m_bus;
    };

    class Console : public Standard
    {
    public:
        Console( );
        Console( int argc, const char ** argv );
        Console( int argc, const wchar_t ** argv );

        ConsoleUI & ui( )
        {
            return *m_ui;
        }
        static ConsoleUI & getUI( )
        {
            Console & console = (Console &)program( ); return console.ui( );
        }

    private:
        std::shared_ptr<ConsoleUI> m_ui;
    };

    class WinApp : public Standard
    {
    public:
        WinApp( void * instance, wchar_t * cmdline );

        static void registerModelessDialog( HWND hwnd );

        bool update( ) override;
        bool updateUntil( const cpp::Time & time ) override;

        GraphicsUI & ui( );
        static GraphicsUI & getUI( )
        {
            WinApp & winapp = (WinApp &)program( ); return winapp.ui( );
        }

    private:
        class Detail;
        std::shared_ptr<Detail> m_detail;
    };



    const Memory Program::arg( const String & key )
    {
        return program( ).arg( key );
    }

    const Properties & Program::args( )
    {
        return program( ).args( );
    }

    FilePath Program::exePath( )
    {
        return program( ).exePath( );
    }

    FilePath Program::workingPath( )
    {
        return program( ).workingPath( );
    }

    AsyncIO & Program::asyncIO( )
    {
        return program( ).asyncIO( );
    }

    Bus & Program::bus( )
    {
        return program( ).bus( );
    }

    Logger & Program::logger( )
    {
        return program( ).logger( );
    }

    uint64_t Program::rand( )
    {
        return program( ).rand( );
    }

    f64_t Program::frand( )
    {
        return program( ).frand( );
    }

    std::mt19937_64 & Program::getRandom( )
    {
        return program( ).getRandom( );
    }

    bool Program::update( )
    {
        return program( ).update( );
    }

    bool Program::updateUntil( const cpp::Time & time )
    {
        return program( ).updateUntil( time );
    }

    Program & Program::instance( )
    {
        return t_program ? *t_program : *s_program;
    }

    void Program::setInstance( Standard * instance )
    {
        if ( !s_program )
        {
            s_program = instance;
        }
        if ( !t_program )
        {
            t_program = instance;
        }
    }

    void Program::clearInstance( Standard * instance )
    {
        if ( s_program == instance )
        {
            s_program = nullptr;
        }
        if ( t_program == instance )
        {
            t_program = nullptr;
        }
    }



}
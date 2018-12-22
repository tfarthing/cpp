#pragma once

#include <cpp/Program.h>
#include <cpp/chrono/Duration.h>
#include <cpp/io/Input.h>
#include <cpp/io/Output.h>
#include <cpp/io/file/FilePath.h>

namespace cpp
{

    class Process
    {
    public:
        static Process run( FilePath exe, String cmdline, FilePath workingPath = FilePath{ } );
        static Process runChild( FilePath exe, String cmdline, FilePath workingPath = FilePath{ } );

        Process( );
        ~Process( );

        bool isRunning( ) const;
        void wait( );
        bool waitFor( Duration timeout );
        bool waitUntil( Time time );
        void detach( );
        void close( );

        Output input( );
        Input output( );
        Input error( );

        FilePath exe( ) const;
        String cmdline( ) const;

    private:
        struct Detail;
        Process( std::shared_ptr<Detail> && detail );

        std::shared_ptr<Detail> m_detail;
    };

}

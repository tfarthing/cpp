#pragma once

#include <cstdio>
#include <cpp/Memory.h>
#include <cpp/io/Output.h>
#include <cpp/util/Handle.h>

namespace cpp
{

    class StandardOutput
        : public Output
    {
    public:
        static StandardOutput & output( )
            { return s_instance; }

        static void print( const Memory & string )
        {
            fwrite( string.begin(), 1, string.length(), stdout );
        }

        template<typename... Params>
        static void print( const Memory & fmt, Params... parameters )
        {
            print( String::format( fmt, parameters... ) );
        }

    private:
        StandardOutput( )
            : Output( Handle<Sink>{ } ) { }

        class Sink
            : public Output::Sink_t
        {
        public:
            Sink( )
            {
            }

            bool isOpen( ) const override
            {
                return true;
            }
            Memory write( const Memory & src ) override
            {
                size_t len = fwrite( src.begin( ), 1, src.length( ), stdout );
                return src.substr( 0, len );
            }
            void flush( ) override
            {
                fflush( stdout );
            }

        };

        static StandardOutput s_instance;
     };

    inline void print( const Memory & string )
    {
        StandardOutput::print( string );
    }

    template<typename... Params>
    void print( const Memory & fmt, Params... parameters )
    {
        StandardOutput::print( fmt, parameters... );
    }

}


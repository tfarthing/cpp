#pragma once

#include <memory>
#include "../data/Memory.h"
#include "../data/String.h"

namespace cpp
{

    class Output
    {
    public:
        struct Sink_t;
        typedef std::shared_ptr<Sink_t> Sink;

        Output( )
            : m_sink( nullptr ) { }
        Output( nullptr_t )
            : m_sink( nullptr ) { }
        Output( Output::Sink sink )
            : m_sink( std::move( sink ) ) { }

        void close( )
            { if ( m_sink ) { m_sink->close( ); m_sink = nullptr; } }

        explicit operator bool( ) const
            { return isOpen( ); }
        
        bool isOpen( ) const
            { return m_sink && m_sink->isOpen( ); }
        
        //  block until something is written
        Memory write( const Memory & src )
            { return m_sink ? m_sink->write( src ) : nullptr; }

        //  block until all is written
        Memory put( const Memory & src );

        template<class T, typename = typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type> 
        Memory put(const T & value)
            { return put( Memory::ofValue( value ) ); }

        //  print formatted string
        void print( const Memory & string )
            { put( string ); }

        template<typename... Params>
        void print( const Memory & fmt, Params... parameters )
            { put( String::format( fmt, parameters... ) ); }

        /*
        template<typename T, typename... Params>
        void print( const Memory & fmt, const T & param, Params... parameters )
            { put( String::format( fmt, param, parameters... ) ); }
        */

        //  block until data is written
        void flush()
            { if (m_sink) { m_sink->flush(); } }

    public:
        struct Sink_t
        {
            virtual ~Sink_t()
                { close(); }
            virtual bool isOpen( ) const = 0;
            virtual Memory write( const Memory & src ) = 0;
            virtual void flush( ) = 0;
            virtual void close( )
                {}
        };

    protected:
        Sink m_sink;

    };

}

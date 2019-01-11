#pragma once

#include <memory>
#include "../data/Memory.h"
#include "../time/Duration.h"
#include "reader/ByteReader.h"
#include "reader/LineReader.h"
#include "reader/SearchReader.h"

namespace cpp
{
    class DataBuffer;
    class Input
    {
    public:
        struct Source_t;
        typedef std::shared_ptr<Source_t> Source;

        Input( )
            : m_source( nullptr ) { }
        Input( nullptr_t )
            : m_source( nullptr ) { }
        Input( Input::Source source )
            : m_source( std::move( source ) ) { }

        void close( )
            { if ( m_source ) { m_source->close( ); } }

        explicit operator bool( ) const
            { return isOpen( ); }
        
        bool isOpen( ) const
            { return m_source && m_source->isOpen( ); }
        
        //  block until something is read, or null if timeout expires
        Memory read( Memory dst, Duration timeout = Duration::Infinite )
            { return m_source ? m_source->read( dst, timeout ) : nullptr; }
        Memory read( DataBuffer & dst, Duration timeout = Duration::Infinite );

        //  block until all is read
        Memory get( Memory dst );
        Memory get( DataBuffer & dst );
        String getAll( );

        template<class T, typename = std::enable_if_t<std::is_scalar<T>::value>> 
        T & getValue( T & dst )
            { get( Memory::ofValue<T>( dst ) ); return dst; }

        //  for asynchronous input
        /*
        using ReadFn = std::function<void( const String & data )>;
        using CloseFn = std::function<void( std::error_code error )>;
        void async( const ReadFn & readFn, const CloseFn & closeFn, const Bus & bus = nullptr );
        */

        //  readers
        ByteReader bytes( size_t buflen = 64 * 1024, Duration timeout = Duration::Infinite )
            { return ByteReader{ *this, buflen, timeout }; }
        LineReader lines( size_t buflen = 64 * 1024, Duration timeout = Duration::Infinite )
            { return LineReader{ *this, buflen, timeout }; }
        SearchReader search( Memory regex, size_t buflen = 64 * 1024, Duration timeout = Duration::Infinite )
            { return SearchReader{ regex, *this, buflen, timeout }; }

    public:
        struct Source_t
        {
            virtual ~Source_t()
                { close(); }
            virtual bool isOpen( ) const = 0;
            virtual Memory read( const Memory & dst, Duration timeout ) = 0;
            virtual void close( )
                { }
            //virtual void async( const ReadFn & readFn, const CloseFn & closeFn, const Bus & bus );
        };

    protected:
        Source m_source;
    };

}
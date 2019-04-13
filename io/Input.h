#pragma once

/*

    Input is a class that abstracts a synchronous (blocking) input source.  Examples of these could be 
    simple file input, TCP socket input, and process' pipe input.

    (1) It is copyable and ref-counted using a shared-pointer.
    (2) Works for both finite and infinite input streams.  Behavior consistent for both.
    (3) There are three possible error conditions while reading from an input:
        * The user explicitly closes the input before all its data is read (ECONNABORTED).
        * An IO error occurs, i.e. a device error (EIO).
        * The user expects to read data beyond the end-of-data (ENODATA).
        These will be thrown as an Input::Exception, or alternatively reported in the std::error_code if provided.

    The Input implementation has the following properties:

    (1) Requires a Source which is instantiated as a shared_ptr.
    (2) The method named readsome() has these propertes:
        (2a) The read dst buffer is passed in so that it is possible for the implementation to avoid unneeded memcpy.
        (2b) Will block until something is read or the input is closed (i.e. isOpen() == false).
        (2c) The input source will be closed automatically once the end-of-data is reached (if any).  
             In this case readsome() will return immediately.  It is valid for readsome() to 
             return an empty buffer once isOpen() == false.
             * The source implementation is what decides if the end-of-data is reached, and whether there is 
               an end-of-data.  In cases where the source is streaming and no data is available, readsome will 
               block until data is available to read.
        (2d) The input source can be closed for the following reasons:
            * The input reads up to the end-of-data.  No error_code is set for this condition.
            * The user closes the input before reading to the end-of-data.  The error_code will be set to 
              connection_aborted (ECONNABORTED).
            * An io error occurs which closes the input source.  The error_code will be set to io_error (EIO).
    (3) The method named read() will block until the entire buffer is read or the input source is closed.
        (3a) In addition to the error conditions possible from readsome, the following error is possible:
            * The user attempts to read past the end-of-data.  The error_code will be set to 
              no_message_available (ENODATA).

*/


#include <cassert>
#include "../../cpp/data/String.h"
#include "../../cpp/io/LineReader.h"



namespace cpp
{

    class Input
    {
    public:
        struct Source
        {
            typedef std::shared_ptr<Source> ptr_t;
            
            virtual                         ~Source( );
            virtual bool                    isOpen( ) const = 0;
            virtual Memory                  readsome( Memory dst, std::error_code & errorCode ) = 0;
            virtual void                    close( ) = 0;
        };
        struct Exception;

                                            Input( );
                                            Input( nullptr_t );
                                            Input( Source::ptr_t source );

        void                                close( );

        bool                                isOpen( ) const;
                                            operator bool( ) const;

        Memory 					            readsome( Memory buffer );
        Memory 					            readsome( Memory buffer, std::error_code & errorCode );

        Memory 					            read( Memory buffer );
        Memory                              read( Memory buffer, std::error_code & errorCode );

        String                              readAll( );

        LineReader                          lines( size_t buflen = LineReader::MaxLineLength );

    private:
        Source::ptr_t                       m_source;
    };



    struct Input::Exception
        : public cpp::Exception
    {
        Exception( std::error_code error )
            : cpp::Exception( getMessage( error ) )
        {  
        }

        static const char * getMessage( std::error_code error )
        {
            if ( error == std::errc::no_message_available )
                { return "ENODATA - an attempt was made to read past the end-of-data"; }
            else if ( error == std::errc::connection_aborted )
                { return "ECONNABORTED - input was closed before all the data could be read."; }
            else
                { return "EIO - device error occurred during input."; }
        }
    };



    Input::Input( )
        : m_source( nullptr )
    {
    }


    Input::Input( nullptr_t )
        : m_source( nullptr )
    {
    }


    Input::Input( Source::ptr_t source )
        : m_source( source )
    {
    }


    void Input::close( )
    {
        if ( m_source )
            { m_source->close( ); }
    }


    bool Input::isOpen( ) const
    {
        return m_source ? m_source->isOpen( ) : false;
    }


    Input::operator bool( ) const
    {
        return isOpen( );
    }


    Memory Input::readsome( Memory buffer )
    {
        if ( !m_source )
            { return Memory::Empty; }

        std::error_code errorCode;
        buffer = m_source->readsome( buffer, errorCode );
        check<Input::Exception>( !errorCode, errorCode );
        return buffer;
    }


    Memory Input::readsome( Memory buffer, std::error_code & errorCode )
    {
        if ( !m_source )
            { return Memory::Empty; }

        return m_source->readsome( buffer, errorCode );
    }

    Memory Input::read( Memory buffer )
    {
        std::error_code errorCode;
        buffer = read( buffer, errorCode );
        check<Input::Exception>( !errorCode, errorCode );
        return buffer;
    }

    Memory Input::read( Memory buffer, std::error_code & errorCode )
    {
        size_t bytes = 0;
        if ( m_source )
        {
            while ( isOpen( ) && bytes < buffer.length( ) && !errorCode )
                { bytes += readsome( buffer.substr( bytes ), errorCode ).length( ); }
        }
        if ( !errorCode && bytes < buffer.length( ) )
        {
            errorCode = std::make_error_code( std::errc::no_message_available ); 
            bytes = 0;
        }

        return buffer.substr(0, bytes);
    }

    String Input::readAll( )
    {
        StringBuffer buffer{ 64 * 1024 };
        while ( isOpen( ) )
        {
            if ( buffer.putable( ).isEmpty( ) )
                { buffer.resize( buffer.size( ) * 2 ); }
            buffer.put( readsome( buffer.putable( ) ).length( ) );
        }
        return buffer.getable( );
    }


    LineReader Input::lines( size_t buflen )
    {
        return LineReader{ *this, buflen };
    }



    Input::Source::~Source( )
    {
        close( );
    }


    /*
    void Input::Source::close( )
    {
        assert( false ); // implementation should always override this
    }
    */

}

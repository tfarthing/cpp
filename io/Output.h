#pragma once

#include <cassert>
#include "../../cpp/process/Exception.h"
#include "../../cpp/data/String.h"



namespace cpp
{
    class Output
    {
    public:
        struct Sink
        {
            typedef std::shared_ptr<Sink>   ptr_t;

            virtual                         ~Sink( );
            virtual bool                    isOpen( ) const = 0;
            virtual Memory                  write( Memory src, std::error_code & errorCode ) = 0;
            virtual void                    flush( ) = 0;
            virtual void                    close( ) = 0;
        };
        struct Exception;
               
                                            Output( );
                                            Output( nullptr_t );
                                            Output( Sink::ptr_t sink );

        void                                flush( );
        void                                close( );
                                            
                                            operator bool( ) const;
        bool                                isOpen( ) const;

        Memory                              write( Memory src );
        Memory                              write( Memory src, std::error_code & errorCode );

        void                                print( Memory string );
        template<typename... Params> void   print( Memory fmt, Params... parameters );

    protected:
        Sink::ptr_t                         m_sink;
    };



    struct Output::Exception
        : public cpp::Exception
    {
        Exception( std::error_code error )
            : cpp::Exception( getMessage( error ) )
        {  
        }

        static const char * getMessage( std::error_code error )
        {
            if ( error == std::errc::connection_aborted )
                { return "ECONNABORTED - output was closed before all the data could be written."; }
            else
                { return "EIO - device error occurred during output."; }
        }
    };



    Output::Output( )
        : m_sink( nullptr ) 
    { 
    }


    Output::Output( nullptr_t )
        : m_sink( nullptr ) 
    { 
    }


    Output::Output( Sink::ptr_t sink )
        : m_sink( std::move( sink ) ) 
    { 
    }


    void Output::close( )
    {
        if ( m_sink ) 
            { m_sink->close( ); }
    }


    Output::operator bool( ) const
    {
        return isOpen( );
    }


    bool Output::isOpen( ) const
    {
        return m_sink && m_sink->isOpen( );
    }


    Memory Output::write( Memory buffer )
    {
        std::error_code errorCode;
        buffer = write( buffer, errorCode );
        check<Output::Exception>( !errorCode, errorCode );
        return buffer;
    }


    Memory Output::write( Memory buffer, std::error_code & errorCode )
    {
        if ( !m_sink )
            { return Memory::Empty; }

        return m_sink->write( buffer, errorCode );
    }


    void Output::print( Memory string )
    {
        write( string );
    }


    template<typename... Params>
    void Output::print( Memory fmt, Params... parameters )
    {
        write( String::format( fmt, parameters... ) );
    }


    void Output::flush( )
    {
        if ( m_sink ) 
            { m_sink->flush( ); }
    }



    Output::Sink::~Sink( )
    {
        close( );
    }

}
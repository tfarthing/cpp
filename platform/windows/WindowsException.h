#pragma once

#include <cpp/Exception.h>
#include <cpp/Integer.h>
#include <cpp/Platform.h>
#include <cpp/util/Utf16.h>

namespace cpp
{

    namespace windows
    {

        class Exception
            : public cpp::Exception
        { 
        public:
            Exception( cpp::String msg = "" )
                : cpp::Exception( "" ), m_error( GetLastError( ) ), m_hresult( S_OK ) { m_what = msg + getErrorMessage( m_error ); }
            Exception( HRESULT hresult, cpp::String msg = "" )
                : cpp::Exception( "" ), m_error( 0 ), m_hresult( hresult ) { m_what = msg + getHresultMessage(hresult); }

            DWORD error( ) const
                { return m_error; }
            HRESULT hresult( ) const
                { return m_hresult; }

            static String getErrorMessage( DWORD error );
            static String getHresultMessage( HRESULT hresult );

        private:
            DWORD m_error;
            HRESULT m_hresult;
        };

        inline bool isSuccess(HRESULT hresult) 
            { return hresult >= S_OK; }

        inline void check( bool success )
            { if ( !success ) { throw Exception( ); } }

        inline void check( HRESULT hresult )
            { if ( !isSuccess( hresult ) ) { throw Exception( hresult ); } }

        inline void check( HRESULT hresult, cpp::String msg )
            { if ( !isSuccess( hresult ) ) { throw Exception( hresult, msg ); } }

        inline String Exception::getErrorMessage( DWORD error )
        {
            switch ( error )
            {
            case 12055:
                return "ERROR_WINHTTP_INVALID_URL - The URL is not valid.";
            default:
                break;
            }

            std::wstring text( 1024, L'\0' );
            text.resize( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                (LPTSTR)text.c_str( ), (DWORD)text.size( ), NULL ) );
            if ( text.size( ) == 0 )
                { return String::format( "error code: % (%)", (uint32_t)error, Integer::toHex(error, 8) ); }
            return toUtf8( text );
        }

        inline String Exception::getHresultMessage( HRESULT hresult )
        {
            if ( (hresult & 0xFFFF0000) == MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, 0 ) )
                { return getErrorMessage( HRESULT_CODE( hresult ) ); }

            if (hresult == S_OK)
                { return "ERROR_SUCCESS"; }

            // Not a Win32 HRESULT so return a generic error code.
            return String::format( "hresult(%)", Integer::toHex( hresult, 8 ) );
        }
    
    }

}
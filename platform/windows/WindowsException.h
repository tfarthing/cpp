#pragma once

#include "../../process/Exception.h"
#include "../../process/Platform.h"

namespace cpp
{

    namespace windows
    {

        class Exception
            : public cpp::Exception
        { 
        public:
			Exception( std::string msg = "" );
			Exception( HRESULT hresult, std::string msg = "" );

			DWORD error( ) const;
			HRESULT hresult( ) const;

            static std::string getErrorMessage( DWORD error );
            static std::string getHresultMessage( HRESULT hresult );

        private:
            DWORD m_error;
            HRESULT m_hresult;
        };


		inline Exception::Exception( std::string msg )
			: cpp::Exception( "" ), m_error( GetLastError( ) ), m_hresult( S_OK ) 
		{ 
			m_what = msg + getErrorMessage( m_error ); 
		}


		inline Exception::Exception( HRESULT hresult, std::string msg )
			: cpp::Exception( "" ), m_error( 0 ), m_hresult( hresult ) 
		{ 
			m_what = msg + getHresultMessage( hresult ); 
		}


		inline DWORD Exception::error( ) const
		{
			return m_error;
		}


		inline HRESULT Exception::hresult( ) const
		{
			return m_hresult;
		}



        inline bool isSuccess(HRESULT hresult) 
            { return hresult >= S_OK; }

        inline void check( bool success )
            { if ( !success ) { throw Exception( ); } }

        inline void check( HRESULT hresult )
            { if ( !isSuccess( hresult ) ) { throw Exception( hresult ); } }

        inline void check( HRESULT hresult, std::string msg )
            { if ( !isSuccess( hresult ) ) { throw Exception( hresult, msg ); } }
    
    }

}
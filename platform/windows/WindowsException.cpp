#include "../../data/Integer.h"
#include "../../text/Utf16.h"
#include "WindowsException.h"



namespace cpp
{

	namespace windows
	{

		std::string Exception::getErrorMessage( DWORD error )
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
			{
				return String::format( "error code: % (%)", (uint32_t)error, Integer::toHex( error, 8 ) );
			}
			return toUtf8( text );
		}

		std::string Exception::getHresultMessage( HRESULT hresult )
		{
			if ( ( hresult & 0xFFFF0000 ) == MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, 0 ) )
			{
				return getErrorMessage( HRESULT_CODE( hresult ) );
			}

			if ( hresult == S_OK )
			{
				return "ERROR_SUCCESS";
			}

			// Not a Win32 HRESULT so return a generic error code.
			return String::format( "hresult(%)", Integer::toHex( hresult, 8 ) );
		}

	}

}
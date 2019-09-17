#include "../../cpp/network/Url.h"
#include "../../cpp/process/Platform.h"
#include "../../cpp/platform/windows/WindowsException.h"
#include <Wininet.h>



namespace cpp
{

    Url::Url( const Memory & url )
    {
        char scheme[128];
        char hostname[1024];
        char username[1024];
        char password[1024];
        char path[1024];
        char extraInfo[1024];

        URL_COMPONENTSA urlComponents = { 0 };
        urlComponents.dwStructSize = sizeof( URL_COMPONENTS );
        urlComponents.lpszScheme = scheme;
        urlComponents.dwSchemeLength = sizeof( scheme );
        urlComponents.lpszUserName = username;
        urlComponents.dwUserNameLength = sizeof( username );
        urlComponents.lpszPassword = password;
        urlComponents.dwPasswordLength = sizeof( password );
        urlComponents.lpszHostName = hostname;
        urlComponents.dwHostNameLength = sizeof( hostname );
        urlComponents.lpszUrlPath = path;
        urlComponents.dwUrlPathLength = sizeof( path );
        urlComponents.lpszExtraInfo = extraInfo;
        urlComponents.dwExtraInfoLength = sizeof( extraInfo );

        windows::check( InternetCrackUrlA( url.data( ), (DWORD)url.length( ), ICU_DECODE | ICU_ESCAPE, &urlComponents ) != FALSE );

        this->scheme = scheme;
        this->username = username;
        this->password = password;
        this->port = urlComponents.nPort;
        this->host = hostname;
        this->path = path;

        Memory extra = extraInfo;
        auto parts = extra.split( "#", cpp::Memory::WhitespaceList, false );
        if ( parts.size( ) > 1 )
        {
            this->fragment = parts[1];
            extra = parts[0];
        }
        if ( extra[0] == '?' )
        {
            extra = extra.substr( 1 );
            parts = extra.split( "&;" );
            for ( auto & part : parts )
            {
                auto query = part.split( "=" );
                if ( query.size( ) > 1 )
                {
                    this->params[query[0]] = query[1];
                }
                else
                {
                    this->params[query[0]] = "";
                }
            }
        }
    }


    Url::Url( const Memory & scheme, const Memory & username, const Memory & password, const Memory & host, int port, const Memory & path, const MemoryMap & params, const Memory & fragment )
    {
        this->scheme = scheme;
        this->username = username;
        this->password = password;
        this->port = port;
        this->host = host;
        this->path = path;
        this->params = params;
        this->fragment = fragment;
    }



    String Url::toString( bool encoded ) const
    {
        String url = scheme + "//";
        
        if ( username.notEmpty( ) || password.notEmpty( ) )
        {
            url += username + ":" + password + "@";
        }

        url += format("%:%", host, port);

        url += path;

        if ( params.notEmpty( ) )
        {
            url += "?";
            for ( auto & itr : params.data )
            {
                if ( itr.second.notEmpty( ) )
                {
                    url += itr.first + "=" + itr.second;
                }
                else
                {
                    url += itr.first;
                }
                
            }
        }
        
        if ( fragment.notEmpty( ) )
        {
            url += "#" + fragment;
        }

        if ( encoded )
        {
            char buffer[1024];
            AtlCanonicalizeUrl( url.data( ), buffer, sizeof( buffer ), ATL_URL_ENCODE_PERCENT );
        }

        return url;
    }


    String Url::hostAndPort( ) const
    {

    
    }


    String Url::toQueryParams( const MemoryMap & params )
    {

    }


    String Url::encodeAuthority( const Memory & text )
    {
        String result;
        for ( int i = 0; i < text.length( ); i++ )
        {
            int c = (int)text[i];
            if ( isalnum( c ) || c == '-' || c == ' )
        }

    }


    String Url::decodeAuthority( const Memory & encodedText )
    {

    }


    String Url::encodePath( const Memory & text )
    {

    }


    String Url::decodePath( const Memory & encodedText )
    {

    }


    String Url::encodeQuery( const Memory & text )
    {

    }


    String Url::decodeQuery( const Memory & encodedText )
    {

    }


}
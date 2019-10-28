#if __has_include(<boost/network/uri.hpp>)

#include <boost/network/uri.hpp>

#include "../../cpp/data/Integer.h"
#include "../../cpp/network/Uri.h"

using namespace boost;

namespace cpp
{

	inline Memory toMemory( network::string_view & s )
		{ return Memory{ s.begin( ), s.end( ) }; }


	Uri::Uri( const Memory & url )
		: port( 0 )
    {
		network::uri uri{ url.data( ) };

		if ( uri.has_scheme( ) )
		{
			scheme = toMemory( uri.scheme( ) );
		}

		if ( uri.has_user_info( ) )
		{
			auto parts = toMemory( uri.user_info( ) ).split( ":" );
			username = parts[0];
			if ( parts.size( ) > 1 )
			{
				password = parts[1];
			}
		}

		if ( uri.has_port( ) )
		{
			port = (int)Integer::parseUnsigned( toMemory( uri.port( ) ) );
		}

		if ( uri.has_host( ) )
		{
			host = toMemory( uri.host( ) );
		}

		if ( uri.has_path( ) )
		{
			path = toMemory( uri.scheme( ) );
		}

		if ( uri.has_fragment( ) )
		{
			fragment = toMemory( uri.fragment( ) );
		}
    }


    Uri::Uri( const Memory & scheme, const Memory & username, const Memory & password, const Memory & host, int port, const Memory & path, const MemoryMap & params, const Memory & fragment )
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



    String Uri::toString( bool encoded ) const
    {	
		network::uri_builder uriBuilder;
		
		if ( scheme )
			{ uriBuilder.scheme( scheme.data ); }

		if ( username || password )
		{ 
			String userInfo = username + ":" + password;
			uriBuilder.user_info( encoded ? encodeUserInfo( userInfo ).data : userInfo.data );
		}

		if ( host )
		{ 
			uriBuilder.host( encoded ? encodeHost( host ).data : host.data );
		}

		if ( port != 0 )
		{ 
			uriBuilder.port( port ); 
		}

		if ( path )
		{ 
			uriBuilder.path( encoded ? encodePath( path ).data : path.data );
		}

		if ( params.notEmpty() )
		{ 
			for ( auto & itr : params.data )
			{ 
				uriBuilder.append_query_key_value_pair( 
					encoded ? encodePath( itr.first ).data : itr.first.data,
					encoded ? encodePath( itr.second ).data : itr.second.data );
			}
		}

		if ( fragment )
		{ 
			uriBuilder.fragment( encoded ? encodePath( fragment ).data : fragment.data );
		}

		return uriBuilder.uri( ).string( );
    }


    String Uri::hostAndPort( ) const
    {
		return ( port != 0 )
			? host + ":" + port 
			: host;
    }


	String Uri::encodeUserInfo( const Memory & value )
	{
		std::string enc;
		network::uri::encode_user_info( value.begin( ), value.end( ), std::back_inserter( enc ) );
		return enc;
	}


	String Uri::encodeHost( const Memory & value )
	{
		std::string enc;
		network::uri::encode_host( value.begin( ), value.end( ), std::back_inserter( enc ) );
		return enc;
	}


	String Uri::encodePath( const Memory & value )
	{
		std::string enc;
		network::uri::encode_path( value.begin( ), value.end( ), std::back_inserter( enc ) );
		return enc;
	}


	String Uri::encodeQuery( const Memory & value )
	{
		std::string enc;
		network::uri::encode_query( value.begin( ), value.end( ), std::back_inserter( enc ) );
		return enc;
	}


	String Uri::encodeFragment( const Memory & value )
	{
		std::string enc;
		network::uri::encode_fragment( value.begin( ), value.end( ), std::back_inserter( enc ) );
		return enc;
	}

}

#endif

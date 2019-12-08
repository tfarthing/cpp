#include "../../cpp/data/Integer.h"
#include "../../cpp/network/Uri.h"



namespace cpp
{


	Uri::Uri( const Memory & uri )
		: port( 0 )
    {
		size_t schemeEnd = uri.findFirstOf( ":" );
		size_t authorityBegin = Memory::npos;
		size_t authorityEnd = Memory::npos;
		size_t userInfoBegin = Memory::npos;	// relative to authorityBegin
		size_t userInfoEnd = Memory::npos;		// relative to authorityBegin
		size_t userInfoColon = Memory::npos;	// relative to userInfoBegin
		size_t hostBegin = Memory::npos;		// relative to authorityBegin
		size_t hostEnd = Memory::npos;			// relative to authorityBegin
		size_t portBegin = Memory::npos;		// relative to authorityBegin
		size_t portEnd = Memory::npos;			// relative to authorityBegin
		size_t pathBegin = Memory::npos;
		size_t pathEnd = Memory::npos;
		size_t queryBegin = Memory::npos;
		size_t queryEnd = Memory::npos;
		size_t fragmentBegin = Memory::npos;
		size_t fragmentEnd = Memory::npos;

		// check for authority
		if ( uri.substr( schemeEnd + 1, 2 ) == "//" )
		{
			authorityBegin = schemeEnd + 3;
			authorityEnd = uri.findFirstOf( "/?#", authorityBegin );
			if ( authorityEnd == Memory::npos ) 
				{ authorityEnd = uri.length( ); }

			Memory authority = uri.substr( authorityBegin, authorityEnd - authorityBegin );

			userInfoEnd = authority.findFirstOf( "@" );
			if ( userInfoEnd != Memory::npos )
			{ 
				userInfoBegin = 0; 
				userInfoColon = authority.substr(0, userInfoEnd).findFirstOf( ":" );
			}

			hostBegin = ( userInfoBegin != Memory::npos )
				? userInfoEnd + 1 
				: 0;
			if ( authority[hostBegin] == '[' )
			{
				hostEnd = authority.findFirstOf( "]", hostBegin );
				if (hostEnd == Memory::npos )
					{ hostEnd = authority.length( ); }
				else
					{ hostEnd -= 1; }
				hostBegin += 1;
			}
			
			portBegin = authority.findLastOf( ":" );
			if ( portBegin != Memory::npos && hostEnd != Memory::npos && portBegin < hostEnd )
				{ portBegin = Memory::npos; }

			if ( portBegin != Memory::npos )
			{
				if ( hostEnd == Memory::npos )
					{ hostEnd = portBegin; }
				portBegin++;
				portEnd = authority.length( );
			}
			else if( hostEnd == Memory::npos )
			{
				hostEnd = authority.length( );
			}
		}

		//	path
		pathBegin = ( authorityEnd != Memory::npos )
			? authorityEnd
			: schemeEnd + 1;
		pathEnd = uri.findFirstOf( "?#", pathBegin );
		if ( pathEnd == Memory::npos )
			{ pathEnd = uri.length( ); }

		//	query
		if ( uri.at(pathEnd) == '?' )
		{
			queryBegin = pathEnd + 1;
			queryEnd = uri.findFirstOf( "#", queryBegin );
			if ( queryEnd == Memory::npos )
			{
				queryEnd = uri.length( );
			}
		}

		//	fragment
		if ( uri.at( pathEnd ) == '#' )
		{
			fragmentBegin = pathEnd + 1;
		}
		else if ( queryEnd != Memory::npos && uri.at( pathEnd ) == '#' )
		{
			fragmentBegin = queryEnd + 1;
		}
		if ( fragmentBegin != Memory::npos )
		{
			fragmentEnd = uri.length( );
		}

		//	decoding & value assignments...

		//	scheme
		if ( schemeEnd != Memory::npos )
			{ this->scheme = uri.substr(0, schemeEnd); }

		//	authority
		if ( authorityBegin != Memory::npos )
		{
			if ( userInfoBegin != Memory::npos )
			{
				Memory userInfo = uri.substr( authorityBegin + userInfoBegin, userInfoEnd );
				//	username
				this->username = decode( userInfo.substr( 0, userInfoColon ) );
				//	password
				if ( userInfoColon != Memory::npos )
					{ this->password = decode( userInfo.substr( userInfoColon + 1 ) ); }
			}

			//	host
			this->host = decode( uri.substr( authorityBegin + hostBegin, hostEnd - hostBegin ) );

			//	port
			if ( portBegin != Memory::npos )
			{
				this->port = uri.substr( authorityBegin + portBegin, portEnd - portBegin ).asDecimal( );
			}
		}

		//	path
		this->path = decode( uri.substr( pathBegin, pathEnd - pathBegin ) );

		//	query
		if ( queryBegin != Memory::npos )
		{
			Memory query = uri.substr( queryBegin, queryEnd - queryBegin );
			for ( auto & kv : query.split( "&;" ) )
			{
				auto parts = kv.split( "=" );
				String key = decode( parts[0] );
				if ( parts.size( ) == 1 )
				{
					this->params[key] = "";
				}
				else
				{
					this->params[key] = decode(parts[1]);
				}
			}
		}

		//	fragment
		if ( fragmentBegin != Memory::npos )
		{
			this->fragment = decode( uri.substr( fragmentBegin, fragmentEnd - fragmentBegin ) );
		}
	}


    Uri::Uri( const Memory & scheme, const Memory & username, const Memory & password, const Memory & host, int port, const Memory & path, const MemoryMap & params, const Memory & fragment )
	{
        this->scheme = scheme;
        this->username = username;
        this->password = password;
        this->host = host;
		this->port = port;
		this->path = path;
        this->params = params;
        this->fragment = fragment;
    }



    String Uri::toString( bool encoded ) const
    {	
		String uri;
		
		if ( scheme.notEmpty( ) )
			{ uri += scheme + ":"; }

		if ( host.notEmpty( ) )
		{
			uri += "//";

			if ( username.notEmpty( ) )
			{
				uri += encoded 
					? encodeUserInfo( username ) 
					: username;
				if ( password.notEmpty( ) )
				{
					uri += ":" + encoded
						? encodeUserInfo( password )
						: password;
				}
				uri += "@";
			}

			uri += hostAndPort( encoded );
		}

		uri += pathAndExtra( encoded );

		return uri;
    }


    String Uri::hostAndPort( bool encoded ) const
    {
		String result;

		bool isIpv6 = host.find( ':' ) != String::npos;
		if ( isIpv6 )
			{ result += "["; }

		result += encoded
			? encodeHost( host )
			: host;

		if ( isIpv6 )
			{ result += "]"; }

		if ( port != 0 )
		{
			result += ":" + Integer::toDecimal( port );
		}

		return result;
    }


	String Uri::pathAndExtra( bool encoded ) const
	{
		String result;
		if ( path.notEmpty( ) )
		{
			result += encoded
				? encodePath( path )
				: path;
		}

		if ( params.notEmpty( ) )
		{
			result += "?";
			bool isInitial = true;
			for ( auto & itr : params.data )
			{
				if ( isInitial )
					{ isInitial = false; } 
				else
					{ result += "&"; }
				
				result += encoded
					? encodeQuery( itr.first )
					: itr.first;
				
				if ( itr.second.notEmpty( ) )
				{
					result += "=" + encoded ?
						encodeQuery( itr.second )
						: itr.second;
				}
			}
		}

		if ( fragment.notEmpty( ) )
		{
			result += "#" + encodeFragment( fragment );
		}

		return result;
	}


	String Uri::decode( const Memory & value )
	{
		String result;

		size_t index = 0;
		size_t escape = 0;
		while ( ( escape = value.find( '%', index ) ) != Memory::npos )
		{
			result += value.substr( index, escape - index );

			uint8_t ch = value.substr( escape + 1, 2 ).asHex( );
			result += (char)ch;

			index = escape + 3;
		}
		result += value.substr( index );

		return result;
	}


	String percentEncode( const Memory & value, const char * permitted )
	{
		std::string enc;

		size_t index = 0;
		while ( index < value.length( ) )
		{
			char ch = value[index++];
			if ( isalnum( ch ) || strchr( permitted, ch ) )
			{
				enc += ch;
			}
			else
			{
				enc += "%" + Integer::toHex( ch, 2, true );
			}
		}

		return enc;
	}

	String Uri::encodeUserInfo( const Memory & value )
	{
		const char * permitted = "-._~!$&'()*+,;=";
		return percentEncode( value, permitted );
	}


	String Uri::encodeHost( const Memory & value )
	{
		const char * permitted = "-._~!$&'()*+,;=";
		return percentEncode( value, permitted );
	}


	String Uri::encodePath( const Memory & value )
	{
		const char * permitted = "-._~!$&'()*+,;=:@";
		return percentEncode( value, permitted );
	}


	String Uri::encodeQuery( const Memory & value )
	{
		const char * permitted = "-._~!$&'()*+,;=:@?/";
		return percentEncode( value, permitted );
	}


	String Uri::encodeFragment( const Memory & value )
	{
		const char * permitted = "-._~!$&'()*+,;=:@?/";
		return percentEncode( value, permitted );
	}

}


#ifndef TEST

#include "../../cpp/data/Integer.h"
#include "../../cpp/network/Uri.h"



namespace cpp
{


	Uri::Uri( const Memory & uri )
		: port( 0 )
    {
		assign( uri );
	}


    Uri::Uri( const Memory & scheme, const Memory & username, const Memory & host, int port, const Memory & path, const Memory & query, const Memory & fragment )
	{
        this->scheme = scheme;
        this->username = username;
        this->host = host;
		this->port = port;
		this->path = path;
        this->query = query;
        this->fragment = fragment;
    }


	Uri & Uri::operator=( const Memory & uri )
	{
		assign( uri );
		return *this;
	}


	Uri & Uri::assign( const Memory & uri )
	{
		size_t schemeEnd = uri.findFirstOf( ":" );
		size_t authorityBegin = Memory::npos;
		size_t authorityEnd = Memory::npos;
		size_t userInfoBegin = Memory::npos;	// relative to authorityBegin
		size_t userInfoEnd = Memory::npos;		// relative to authorityBegin
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
			{
				authorityEnd = uri.length( );
			}

			Memory authority = uri.substr( authorityBegin, authorityEnd - authorityBegin );

			userInfoEnd = authority.findFirstOf( "@" );
			if ( userInfoEnd != Memory::npos )
			{
				userInfoBegin = 0;
			}

			hostBegin = ( userInfoBegin != Memory::npos )
				? userInfoEnd + 1
				: 0;
			if ( authority[hostBegin] == '[' )
			{
				hostEnd = authority.findFirstOf( "]", hostBegin );
				if ( hostEnd == Memory::npos )
				{
					hostEnd = authority.length( );
				}
				hostBegin += 1;
			}

			portBegin = authority.findLastOf( ":" );
			if ( portBegin != Memory::npos && hostEnd != Memory::npos && portBegin < hostEnd )
			{
				portBegin = Memory::npos;
			}

			if ( portBegin != Memory::npos )
			{
				if ( hostEnd == Memory::npos )
				{
					hostEnd = portBegin;
				}
				portBegin++;
				portEnd = authority.length( );
			}
			else if ( hostEnd == Memory::npos )
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
		{
			pathEnd = uri.length( );
		}

		//	query
		if ( uri.at( pathEnd ) == '?' )
		{
			queryBegin = pathEnd + 1;
			queryEnd = uri.findFirstOf( "#", queryBegin );
			if ( queryEnd == Memory::npos )
			{
				queryEnd = uri.length( );
			}
		}
		
		//	fragment
		if ( queryEnd != Memory::npos && uri.at( queryEnd ) == '#' )
		{
			fragmentBegin = queryEnd + 1;
		}
		else if ( pathEnd != Memory::npos && uri.at( pathEnd ) == '#' )
		{
			fragmentBegin = pathEnd + 1;
		}
		if ( fragmentBegin != Memory::npos )
		{
			fragmentEnd = uri.length( );
		}

		//	decoding & value assignments...

		//	scheme
		scheme.clear( );
		if ( schemeEnd != Memory::npos )
			{ scheme = uri.substr( 0, schemeEnd ); }

		//	authority
		username.clear( );
		host.clear( );
		port = 0;
		if ( authorityBegin != Memory::npos )
		{
			//	username
			if ( userInfoBegin != Memory::npos )
				{ username = decode( uri.substr( authorityBegin + userInfoBegin, userInfoEnd ) ); }

			//	host
			host = decode( uri.substr( authorityBegin + hostBegin, hostEnd - hostBegin ) );

			//	port
			if ( portBegin != Memory::npos )
				{ port = uri.substr( authorityBegin + portBegin, portEnd - portBegin ).asDecimal( ); }
		}

		//	path
		path = decode( uri.substr( pathBegin, pathEnd - pathBegin ) );

		//	query
		query.clear( );
		if ( queryBegin != Memory::npos )
			{ query = decode( uri.substr( queryBegin, queryEnd - queryBegin ) ); }

		//	fragment
		fragment.clear( );
		if ( fragmentBegin != Memory::npos )
			{ fragment = decode( uri.substr( fragmentBegin, fragmentEnd - fragmentBegin ) ); }

		return *this;
	}


    String Uri::toString( bool encoded ) const
    {	
		String uri;
		
		if ( scheme.notEmpty( ) )
			{ uri += scheme + ":"; }

		if ( host.notEmpty( ) )
		{
			uri += "//" + authority( encoded );
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


	String Uri::authority( bool encoded ) const
	{
		String result;

		if ( username.notEmpty( ) )
		{
			result += encoded
				? encodeUserInfo( username )
				: username;
			result += "@";
		}

		result += hostAndPort( encoded );

		return result;
	}


	String Uri::pathAndExtra( bool isEncoded ) const
	{
		String result;
		if ( path.notEmpty( ) )
		{
			result += isEncoded
				? encodePath( path )
				: path;
		}

		if ( query.notEmpty( ) )
		{
			result += "?";
			result += isEncoded
				? encodeQuery( query )
				: query;
		}

		if ( fragment.notEmpty( ) )
		{
			result += "#";
			result += isEncoded
				? encodeFragment( fragment )
				: fragment;
		}

		return result;
	}


	MemoryMap Uri::params( ) const
	{
		MemoryMap result;

		for ( auto & kv : query.split( "&;" ) )
		{
			auto parts = kv.split( "=" );
			String key = parts[0];
			if ( parts.size( ) == 1 )
			{
				result[key] = "";
			}
			else
			{
				result[key] = parts[1];
			}
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
		const char * permitted = "-._~!$&'()*+,;=:@/";
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

	
	String Uri::toQuery( const MemoryMap & params )
	{
		String result;

		bool isInitial = true;
		for ( auto & itr : params.data )
		{
			if ( isInitial )
				{ isInitial = false; }
			else
				{ result += "&"; }

			result += itr.first;

			if ( itr.second.notEmpty( ) )
			{
				result += "=" + itr.second;
			}
		}

		return result;
	}


}

#else

#include <cpp/meta/Test.h>
#include <cpp/network/Uri.h>

TEST_CASE( "Uri" )
{
	using namespace cpp;

	Uri uri = "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top";
	CHECK( uri.scheme == "https" );
	CHECK( uri.username == "john.doe" );
	CHECK( uri.host == "www.example.com" );
	CHECK( uri.port == 123 );
	CHECK( uri.path == "/forum/questions/" );
	CHECK( uri.query == "tag=networking&order=newest" );
	CHECK( uri.fragment == "top" );
	CHECK( uri.authority( ) == "john.doe@www.example.com:123" );
	CHECK( uri.hostAndPort( ) == "www.example.com:123" );
	CHECK( uri.pathAndExtra( ) == "/forum/questions/?tag=networking&order=newest#top" );
	

	uri = "ldap://[2001:db8::7]/c=GB?objectClass?one";
	CHECK( uri.scheme == "ldap" );
	CHECK( uri.host == "2001:db8::7" );
	CHECK( uri.authority( ) == "[2001:db8::7]" );
	CHECK( uri.path == "/c=GB" );
	CHECK( uri.query == "objectClass?one" );


	uri = "mailto:John.Doe@example.com";
	CHECK( uri.path == "John.Doe@example.com" );


	uri = "news:comp.infosystems.www.servers.unix";
	CHECK( uri.path == "comp.infosystems.www.servers.unix" );


	uri = "tel:+1-816-555-1212";
	CHECK( uri.path == "+1-816-555-1212" );
	CHECK( uri.toString() == "tel:+1-816-555-1212" );


	uri = "telnet://192.0.2.16:80/";
	CHECK( uri.host == "192.0.2.16" );
	CHECK( uri.path == "/" );


	uri = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";
	CHECK( uri.path == "oasis:names:specification:docbook:dtd:xml:4.1.2" );

	uri = "https://www.example.com:123/?key=some%20value";
	CHECK( uri.query == "key=some value" );
	CHECK( uri.toString() == "https://www.example.com:123/?key=some%20value" );


	// copy
	Uri uri2{ uri };
	// copy
	Uri uri3 = uri;
	// assign
	uri2 = uri3;

	// move assign
	CHECK( uri2.scheme.notEmpty( ) );
	uri3 = std::move( uri2 );
	CHECK( uri2.scheme.isEmpty( ) );
}


#endif

#pragma once

/*

	Uri is an encapsulation of URI parsing & encoding.
		(1) Fields are stored unencoded

*/


#include "../../cpp/data/String.h"
#include "../../cpp/data/DataMap.h"


namespace cpp
{

    struct Uri
    {
                                            Uri( const Memory & uri );
                                            Uri( const Memory & scheme, const Memory & username, const Memory & host, int port, const Memory & path, const Memory & query, const Memory & fragment );
											
		Uri &								operator=( const Memory & uri );

		Uri &								assign( const Memory & uri );

        String                              toString( bool encoded = true ) const;
		String								authority( bool encoded = false ) const;
        String                              hostAndPort( bool encoded = false ) const;
		String								pathAndExtra( bool encoded = false ) const;
		MemoryMap							params( ) const;

		static String						decode( const Memory & value );

		static String						encodeUserInfo( const Memory & value );
		static String						encodeHost( const Memory & value );
		static String						encodePath( const Memory & value );
		static String						encodeQuery( const Memory & value );
		static String						encodeFragment( const Memory & value );

		static String						toQuery( const MemoryMap & params );

        String                              scheme;
        String                              username;
        String                              host;
        int                                 port;
        String                              path;
        String								query;
        String                              fragment;
    };

}

#pragma once


#include "../../cpp/data/String.h"
#include "../../cpp/data/DataMap.h"


namespace cpp
{

    struct Uri
    {
                                            Uri( const Memory & url );
                                            Uri( const Memory & scheme, const Memory & username, const Memory & password, const Memory & host, int port, const Memory & path, const MemoryMap & params, const Memory & fragment );

        String                              toString( bool encoded = true ) const;
        String                              hostAndPort( bool encoded = false ) const;
		String								pathAndExtra( bool encoded = false ) const;

		static String						decode( const Memory & value );

		static String						encodeUserInfo( const Memory & value );
		static String						encodeHost( const Memory & value );
		static String						encodePath( const Memory & value );
		static String						encodeQuery( const Memory & value );
		static String						encodeFragment( const Memory & value );

        String                              scheme;
        String                              username;
        String                              password;
        String                              host;
        int                                 port;
        String                              path;
        StringMap                           params;
        String                              fragment;
    };

}

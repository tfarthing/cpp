#pragma once

#include "../../cpp/data/String.h"
#include "../../cpp/data/DataMap.h"


namespace cpp
{

    struct Url
    {
                                            Url( const Memory & url );
                                            Url( const Memory & scheme, const Memory & username, const Memory & password, const Memory & host, int port, const Memory & path, const MemoryMap & params, const Memory & fragment );

        String                              toString( bool encoded = true ) const;
        String                              hostAndPort( ) const;

        static String                       toQueryParams( const MemoryMap & params );

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


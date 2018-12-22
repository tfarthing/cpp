#pragma once

#include <cpp/String.h>

namespace cpp
{

    namespace SHA2
    {

        String hash256( const Memory & src );
        String hash512( const Memory & src );

    }

}
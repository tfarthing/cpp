#pragma once

#include "../data/String.h"

namespace cpp
{

    namespace AES
    {

        String encrypt256( const Memory & plainText, const Memory & cipherKey /* 32B */, const Memory & initVector /* 16B */ );
        String decrypt256( const Memory & cipherText, const Memory & cipherKey /* 32B */, const Memory & initVector /* 16B */ );

    }

}
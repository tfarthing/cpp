#pragma once

#include <cpp/io/Input.h>
#include <cpp/io/reader/ByteReader.h>
#include <cpp/io/reader/LineReader.h>

namespace cpp
{

    class StandardInput
    {
    public:
        static Input & input( size_t buflen = 64 * 1024 );

        static ByteReader bytes( size_t buflen = 64 * 1024 )
            { return ByteReader{ input( ), buflen }; }
        
        static LineReader lines( size_t buflen = 64 * 1024 )
            { return LineReader{ input( ), buflen }; }

    };

}


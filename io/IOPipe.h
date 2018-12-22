#pragma once

#include <cpp/io/Input.h>
#include <cpp/io/Output.h>

namespace cpp
{ 

    class IOPipe
    {
    public:
        static IOPipe createChildOutput( size_t buflen = 64 * 1024 );
        static IOPipe createChildInput( size_t buflen = 64 * 1024 );
        static IOPipe createPipe( size_t buflen = 64 * 1024, bool startInputThread = true );

        IOPipe( );
        void startInput( );

        Input & input( );
        Output & output( );

        void * getInputHandle( );
        void * getOutputHandle( );

    private:
        struct Detail;
        IOPipe( std::shared_ptr<Detail> && detail );
        IOPipe( size_t buflen, bool startInputThread );

        std::shared_ptr<Detail> m_detail;
    };

}
#pragma once

#include <algorithm>
#include <cpp/process/Lock.h>
#include <cpp/io/Input.h>
#include <cpp/io/Output.h>
#include <cpp/util/Handle.h>
#include <cpp/util/DataBuffer.h>

namespace cpp
{

    class IOBuffer
    {
    public:
        IOBuffer( size_t buflen = 64 * 1024 )
            : m_buffer(buflen), m_isOpen(true), m_input(*this), m_output(*this) { }

        Input & input( )
            { return m_input; }

        Output & output( )
            { return m_output; }

    private:
        class BufferedInput
            : public Input
        {
        public:
            BufferedInput( IOBuffer & io )
                : Input( Handle<Source>{ io } ) { }

        private:
            friend class IOBuffer;
            class Source
                : public Input::Source_t
            {
            public:
                Source( IOBuffer & io )
                    : m_io( io ) { }

                bool isOpen( ) const override;

                Memory read( const Memory & dst, Duration timeout ) override;

                void close( ) override;

            private:
                IOBuffer & m_io;
            };
        };
        friend class BufferedInput::Source;

    private:
        class BufferedOutput
            : public Output
        {
        public:
            BufferedOutput( IOBuffer & io )
                : Output( Handle<Sink>{ io } ) { }

        private:
            friend class IOBuffer;
            class Sink
                : public Output::Sink_t
            {
            public:
                Sink( IOBuffer & io )
                    : m_io( io )
                {
                }

                bool isOpen( ) const override;

                Memory write( const Memory & src ) override;

                void flush( ) override;

                void close( ) override;

            private:
                IOBuffer & m_io;
            };
        };
        friend class BufferedOutput::Sink;

    private:
        Mutex m_mutex;
        DataBuffer m_buffer;
        bool m_isOpen;
        BufferedInput m_input;
        BufferedOutput m_output;
    };

}

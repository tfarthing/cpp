#pragma once

#include <algorithm>
#include <cpp/io/Output.h>
#include <cpp/util/Handle.h>

namespace cpp
{

    class StringOutput
        : public Output
    {
    public:
        StringOutput( size_t reserveBytes = 64 )
            : Output( Handle<Sink>{ reserveBytes } ) { }

        Memory data() const
            { return std::dynamic_pointer_cast<Sink>( m_sink )->data(); }
        
    private:
        class Sink
            : public Output::Sink_t
        {
        public:
            Sink( size_t reserveBytes )
                : m_isOpen(true)
            {
                m_buffer.reserve( reserveBytes );
            }

            Memory data() const
            {
                return m_buffer;
            }

            bool isOpen( ) const override
            {
                return m_isOpen;
            }
            
            Memory write( const Memory & src ) override
            {
                m_buffer += src;
                return src;
            }

            void flush( ) override
            {
            }

            void close( ) override
            {
                m_isOpen = false;
            }

        private:
            bool m_isOpen;
            String m_buffer;
        };
    };

}


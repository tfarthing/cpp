#pragma once

#include <algorithm>
#include <cpp/io/Input.h>
#include <cpp/util/Handle.h>

namespace cpp
{

    class MemoryInput
        : public Input
    {
    public:
        MemoryInput( Memory input )
            : Input( std::move( Handle<Source>{ input } ) ) { }
        
    private:
        class Source
            : public Input::Source_t
        {
        public:
            Source( Memory input )
                : m_memory( input ), m_offset(0)
            {
            }

            bool isOpen( ) const override
            {
                return m_offset < m_memory.length( );
            }

            Memory read( const Memory & dst, Duration /*timeout*/ ) override
            {
                size_t len = std::min( dst.length(), m_memory.length() - m_offset );
                Memory result = Memory::copy( dst, m_memory.substr( m_offset, len ) );
                m_offset += len;
                return result;
            }

            void close( ) override
            {
                m_offset = m_memory.length( );
            }

        private:
            Memory m_memory;
            size_t m_offset;
        };
    };

}


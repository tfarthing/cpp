#pragma once

#include <algorithm>
#include <cpp/io/Input.h>
#include <cpp/util/Handle.h>
#include <cpp/util/DataBuffer.h>

namespace cpp
{

    class StringInput
        : public Input
    {
    public:
        StringInput( String input )
            : Input( Handle<Source>{ std::move(input) } ) { }
        
    private:
        class Source
            : public Input::Source_t
        {
        public:
            Source( String input )
                : m_buffer( std::move( input ) )
            {
            }

            bool isOpen( ) const override
            {
                return m_buffer.getable().isEmpty() == false;
            }

            Memory read( const Memory & dst, Duration /*timeout*/ ) override
            {
                size_t len = std::min( dst.length(), m_buffer.getable().length() );
                return Memory::copy( dst, m_buffer.get( len ) );
            }

            void close( ) override
            {
                m_buffer.getAll();
            }

        private:
            DataBuffer m_buffer;
        };
    };

}


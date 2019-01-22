#pragma once

#include "Unicode.h"
#include "../process/Exception.h"
#include "../data/String.h"
#include "../data/DataBuffer.h"
#include "../io/Input.h"

namespace cpp
{

    namespace Utf8
    {
		static const uint32_t BadCodePoint = (uint32_t)-1;

        class Text
        {
        public:
            static Text fromUtf16( const wchar_t * utf16 );
            static Text fromUtf16( const std::wstring & utf16 );

            Text( );
            Text( unicode_t ch );
            Text( const char * str );
            Text( const char * begin, const char * end );
            explicit Text( const Memory & str );
            Text( String && move );
            Text( const String & copy );
            Text( Text && move );
            Text( const Text & copy );

            Text & assign( const char * str );
            Text & assign( const char * begin, const char * end );
            Text & assign( const Memory & str );
            Text & assign( String && move );
            Text & assign( const String & copy );
            Text & assign( Text && move );
            Text & assign( const Text & copy );

            Text & operator=( const Memory & memory );
            Text & operator=( const char * str );
            Text & operator=( String && move );
            Text & operator=( const String & copy );
            Text & operator=( Text && move );
            Text & operator=( const Text & copy );

            Text & append( const Memory & memory );
            Text & append( unicode_t ch );
            Text & operator+=( const Memory & memory );
            Text & operator+=( unicode_t ch );

            void push_back( unicode_t ch );

            void clear( );
            bool isEmpty( ) const;

            operator const char *( ) const;
            const char * begin( ) const;
            const char * end( ) const;
            size_t size( ) const;
            Memory data( ) const;

            // length in Unicode characters
            size_t length( ) const; 
            // Unicode character at character pos ( from 0 up to length() )
            unicode_t at( size_t pos ) const;
            // Unicode character at character pos ( from 0 up to length() )
            unicode_t operator[]( size_t pos ) const;
            
            //
            uint32_t charOffset( uint32_t pos ) const;
            //
            uint32_t ptrToPos( const char * ptr );

            Memory substr( size_t pos, size_t len ) const;

        private:
            void scan( size_t pos ) const;

        private:
            String m_str;
            mutable std::vector<uint32_t> m_offsets;
        };
        

        unicode_t decode( const Memory & utf8, size_t offset = 0, int * encodedLength = nullptr );
        Memory encode( unicode_t ch, Memory & dst );

        Memory substr( const Memory & utf8, size_t pos, size_t offset = -1 );
        size_t strlen( const Memory & utf8 );

        //Unicode toUnicode( const Memory & utf8 );

		/*
        class Reader
        {
        public:
            class iterator
            {
			public:
                iterator( Reader * reader );

                unicode_t operator*( );
                bool operator!=( iterator & iter ) const;
                iterator & operator++( );

			private:
                void get( );

			private:
                Reader * m_reader;
                unicode_t m_value;
            };
        public:
			Reader( const char * data );
            Reader( const Utf8::Text & data );
            Reader( const Memory & data );
            Reader( Input input );

            bool isOpen( ) const;
            iterator begin( );
            iterator end( );

		private:
            void tryRead( );
            bool tryDecode( unicode_t & result );
            unicode_t get( );

        private:
            Input m_input;
            StringBuffer m_buffer;
        };
		*/
    };

}

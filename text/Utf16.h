#pragma once

#include <cpp/Object.h>
#include <cpp/Exception.h>
#include <cpp/String.h>
#include <cpp/io/Input.h>

namespace cpp
{
    namespace Utf8 { class Text; }

    std::wstring toUtf16( const cpp::Memory & src );
    String toUtf8( const wchar_t * begin, const wchar_t * end );
    String toUtf8( const std::wstring & utf16 );

	namespace Utf16
	{
		static const uint32_t BadCodePoint = (uint32_t)-1;

        typedef std::pair<const wchar_t *, const wchar_t *> Span;
        typedef std::vector<Span> Match;
        typedef std::vector<Match> Matches;

        Match match( const wchar_t * begin, const wchar_t * end, const wchar_t * regex );
        Match searchOne( const wchar_t * begin, const wchar_t * end, const wchar_t * regex );
        Matches searchAll( const wchar_t * begin, const wchar_t * end, const wchar_t * regex );

        class Text
        {
        public:
            static Text fromUtf8( const cpp::Memory & utf8 );

            Text( );
            Text( unicode_t ch );
            Text( const wchar_t * str );
            Text( const wchar_t * begin, const wchar_t * end );
            explicit Text( const Memory & str );
            Text( std::wstring && move );
            Text( const std::wstring & copy );
            Text( Text && move );
            Text( const Text & copy );

            Text & assign( const wchar_t * str );
            Text & assign( const wchar_t * begin, const wchar_t * end );
            Text & assign( const Memory & str );
            Text & assign( std::wstring && move );
            Text & assign( const std::wstring & copy );
            Text & assign( Text && move );
            Text & assign( const Text & copy );

            Text & operator=( const Memory & memory );
            Text & operator=( const wchar_t * str );
            Text & operator=( std::wstring && move );
            Text & operator=( const std::wstring & copy );
            Text & operator=( Text && move );
            Text & operator=( const Text & copy );

            Text & append( const Memory & memory );
            Text & append( unicode_t ch );
            Text & operator+=( const Memory & memory );
            Text & operator+=( unicode_t ch );

            void push_back( unicode_t ch );

            void clear( );
            bool isEmpty( ) const;

            operator const wchar_t *( ) const;
            const wchar_t * begin( ) const;
            const wchar_t * end( ) const;
            const wchar_t * dataAt( size_t pos ) const;
            size_t size( ) const;
            Memory data( ) const;

            // length in Unicode characters
            size_t length( ) const; 
            // Unicode character at character pos ( from 0 up to length() )
            unicode_t at( size_t pos ) const;
            // Unicode character at character pos ( from 0 up to length() )
            unicode_t operator[]( size_t pos ) const;

            Text substr( size_t pos, size_t len = -1 ) const;

            Match match( const wchar_t * regex ) const;
            Match searchOne( const wchar_t * regex ) const;
            Matches searchAll( const wchar_t * regex ) const;

        private:
            void scan( size_t pos ) const;
            Text( std::wstring str, std::vector<uint32_t> offsets );

        private:
            std::wstring m_str;
            mutable std::vector<uint32_t> m_offsets;
        };
        
		unicode_t decode(const wchar_t * begin, const wchar_t * end, size_t offset = 0, int * encodedLength = nullptr);
		Memory encode( unicode_t ch, wchar_t * dstBegin, const wchar_t * dstEnd );

		class Reader
			: public Object
		{
		public:
			class iterator
			{
			public:
                iterator( Reader * reader );
                
                unicode_t operator*( ) const;
                bool operator!=( iterator & iter ) const;
                iterator & operator++( );

			private:
                void get( );

			private:
				Reader * m_reader;
                unicode_t m_value;
			};
		public:
            Reader( const wchar_t * data );
            Reader( const Utf16::Text & data );
            explicit Reader( const Memory & data );
            Reader( Input && input );

            bool isOpen( );
            iterator begin( );
            iterator end( );

		private:
            void tryRead( );
            bool tryDecode( unicode_t & result );
            unicode_t get( );

		private:
			Input m_input;
            wchar_t m_buffer[2];
            size_t m_bufferOffset;
		};
	};

}

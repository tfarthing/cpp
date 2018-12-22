#ifndef TEST

#include <cpp/Exception.h>
#include <cpp/io/StringInput.h>
#include <cpp/io/MemoryInput.h>
#include <cpp/util/Utf8.h>
#include <cpp/util/Utf16.h>

namespace cpp
{

	namespace Utf8
	{

        Text Text::fromUtf16( const wchar_t * utf16 )
            { return Utf8::Text{ toUtf8( utf16 ) }; }

        Text Text::fromUtf16( const std::wstring & utf16 )
            { return Utf8::Text{ toUtf8( utf16 ) }; }

        Text::Text( )
            : m_str( ), m_offsets( ) { }

        Text::Text( unicode_t ch )
            : m_str( ), m_offsets( ) { append(ch); }

        Text::Text( const Memory & str )
            : m_str( ), m_offsets( ) { assign(str); }

        Text::Text( const char * str )
            : m_str( str ), m_offsets( ) { }

        Text::Text( String && move )
            : m_str( std::move( move ) ), m_offsets( ) { }

        Text::Text( const String & copy )
            : m_str( copy ), m_offsets( ) { }

        Text::Text( Text && move )
            : m_str( std::move( move.m_str ) ), m_offsets( std::move( move.m_offsets ) ) { }

        Text::Text( const Text & copy )
            : m_str( copy.m_str ), m_offsets( copy.m_offsets ) { }

        Text & Text::assign( const char * str )
        {
            m_offsets.clear( );
            m_str.assign( str );
            return *this;
        }

        Text & Text::assign( const char * begin, const char * end )
        {
            m_offsets.clear( );
            m_str.assign( begin, end );
            return *this;
        }

        Text & Text::assign( const Memory & str )
        {
            clear( );
            if ( str )
            { 
                m_str.assign( str.begin( ), str.end( ) ); 
            }
            return *this;
        }

        Text & Text::assign( String && move )
        {
            m_offsets.clear( );
            m_str.assign( std::move( move ) );
            return *this;
        }

        Text & Text::assign( const String & copy )
        {
            m_offsets.clear( );
            m_str.assign( copy );
            return *this;
        }

        Text & Text::assign( Text && move )
        {
            m_offsets = std::move( move.m_offsets );
            m_str.assign( std::move( move.m_str ) );
            return *this;
        }

        Text & Text::assign( const Text & copy )
        {
            m_offsets = copy.m_offsets;
            m_str = copy.m_str;
            return *this;
        }

        Text & Text::operator=( const Memory & memory )
            { return assign( memory ); }

        Text & Text::operator=( const char * str )
            { return assign( str ); }

        Text & Text::operator=( String && str )
            { return assign( std::move( str ) ); }

        Text & Text::operator=( const String & str )
            { return assign( str ); }

        Text & Text::operator=( Text && str )
            { return assign( std::move( str ) ); }

        Text & Text::operator=( const Text & str )
            { return assign( str ); }

        Text & Text::append( const Memory & memory )
        {
            if ( m_offsets.empty() )
                { scan(0); }
            size_t pos = m_str.length( );
            m_str.append( memory.begin( ), memory.end( ) );
            scan( pos );
            return *this;
        }

        Text & Text::append( unicode_t ch )
        {
            char dst[4];
            return append( Utf8::encode( ch, Memory{ dst, dst + 4 } ) );
        }

        Text & Text::operator+=( const Memory & memory )
            { return append(memory); }

        Text & Text::operator+=( unicode_t ch )
            { return append(ch); }

        void Text::push_back( unicode_t ch )
            { append(ch); }

        void Text::clear( )
        {
            m_str.clear( );
            m_offsets.clear( );
        }

        bool Text::isEmpty( ) const
        {
            return m_str.empty( );
        }

        Text::operator const char *( ) const
        {
            return begin( );
        }

        const char * Text::begin( ) const
        {
            return m_str.data( );
        }

        const char * Text::end( ) const
        {
            return m_str.data( ) + m_str.length( );
        }

        size_t Text::size( ) const
        {
            return m_str.size();
        }

        Memory Text::data( ) const
        {
            return Memory{ m_str };
        }

        size_t Text::length( ) const
        {
            if ( m_offsets.empty() )
                { scan(0); }
            return m_offsets.size( );
        }

        unicode_t Text::at( size_t pos ) const
        {
            if ( m_offsets.empty() )
                { scan(0); }
            int front = pos ? m_offsets[pos - 1] : 0;
            int back = m_offsets[pos];
            return decode( Memory{ begin( ) + front, begin( ) + back }, 0 );
        }

        unicode_t Text::operator[]( size_t pos ) const
        {
            return at( pos );
        }

        uint32_t Text::charOffset(uint32_t pos) const
        { 
            if ( m_offsets.empty() ) 
                { scan(0); }
            pos = std::min( pos, (uint32_t)m_offsets.size() ); 
            return pos > 0 ? m_offsets[pos - 1] : 0; 
        }

        uint32_t Text::ptrToPos( const char * ptr )
        {
            if ( m_offsets.empty() )
                { scan(0); }

            uint32_t begin = 0;
            uint32_t end = (uint32_t)m_offsets.size();
            uint32_t imin = begin;
            uint32_t imax = end - 1;
            uint32_t imid = (imin + imax) / 2;

            if ( end <= begin || ptr < m_str.c_str() )
                { return begin; }

            uint32_t offset = (uint32_t)( ptr - m_str.c_str( ) );

            while ( imin <= imax )
            {
                imid = (imin + imax) / 2;
                uint32_t mid1 = (uint32_t)charOffset(imid);
                uint32_t mid2 = (uint32_t)charOffset(imid+1);

                if ( mid2 <= offset )
                    { imin = imid + 1; }
                else if ( offset >= mid1 && offset < mid2 ) 
                    { return imid; }
                else
                    { imax = imid - 1; }
            }

            return end;
        }

        Memory Text::substr( size_t pos, size_t len ) const
        {
            if ( m_offsets.empty() )
                { scan(0); }
            int front = pos ? m_offsets[pos - 1] : 0;
            int back = m_offsets[pos + len];
            return Memory{ begin( ) + front, begin( ) + back };
        }

        void Text::scan( size_t pos ) const
        {
            int len = 0;
            int front = pos ? m_offsets[pos - 1] : 0;
            while ( front < m_str.length() )
            {
                unicode_t ch = decode( Memory{ begin( ) + front, end( ) }, 0, &len );
                front += len;
                m_offsets.push_back( front );
                pos++;
            }
        }



		inline unicode_t decodeResult( unicode_t value, int len, int * encodedLength )
		{
			if ( encodedLength )
			    { *encodedLength = len; }
			return value;
		}

        Memory substr( const Memory & utf8, size_t pos, size_t len )
        {
            int bytes;
            size_t index = 0;
            size_t byteOffset = 0;
            while ( index < pos && byteOffset < utf8.length() )
            {
                decode( utf8, byteOffset, &bytes );
                if ( bytes == 0 )
                    { return Memory::Empty; }
                index++;
                byteOffset += bytes;
            }

            size_t begin = byteOffset;
            while ( index - pos < len && byteOffset < utf8.length() )
            {
                decode( utf8, byteOffset, &bytes );
                if ( bytes == 0 )
                    { return Memory::Empty; }
                index++;
                byteOffset += bytes;
            }
            return utf8.substr( begin, byteOffset - begin );
        }
        
        size_t strlen( const Memory & utf8 )
        {
            int bytes;
            size_t index = 0;
            size_t byteOffset = 0;
            while ( byteOffset < utf8.length() )
            {
                decode( utf8, byteOffset, &bytes );
                if ( bytes == 0 )
                    { break; }
                index++;
                byteOffset += bytes;
            }
            return index;
        }

        Unicode toUnicode( const Memory & utf8 )
        {
            Unicode result;
            for ( auto ch : Utf8::Reader{ cpp::MemoryInput{ utf8 } } )
                { result += ch; }
            return result;
        }

        unicode_t decode( const Memory & utf8, size_t offset, int * encodedLength )
		{
			int len = 0;

			/* 1-byte sequence */
			int cp1 = (uint8_t)utf8.get(offset + 0);
			if ( cp1 < 0x80 )
			{
				return decodeResult( cp1, 1, encodedLength );
			}

			if ( cp1 < 0xC2 )
			{
				return decodeResult( BadCodePoint, 1, encodedLength );
			}

			/* 2-byte sequence */
			if ( utf8.length( ) < offset + 2 )
			{
				return decodeResult( BadCodePoint, 1, encodedLength );
			}

			int cp2 = (uint8_t)utf8.get(offset + 1);
			if ( cp1 < 0xE0 )
			{
				if ( ( cp2 & 0xC0 ) != 0x80 )
				{
					return decodeResult( BadCodePoint, 1, encodedLength );
				}
				return decodeResult( ( cp1 << 6 ) + cp2 - 0x3080, 2, encodedLength );
			}

			/* 3-byte sequence */
			if ( utf8.length( ) < offset + 3 )
			{
				return decodeResult( BadCodePoint, 1, encodedLength );
			}

			int cp3 = (uint8_t)utf8.get(offset + 2);
			if ( cp1 < 0xF0 ) {
				if ( ( cp2 & 0xC0 ) != 0x80 ||
					( cp3 & 0xC0 ) != 0x80 ||
					( cp1 == 0xE0 && cp2 < 0xA0 ) ) /* overlong */
				{
					return decodeResult( BadCodePoint, 1, encodedLength );
				}
				return decodeResult( ( cp1 << 12 ) + ( cp2 << 6 ) + cp3 - 0xE2080, 3, encodedLength );
			}

			/* 4-byte sequence */
			if ( utf8.length( ) < offset + 3 )
			{
				return decodeResult( BadCodePoint, 1, encodedLength );
			}

			int cp4 = (uint8_t)utf8.get(offset + 3);
			if ( cp1 < 0xF5 ) {
				/* 4-byte sequence */
				if ( ( cp2 & 0xC0 ) != 0x80 ||
					( cp3 & 0xC0 ) != 0x80 ||
					( cp4 & 0xC0 ) != 0x80 ||
					( cp1 == 0xF0 && cp2 < 0x90 ) || /* overlong */
					( cp1 == 0xF4 && cp2 >= 0x90 ) ) /* > U+10FFFF */
				{
					return decodeResult( BadCodePoint, 1, encodedLength );
				}
				return decodeResult( ( cp1 << 18 ) + ( cp2 << 12 ) + ( cp3 << 6 ) + cp4 - 0x3C82080, 4, encodedLength );
			}

			//	return bad character
			return decodeResult( BadCodePoint, 1, encodedLength );
		}

		Memory encode( unicode_t value, Memory & dst )
		{
			if ( value < 0x80 )
			{
                dst.put( 0, (char)value );
                return dst.substr( 0, 1 );
			}
			else if ( value <= 0x7FF )
			{
                dst.put( 0, (char)( ( value >> 6 ) + 0xC0 ) );
                dst.put( 1, (char)( ( value & 0x3F ) + 0x80 ) );
                return dst.substr( 0, 2 );
			}
			else if ( value <= 0xFFFF )
			{
                dst.put( 0, (char)( ( value >> 12 ) + 0xE0 ) );
                dst.put( 1, (char)( ( ( value >> 6 ) & 0x3F ) + 0x80 ) );
                dst.put( 2, (char)( ( value & 0x3F ) + 0x80 ) );
                return dst.substr( 0, 3 );
			}
			else if ( value <= 0x10FFFF )
			{
                dst.put( 0, (char)( ( value >> 18 ) + 0xF0 ) );
                dst.put( 1, (char)( ( ( value >> 12 ) & 0x3F ) + 0x80 ) );
                dst.put( 2, (char)( ( ( value >> 6 ) & 0x3F ) + 0x80 ) );
                dst.put( 3, (char)( ( value & 0x3F ) + 0x80 ) );
                return dst.substr( 0, 4 );
			}
			else
			{
				throw Exception( "invalid unicode code_point" );
			}
		}

        Utf8::Reader::iterator::iterator( Reader * reader )
            : m_reader( reader ) { get( ); }

        unicode_t Utf8::Reader::iterator::operator*( )
			{ return m_value; }

        bool Utf8::Reader::iterator::operator!=( iterator & iter ) const
			{ return m_reader != iter.m_reader; }

        Utf8::Reader::iterator & Utf8::Reader::iterator::operator++( )
			{ get( ); return *this; }

		void Utf8::Reader::iterator::get( )
		{
			if ( m_reader )
			{
                if ( !m_reader->isOpen( ) )
                    { m_reader = nullptr; return; }
                m_value = m_reader->get( );
			}
		}


        Utf8::Reader::Reader( const char * data )
            : m_input( StringInput{ data } ), m_buffer(4) { }

        Utf8::Reader::Reader( const Memory & data )
            : m_input( StringInput{ data } ), m_buffer(4) { }

        Utf8::Reader::Reader( const Utf8::Text & text )
            : m_input( StringInput{ text.data() } ), m_buffer(4) { }

        Utf8::Reader::Reader( Input input )
            : m_input( std::move( input ) ), m_buffer(4) { }

        bool Utf8::Reader::isOpen( ) const
			{ return m_buffer.getable() || m_input.isOpen(); }

        Utf8::Reader::iterator Utf8::Reader::begin( )
			{ return iterator( isOpen( ) ? this : nullptr ); }

        Utf8::Reader::iterator Utf8::Reader::end( )
	        { return iterator( nullptr ); }

        void Utf8::Reader::tryRead( )
        {
            if ( m_input && m_buffer.putable() )
                { m_buffer.put( m_input.read( m_buffer.putable() ) ); }
        }

        bool Utf8::Reader::tryDecode( unicode_t & result )
        {
            int len = 0;
            if ( m_buffer.getable() )
            {
                result = decode( m_buffer.getable(), 0, &len );
                if ( len > 0 )
                    { m_buffer.get(len); }
            }
            return len != 0;
        }

        unicode_t Utf8::Reader::get( )
		{
            unicode_t result = BadCodePoint;
            while ( !tryDecode( result ) && m_input )
                { tryRead( ); }
            return result;
		}

	}

}

#else

#include <cpp/util/Utf8.h>
#include <cpp/meta/Unittest.h>

SUITE( Utf8 )
{
	using namespace cpp;

	TEST( Reader )
	{
		int len = 0;
		for ( auto ch : Utf8::Reader{ "hello" } )
			{ len++; }
		CHECK( len == 5 );
	}

    TEST( Text )
    {

    }

    TEST( substr )
    {
        String utf8 = u8"ありがとう";
        
        CHECK( Utf8::strlen( utf8 ) == 5 && utf8.length() != 5 );
        CHECK( Utf8::substr( utf8, 1, 3 ) == u8"りがと" );
        CHECK( Utf8::substr( utf8, 2 ) == u8"がとう" );
        CHECK( Utf8::substr( utf8, 2, 10 ) == u8"がとう" );
        CHECK( Utf8::substr( utf8, 10, 2 ).isEmpty() );
    }

}

#endif

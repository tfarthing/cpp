#ifndef TEST

#include "../process/Exception.h"
#include "Utf16.h"
#include "Utf8.h"
//#include <cpp/io/StringInput.h>

namespace cpp
{
    namespace Utf16
    {

        Match match( const wchar_t * begin, const wchar_t * end, const wchar_t * regex )
        {
            Match results;

            std::wcmatch matchResults;
            if ( std::regex_match( begin, end, matchResults, std::wregex{ regex } ) )
            {
                for ( auto m : matchResults )
                    { results.emplace_back( m.first, m.second ); }
            }

            return results;
        }

        Match searchOne( const wchar_t * begin, const wchar_t * end, const wchar_t * regex )
        {
            Match result;

            std::wcmatch matchResults;
            if ( std::regex_search( begin, end, matchResults, std::wregex{ regex } ) )
            {
                for ( auto m : matchResults )
                    { result.emplace_back( m.first, m.second ); }
            }

            return result;
        }

        Matches searchAll( const wchar_t * begin, const wchar_t * end, const wchar_t * regex )
        {
            Matches results;

            std::wregex pattern{ regex };

            const wchar_t * pos = begin;
            while ( pos < end )
            {
                Match result;
                std::wcmatch matchResults;
                if ( !std::regex_search( pos, end, matchResults, pattern ) )
                    { break; }

                pos = matchResults.suffix( ).first;
                for ( auto m : matchResults )
                    { result.emplace_back( m.first, m.second ); }

                results.emplace_back( std::move( result ) );
            }

            return results;
        }


        Text Text::fromUtf8( const cpp::Memory & utf8 )
            { return Utf16::Text{ toUtf16( utf8 ) }; }

        Text::Text( )
            : m_str( ), m_offsets( ) { }

        Text::Text( unicode_t ch )
            : m_str(), m_offsets() { push_back(ch); }

        Text::Text( const Memory & str )
            : m_str( ), m_offsets( ) { assign(str); }

        Text::Text( const wchar_t * str )
            : m_str( str ), m_offsets( ) { }

        Text::Text( std::wstring && move )
            : m_str( std::move( move ) ), m_offsets( ) { }

        Text::Text( const std::wstring & copy )
            : m_str( copy ), m_offsets( ) { }

        Text::Text( Text && move )
            : m_str( std::move( move.m_str ) ), m_offsets( std::move( move.m_offsets ) ) { }

        Text::Text( const Text & copy )
            : m_str( copy.m_str ), m_offsets( copy.m_offsets ) { }

        Text::Text( std::wstring str, std::vector<uint32_t> offsets )
            : m_str(str), m_offsets(offsets) { }

        Text & Text::assign( const wchar_t * str )
        {
            m_offsets.clear( );
            m_str.assign( str );
            return *this;
        }

        Text & Text::assign( const wchar_t * begin, const wchar_t * end )
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
                const wchar_t * begin = reinterpret_cast<const wchar_t *>( str.begin( ) );
                const wchar_t * end = reinterpret_cast<const wchar_t *>( str.end( ) );
                m_str.assign( begin, end ); 
            }
            return *this;
        }

        Text & Text::assign( std::wstring && move )
        {
            m_offsets.clear( );
            m_str.assign( std::move( move ) );
            return *this;
        }

        Text & Text::assign( const std::wstring & copy )
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

        Text & Text::operator=( const wchar_t * str )
            { return assign( str ); }

        Text & Text::operator=( std::wstring && str )
            { return assign( std::move(str) ); }

        Text & Text::operator=( const std::wstring & str )
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
            const wchar_t * begin = reinterpret_cast<const wchar_t *>(memory.begin( ));
            const wchar_t * end = reinterpret_cast<const wchar_t *>(memory.end( ));
            m_str.append( begin, end );
            scan( pos );
            return *this;
        }

        Text & Text::append( unicode_t ch )
        {
            wchar_t dst[2];
            return append( Utf16::encode( ch, dst, dst+2 ) );
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

        Text::operator const wchar_t *( ) const
        {
            return begin( );
        }

        const wchar_t * Text::begin( ) const
        {
            return m_str.data( );
        }

        const wchar_t * Text::end( ) const
        {
            return m_str.data( ) + m_str.length( );
        }

        const wchar_t * Text::dataAt( size_t pos ) const
        {
            if ( m_offsets.empty() )
                { scan(0); }
            int front = pos ? m_offsets[pos - 1] : 0;
            return m_str.data( ) + front;
        }

        size_t Text::size( ) const
        {
            return end( ) - begin( );
        }

        Memory Text::data( ) const
        {
            return Memory{ (char *)m_str.data(), m_str.length() * sizeof(wchar_t) };
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
            return decode( begin( ) + front, begin( ) + back, 0 );
        }

        unicode_t Text::operator[]( size_t pos ) const
        {
            return at( pos );
        }

        Utf16::Text Text::substr( size_t pos, size_t len ) const
        {
            if ( m_offsets.empty() )
                { scan(0); }
            pos = std::min( m_offsets.size( ), pos );
            len = std::min( m_offsets.size( ) - pos, len );
            int front = pos ? m_offsets[pos - 1] : 0;
            int back = m_offsets[pos + len - 1];
            return Text( m_str.substr( front, back - front ), std::vector<uint32_t>{ m_offsets.data() + pos, m_offsets.data() + pos + len } );
        }

        Match Text::match( const wchar_t * regex ) const
        {
            return Utf16::match( begin( ), end( ), regex );
        }

        Match Text::searchOne( const wchar_t * regex ) const
        {
            return Utf16::searchOne( begin( ), end( ), regex );
        }

        Matches Text::searchAll( const wchar_t * regex ) const
        {
            return Utf16::searchAll( begin( ), end( ), regex );
        }

        void Text::scan( size_t pos ) const
        {
            int len = 0;
            int front = pos ? m_offsets[pos - 1] : 0;
            while ( pos < m_str.length() )
            {
                unicode_t ch = decode( begin( ) + front, end( ), 0, &len );
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

        unicode_t decode( const wchar_t * begin, const wchar_t * end, size_t offset, int * encodedLength )
		{
			int cp1 = *begin;
			if ( cp1 >= 0xD800 && cp1 <= 0xDBFF )
			{
                if ( begin + 1 >= end )
                    { return decodeResult( BadCodePoint, 1, encodedLength ); }
				int cp2 = *(begin + 1);
				if ( cp2 >= 0xDC00 && cp2 <= 0xDFFF )
				    { return decodeResult( ( cp1 << 10 ) + cp2 - 0x35FDC00, 2, encodedLength ); }
			}
			return decodeResult( cp1, 1, encodedLength );
		}

        Memory encode( unicode_t ch, wchar_t * dstBegin, const wchar_t * dstEnd )
        {
			if ( ch < 0x10000 && dstBegin < dstEnd )
			{
                *dstBegin = (wchar_t)ch;
                return Memory{ (const char *)dstBegin, sizeof( wchar_t ) };
			}
			else if ( ch <= 0x10FFFF && dstBegin + 1 < dstEnd )
			{
                *dstBegin = (wchar_t)( ( ch >> 10 ) + 0xD7C0 );
                *( dstBegin + 1 ) = (wchar_t)( ( ch >> 10 ) + 0xD7C0 );
                return Memory{ (const char *)dstBegin, sizeof( wchar_t ) * 2 };
			}
			else
			{
				throw Exception( "invalid unicode code_point" );
			}
        }
	
		/*
		Reader::iterator::iterator( Reader * reader )
					: m_reader( reader ) { get( ); }

        unicode_t Reader::iterator::operator*( ) const
			{ return m_value; }

		bool Reader::iterator::operator!=( iterator & iter ) const
			{ return m_reader != iter.m_reader; }
				
		Reader::iterator & Reader::iterator::operator++( )
			{ get( ); return *this; }

		void Reader::iterator::get( )
		{
			if ( m_reader )
			{
				if ( !m_reader->isOpen( ) )
                    { m_reader = nullptr; return; }
				m_value = m_reader->get( );
			}
		}

        Reader::Reader( const wchar_t * data )
            : m_input( StringInput{ Memory{ (const char *)data, wcslen(data) * sizeof(wchar_t) } } ), m_bufferOffset(0) { }

        Reader::Reader( const Utf16::Text & data )
            : m_input( StringInput{ Memory{ (const char *)data.begin(), (const char *)data.end() } } ), m_bufferOffset(0) { }

        Reader::Reader( const Memory & data )
            : m_input( StringInput{ data } ), m_bufferOffset(0) { }

		Reader::Reader( Input && input )
			: m_input( input ), m_bufferOffset(0) { }

		bool Reader::isOpen( )
			{ return m_bufferOffset > 0 || m_input.isOpen(); }
			
		Reader::iterator Reader::begin( )
			{ return iterator( isOpen( ) ? this : nullptr ); }
			
		Reader::iterator Reader::end( )
			{ return iterator( nullptr ); }

        void Reader::tryRead( )
        {
            while ( m_input && m_bufferOffset < 2 )
            { 
                m_input.getValue<wchar_t>( m_buffer[m_bufferOffset++] );
            }
        }

        bool Reader::tryDecode( unicode_t & result )
        {
            int len = 0;
            if ( m_bufferOffset )
            {
                result = decode( m_buffer, m_buffer + m_bufferOffset, 0, &len );
                if ( len == 1 && m_bufferOffset == 2 )
                    { *m_buffer = *(m_buffer + 1); m_bufferOffset = 1; }
                else if ( len > 0 )
                    { m_bufferOffset = 0; }
            }
            return len != 0;
        }

        unicode_t Reader::get( )
		{
            unicode_t result = BadCodePoint;
            while ( !tryDecode( result ) && m_input )
                { tryRead( ); }
            return result;
		}
		*/
	}

    std::wstring toUtf16( const cpp::Memory & utf8 )
    {
        std::wstring result;
		result.reserve( utf8.length( ) );

        size_t pos = 0;
        while ( pos < utf8.length() )
        {
            //  decode utf-8
            int len;
            unicode_t ch = Utf8::decode( utf8, pos, &len );
            pos += len;

            if ( ch == Utf8::BadCodePoint )
                { ch = 0xFFFD; }

            //  encode utf-16
            wchar_t dst[2];
            Memory encoded = Utf16::encode( ch, dst, dst + 2 );
            result.append( dst, dst + encoded.length( ) / sizeof( wchar_t ) );
        }
        
        return result;
    }


	std::string toUtf8( const wchar_t * begin, const wchar_t * end )
    {
        String result;
        result.data.reserve( end - begin );
        while ( begin < end )
        {
            //  decode utf-16
            int len;
            unicode_t ch = Utf16::decode( begin, begin + 2, 0, &len );
            begin += len;
            
            if ( ch == Utf8::BadCodePoint )
                { ch = 0xFFFD; }

            //  encode utf-8
            char dst[4];
            result += Utf8::encode( ch, Memory{ dst, dst + 4 } );
        }
        return result;
    }


	std::string toUtf8( const std::wstring & utf16 )
    {
        return toUtf8( utf16.data( ), utf16.data( ) + utf16.length() );
    }

}

#else

#include <cpp/util/Utf16.h>
#include <cpp/meta/Unittest.h>

SUITE( Utf16 )
{
	using namespace cpp;

	TEST( Reader )
	{
		int len = 0;
		for ( auto ch : Utf16::Reader{ L"hello" } )
			{ len++; }
		CHECK( len == 5 );
	}

    TEST( Text )
    {

    }

}

#endif

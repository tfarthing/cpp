#pragma once

#include "../../cpp/data/DataBuffer.h"


namespace cpp
{

    class Input;

    class LineReader
    {
    public:
        static const size_t                 MaxLineLength = 64 * 1024;
        struct Cursor
        {
            size_t						    position;
            size_t						    lineNumber;
            Memory						    line;
        };

        class iterator
        {
        public:
										    iterator( Input * input, size_t buflen );

            const Cursor &				    operator*( ) const;
            bool						    operator!=( iterator & iter ) const;
            iterator &					    operator++( );

        private:
            void						    get( );
            bool						    tryRead( );
            bool						    tryFind( );

        private:
            Input *			                m_input;
            size_t						    m_nextPosition;
            size_t						    m_nextLine;
            Cursor						    m_cursor;
            StringBuffer				    m_buffer;
            size_t						    m_findPos = 0;
        };

    public:
										    LineReader( Input & input, size_t buflen );

        iterator						    begin( ) const;
        iterator						    end( ) const;

        String::Array					    readAll( );

    private:
		Input &				                m_input;
        size_t							    m_buflen;
    };



    LineReader::iterator::iterator( Input * input, size_t buflen )
        : m_input( input ), m_buffer{ buflen }
    {
        m_nextPosition = 0;
        m_nextLine = 0;
        m_cursor = Cursor{ 0, 0, Memory::Empty };
        get( );
    }


    const LineReader::Cursor & LineReader::iterator::operator*( ) const
    {
        return m_cursor;
    }


    bool LineReader::iterator::operator!=( iterator & iter ) const
    {
        return &m_input != &( iter.m_input ) || m_cursor.position != iter.m_cursor.position;
    }


    LineReader::iterator & LineReader::iterator::operator++( )
    {
        get( ); return *this;
    }


    void LineReader::iterator::get( )
    {
        while ( !tryFind( ) && tryRead( ) );
    }



    LineReader::LineReader( Input & input, size_t buflen )
        : m_input( input ), m_buflen( buflen )
    {

    }


	LineReader::iterator LineReader::begin( ) const
    {
        return iterator{ &m_input, m_buflen };
    }
    

	LineReader::iterator LineReader::end( ) const
    {
        return iterator{ nullptr, 0 };
    }


    String::Array LineReader::readAll( )
    {
        String::Array results;
        for ( auto & cursor : *this )
        {
            results.push_back( cursor.line );
        }
        return results;
    }

}

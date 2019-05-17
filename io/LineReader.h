#pragma once

#include "../../cpp/data/DataBuffer.h"
#include "../../cpp/data/DataArray.h"



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

		StringArray							readAll( );

    private:
		Input &				                m_input;
        size_t							    m_buflen;
    };



    inline LineReader::iterator::iterator( Input * input, size_t buflen )
        : m_input( input ), m_buffer{ buflen }
    {
        m_nextPosition = 0;
        m_nextLine = 0;
        m_cursor = Cursor{ 0, 0, Memory::Empty };
        get( );
    }


	inline const LineReader::Cursor & LineReader::iterator::operator*( ) const
    {
        return m_cursor;
    }


	inline bool LineReader::iterator::operator!=( iterator & iter ) const
    {
        return m_input != iter.m_input || m_nextPosition != iter.m_nextPosition;
    }


	inline LineReader::iterator & LineReader::iterator::operator++( )
    {
        get( ); return *this;
    }


	inline void LineReader::iterator::get( )
    {
        while ( !tryFind( ) && tryRead( ) );
    }



	inline LineReader::LineReader( Input & input, size_t buflen )
        : m_input( input ), m_buflen( buflen )
    {

    }


	inline LineReader::iterator LineReader::begin( ) const
    {
        return iterator{ &m_input, m_buflen };
    }
    

	inline LineReader::iterator LineReader::end( ) const
    {
        return iterator{ nullptr, 0 };
    }


	inline StringArray LineReader::readAll( )
    {
		StringArray results;
        for ( auto & cursor : *this )
        {
            results.add( cursor.line );
        }
        return results;
    }

}

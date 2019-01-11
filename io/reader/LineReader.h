#pragma once

#include "../../data/String.h"

namespace cpp
{

    class Input;

    class LineReader
    {
    public:
        class Cursor
        {
        public:
            Cursor()
                : m_position(0), m_lineNumber(0), m_line() { }

            Cursor(size_t position, size_t lineNumber, const Memory & line)
                : m_position( position ), m_lineNumber( lineNumber ), m_line( line ) { }

            const Memory & line() const
                { return m_line; }

            size_t lineNumber() const
                { return m_lineNumber; }

            size_t position() const
                { return m_position; }

            bool isNull() const
                { return m_line.isNull(); }

        private:
            size_t m_position;
            size_t m_lineNumber;
            Memory m_line;
        };

        class iterator
        {
        public:
            iterator( LineReader * reader )
                : m_reader( reader ) { get( ); }

            const Cursor & operator*( ) const
                { return m_cursor; }

            bool operator!=( iterator & iter ) const
                { return m_reader != iter.m_reader; }

            iterator & operator++( )
                { get( ); return *this; }

        private:
            void get( );

        private:
            LineReader * m_reader;
            Cursor m_cursor;
        };

    public:
        LineReader( const Input & input, size_t buflen, Duration timeout = Duration::Infinite );

        bool isOpen( ) const;
        iterator begin( );
        iterator end( );

        Cursor get( );
        Cursor flush( );
        String::Array getAll( );

    private:
        struct Detail;
        std::shared_ptr<Detail> m_detail;
    };

}

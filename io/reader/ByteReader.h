#pragma once

#include <cpp/Object.h>
#include <cpp/String.h>
#include <cpp/util/Handle.h>

namespace cpp
{

    class Input;

    class ByteReader
        : public Object
    {
    public:
        class Cursor
        {
        public:
            Cursor()
                : m_position(0), m_data() { }

            Cursor(size_t position, const Memory & data)
                : m_position( position ), m_data(data) { }

            const Memory & data() const
                { return m_data; }

            size_t offset() const
                { return m_position; }

            bool isNull() const
                { return m_data.isNull(); }

        private:
            size_t m_position;
            Memory m_data;
        };

        class iterator
        {
        public:
            iterator( ByteReader * reader )
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
            ByteReader * m_reader;
            Cursor m_cursor;
        };

    public:
        ByteReader( const Input & input, size_t buflen, Duration timeout = Duration::infinite );

        bool isOpen( ) const;

        iterator begin( );
        iterator end( );

        Cursor get( );
        String getAll( );

    private:
        struct Detail;
        Handle<Detail> m_detail;
    };

}

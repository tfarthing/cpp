#pragma once

#include <regex>
#include <cpp/Object.h>
#include <cpp/String.h>
#include <cpp/util/Handle.h>

namespace cpp
{

    class Input;

    class SearchReader
        : public Object
    {
    public:
        class Cursor
        {
        public:
            Cursor()
                : m_position(0), m_matchIndex(0), m_match() { }

            Cursor( size_t position, size_t matchIndex, const std::cmatch & match )
                : m_position( position ), m_matchIndex( matchIndex ), m_match( match ) { }

            const std::cmatch & match() const
                { return m_match; }

            size_t matchIndex() const
                { return m_matchIndex; }

            size_t position() const
                { return m_position; }

            bool isNull() const
                { return !m_match.ready(); }

        private:
            size_t m_position;
            size_t m_matchIndex;
            std::cmatch m_match;
        };

        class iterator
        {
        public:
            iterator( SearchReader * reader )
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
            SearchReader * m_reader;
            Cursor m_cursor;
        };

    public:
        SearchReader( const Memory & regex, const Input & input, size_t buflen, Duration timeout = Duration::infinite );

        bool isOpen( ) const;
        iterator begin( );
        iterator end( );

        Cursor get( );

    private:
        struct Detail;
        Handle<Detail> m_detail;
    };

}

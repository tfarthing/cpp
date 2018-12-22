#pragma once

#include <vector>
#include <regex>

namespace cpp
{
    template<typename T>
    class RegexMatch
    {
    public:
        RegexMatch( );
        RegexMatch( const std::cmatch & source );
        RegexMatch( std::vector<T> source );

        RegexMatch operator=( const std::cmatch & source );

        bool operator!( ) const;
        explicit operator bool( ) const;

        bool hasMatch( ) const;
        const T & at( size_t index ) const;
        const T & operator[]( size_t index ) const;
        const std::vector<T> & groups( ) const;

    private:
        std::vector<T> m_groups;
    };


    template<typename T> RegexMatch<T>::RegexMatch( )
        {}

    template<typename T> RegexMatch<T>::RegexMatch( const std::cmatch & source )
        { for ( auto m : source ) { m_groups.push_back( Memory{ m.first, m.second } ); } }

    template<typename T> RegexMatch<T>::RegexMatch( std::vector<T> groups )
        : m_groups( std::move( groups ) ) {}

    template<typename T> RegexMatch<T> RegexMatch<T>::operator=( const std::cmatch & source )
        { m_groups.clear( ); for ( auto m : source ) { m_groups.push_back( Memory{ m.first, m.second } ); } return *this; }

    template<typename T> bool RegexMatch<T>::operator!( ) const
        { return !hasMatch( ); }
    
    template<typename T> RegexMatch<T>::operator bool( ) const
        { return hasMatch( ); }

    template<typename T> bool RegexMatch<T>::hasMatch( ) const
        { return m_groups.size() > 0; }

    template<typename T> const T & RegexMatch<T>::at( size_t index ) const
        { return m_groups[index]; }

    template<typename T> const T & RegexMatch<T>::operator[]( size_t index ) const
        { return m_groups[index]; }

    template<typename T> const std::vector<T> & RegexMatch<T>::groups( ) const
        { return m_groups; }

}
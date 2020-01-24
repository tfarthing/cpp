#pragma once

#include <vector>
#include <regex>



namespace cpp
{

    template<typename T>
    struct RegexMatch
    {
							RegexMatch( );
							RegexMatch( const std::cmatch & source );

        RegexMatch &		operator=( const std::cmatch & source );

        bool				operator!( ) const;
							explicit operator bool( ) const;

        bool				hasMatch( ) const;
        const T &			at( size_t index ) const;
        const T &			operator[]( size_t index ) const;

		std::vector<T>		groups;
    };


    template<typename T> RegexMatch<T>::RegexMatch( )
        { }

    template<typename T> RegexMatch<T>::RegexMatch( const std::cmatch & source )
        { for ( auto m : source ) { groups.push_back( T{ m.first, m.second } ); } }

    template<typename T> RegexMatch<T> & RegexMatch<T>::operator=( const std::cmatch & source )
        { groups.clear( ); for ( auto m : source ) { groups.push_back( T{ m.first, m.second } ); } return *this; }

    template<typename T> bool RegexMatch<T>::operator!( ) const
        { return !hasMatch( ); }
    
    template<typename T> RegexMatch<T>::operator bool( ) const
        { return hasMatch( ); }

    template<typename T> bool RegexMatch<T>::hasMatch( ) const
        { return groups.size() > 0; }

    template<typename T> const T & RegexMatch<T>::at( size_t index ) const
        { return groups[index]; }

    template<typename T> const T & RegexMatch<T>::operator[]( size_t index ) const
        { return groups[index]; }

}
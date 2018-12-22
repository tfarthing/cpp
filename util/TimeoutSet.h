#pragma once

#include <optional>
#include <cpp/chrono/Time.h>

namespace cpp
{

    template<typename T>
    class TimeoutSet
    {
    public:
        TimeoutSet( );

        bool contains( const T & value ) const;
        std::optional<cpp::Time> nextTimeout( ) const;

        std::vector<T> poll( );
        bool insert( const T & value, cpp::Duration timeoutDuration );
        bool erase( const T & value );
        void clear( );

    private:
        std::map<T, cpp::Time> m_indexMap;
        std::map<cpp::Time, std::set<T> > m_timeoutMap;
    };

    template <typename T>
    TimeoutSet<T>::TimeoutSet( )
    {
    }

    template <typename T>
    bool TimeoutSet<T>::contains( const T & value ) const
    {
        return m_indexMap.count( value ) != 0;
    }

    template <typename T>
    std::optional<cpp::Time> TimeoutSet<T>::nextTimeout( ) const
    {
        return ( m_timeoutMap.empty( ) ) 
            ? std::optional<cpp::Time>{}
            : std::optional<cpp::Time>{ m_timeoutMap.begin( )->first };
    }

    template <typename T>
    void TimeoutSet<T>::clear( )
    {
        m_indexMap.clear( );
        m_timeoutMap.clear( );
    }

    template <typename T>
    bool TimeoutSet<T>::insert( const T & value, cpp::Duration timeoutDuration )
    {
        bool touchExisting = erase( value );
        cpp::Time then = cpp::Time::inFuture( timeoutDuration );
        m_indexMap[value] = then;
        m_timeoutMap[then].insert( value );
        return touchExisting;
    }

    template <typename T>
    bool TimeoutSet<T>::erase( const T & value )
    {
        auto itr = m_indexMap.find( value );
        if ( itr != m_indexMap.end( ) )
        {
            auto & timeoutSet = m_timeoutMap[itr->second];
            timeoutSet.erase( value );
            if ( timeoutSet.empty( ) )
                { m_timeoutMap.erase( itr->second ); }
            m_indexMap.erase( itr );
            return true;
        }
        return false;
    }

    template <typename T>
    std::vector<T> TimeoutSet<T>::poll( )
    {
        std::vector<T> result;
        while ( !m_timeoutMap.empty() && m_timeoutMap.begin( )->first <= cpp::Time::now( ) )
        {
            auto & timeoutSet = m_timeoutMap.begin( )->second;
            for ( T & item : timeoutSet )
                { result.push_back( item ); }
            m_timeoutMap.erase( m_timeoutMap.begin( ) );
        }
        return result;
    }

}

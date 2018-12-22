#pragma once

#include <map>

#include <cpp/types.h>
#include <cpp/Exception.h>

namespace cpp
{

    template<typename T>
    class IndexedSet
    {
    public:
        IndexedSet( );

        static const size_t npos = (size_t)-1;

        void reserve( size_t size );
        void resize( size_t size, T placeholder );
        void clear( );

        void add( T key );
        void addAt( size_t index, T key );
        void setAt( size_t index, T key );

        bool remove( T key );
        bool removeAt( size_t index );

        size_t size( ) const;
        bool contains( T key ) const;

        T getAt( size_t index ) const;
        T rgetAt( size_t index ) const;

        size_t lowerBoundOf( T key ) const;
        size_t rlowerBoundOf( T key ) const;
        
        size_t indexOf( T key ) const;
        size_t rindexOf( T key ) const;
        
        std::vector<T> list( size_t index, size_t count ) const;
        std::vector<T> rlist( size_t index, size_t count ) const;

        size_t rindex( size_t index ) const;

    private:
        std::map<T, uint32_t> m_posMap;
        std::vector<T> m_array;
    };



    template<typename T>
    IndexedSet<T>::IndexedSet( )
    {
    }

    template<typename T>
    void IndexedSet<T>::reserve( size_t size )
    {
        m_array.reserve( size );
    }

    template<typename T>
    void IndexedSet<T>::resize( size_t size, T placeholder )
    {
        assert( indexOf( placeholder ) == npos );
        m_array.resize( size, placeholder );
    }

    template<typename T>
    void IndexedSet<T>::clear( )
    {
        m_posMap.clear( );
        m_array.clear( );
    }

    template<typename T>
    void IndexedSet<T>::add( T key )
    {
        remove( key );

        m_posMap[key] = (uint32_t)m_array.size( );
        m_array.push_back( key );
    }

    template<typename T>
    void IndexedSet<T>::addAt( size_t index, T key )
    {
        remove( key );

        if ( index > size( ) )
            { throw cpp::OutOfBoundsException( cpp::String::format( "IndexedSet::addAt() : index='%', size='%'", index, size( ) ) ); }

        m_array.insert( m_array.begin( ) + index, key );
        m_posMap[key] = (uint32_t)index;
        for ( size_t i = index + 1; i < m_array.size( ); i++ )
            { m_posMap[m_array.at( i )] = (uint32_t)i; }
    }

    template<typename T>
    void IndexedSet<T>::setAt( size_t index, T key )
    {
        remove( key );

        if ( index > size( ) )
            { throw cpp::OutOfBoundsException( cpp::String::format( "IndexedSet::addAt() : index='%', size='%'", index, size( ) ) ); }

        m_array[index] = key;
        m_posMap[key] = (uint32_t)index;
    }

    template<typename T>
    bool IndexedSet<T>::remove( T key )
    {
        return removeAt( indexOf( key ) );
    }

    template<typename T>
    bool IndexedSet<T>::removeAt( size_t index )
    {
        if ( index >= size( ) )
            { return false; }

        // shift array indexes, not an efficient operation unless key is at tail
        for ( size_t i = index + 1; i < size( ); i++ )
        {
            T & k = m_array.at( i );
            m_posMap[k] = (uint32_t)i - 1;
        }
        m_posMap.erase( m_array[index] );
        m_array.erase( m_array.begin() + index );

        return true;
    }

    template<typename T>
    size_t IndexedSet<T>::size( ) const
    {
        return m_array.size( );
    }

    template<typename T>
    bool IndexedSet<T>::contains( T key ) const
    {
        return m_posMap.count( key ) > 0;
    }

    template<typename T>
    T IndexedSet<T>::getAt( size_t index ) const
    {
        if ( index >= m_array.size( ) )
            { throw cpp::OutOfBoundsException{ cpp::String::format( "IndexedSet::getAt() : index(%) is out-of-bounds, size(%)", index, m_array.size( ) ) }; }
        return m_array.at( index );
    }

    template<typename T>
    T IndexedSet<T>::rgetAt( size_t index ) const
    {
        return getAt( rindex( index ) );
    }

    template<typename T>
    size_t IndexedSet<T>::lowerBoundOf( T key ) const
    {
        auto itr = m_posMap.lower_bound( key );
        return (itr != m_posMap.end( )) ? indexOf( itr.first ) : npos;
    }

    template<typename T>
    size_t IndexedSet<T>::rlowerBoundOf( T key ) const
    {
        auto itr = m_posMap.lower_bound( key );
        return (itr != m_posMap.end( )) ? rindexOf( itr.first ) : npos;
    }

    template<typename T>
    size_t IndexedSet<T>::indexOf( T key ) const
    {
        auto itr = m_posMap.find( key );
        return (itr != m_posMap.end( )) ? itr->second : npos;
    }

    template<typename T>
    size_t IndexedSet<T>::rindexOf( T key ) const
    {
        auto itr = m_posMap.find( key );
        return (itr != m_posMap.end( )) ? rindex( index ) : npos;
    }

    template<typename T>
    std::vector<T> IndexedSet<T>::list( size_t pos, size_t count ) const
    {
        std::vector<T> result;
        if ( pos < m_array.size( ) ) {
            if ( count < 0 || pos + count >= m_array.size( ) )
            {
                count = m_array.size( ) - pos;
            }
            size_t end = pos + count;
            for ( size_t i = pos; i < end; i++ )
            {
                result.push_back( m_array.at( i ) );
            }
        }
        return result;
    }

    template<typename T>
    std::vector<T> IndexedSet<T>::rlist( size_t pos, size_t count ) const
    {
        std::vector<T> result;
        if ( pos < m_array.size( ) ) {
            if ( count < 0 || pos + count >= m_array.size( ) )
            {
                count = m_array.size( ) - pos;
            }
            int end = pos + count;
            for ( size_t i = pos; i < end; i++ )
            {
                result.add( m_array.at( rindex( i ) ) );
            }
        }
        return result;
    }

    template<typename T>
    size_t IndexedSet<T>::rindex( size_t index ) const
    {
        return m_array.size( ) - index - 1;
    }


}

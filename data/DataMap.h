#pragma once

/*

	DataMap and StringMap are extensions of std::map.

	(1) Objects of each can convert to/from DataMap & StringMap.
	(2) Extends the interface of std::map, except operator[] which is overridden as read-only.  
	(3) Adds isEmpty(), notEmpty(), get(), set(), remove().
	(4) get() and operator[] return a Memory object which may be null.
	(5) toText() to encode the map as a text string, a constructor to decode the map from EncodedText (e.g. DataMap map = data.asText();). 
	(6) toBinary() to encode the map to a DataBuffer, fromBinary() to decode the map from a DataBuffer. 

*/

#include <map>
#include <cpp/data/Memory.h>



namespace cpp
{

    template<class K, class V>
    struct DataMap
    {
		typedef std::map<K, V>              map_t;

							                DataMap( );
							                DataMap( DataMap && move ) noexcept;
							                DataMap( const DataMap & copy );
                                            DataMap( std::initializer_list<std::pair<Memory,Memory>> init );

        template<class Kb, class Vb>        DataMap( DataMap<Kb,Vb> && move ) noexcept;
        template<class Kb, class Vb>        DataMap( const DataMap<Kb, Vb> & copy );
        template<class Kb, class Vb>        DataMap( std::map<Kb, Vb> && move ) noexcept;
        template<class Kb, class Vb>        DataMap( const std::map<Kb, Vb> & copy );

        DataMap &			                operator=( DataMap && move ) noexcept;
        DataMap &			                operator=( const DataMap & copy );
        template<class Kb, class Vb>
		DataMap &			                operator=( DataMap<Kb, Vb> && move ) noexcept;
        template<class Kb, class Vb>
		DataMap &			                operator=( const DataMap<Kb, Vb> & copy );
        template<class Kb, class Vb>
		DataMap &			                operator=( std::map<Kb, Vb> && move ) noexcept;
        template<class Kb, class Vb>
		DataMap &			                operator=( const std::map<Kb, Vb> & copy );

        bool								isEmpty( ) const;
        bool								notEmpty( ) const;
        size_t                              size( ) const;

        Memory							    get( const typename map_t::key_type & k ) const;
        Memory								operator[]( const typename map_t::key_type & k ) const;

        template<class M> void	            set( typename map_t::key_type && k, M && value );
        template<class M> void 	            set( const typename map_t::key_type & k, M && value );
        void    						    remove( Memory key );
        void                                clear( );

        map_t                               data;
    };



    typedef DataMap<Memory, Memory> MemoryMap;
    typedef DataMap<String, String> StringMap;
    


    template<class K, class V>
    DataMap<K,V>::DataMap( )
        {}


    template<class K, class V>
    DataMap<K,V>::DataMap( DataMap && move ) noexcept
        : data( std::move( move.data ) ) {}


    template<class K, class V>
    DataMap<K,V>::DataMap( const DataMap & copy )
        : data( copy.data ) {}


    template<class K, class V>
    DataMap<K,V>::DataMap( std::initializer_list<std::pair<Memory, Memory>> init )
    {
        for ( auto & item : init )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ item.first, item.second } );
        }
    }


    template<class K, class V>
    template<class Kb, class Vb> DataMap<K, V>::DataMap( DataMap<Kb, Vb> && move ) noexcept
    {
        for ( auto & itr : move.data )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ std::move(itr.first), std::move(itr.second) } );
        }
    }


    template<class K, class V>
    template<class Kb, class Vb> DataMap<K, V>::DataMap( const DataMap<Kb, Vb> & copy )
    {
        for ( auto & itr : copy.data )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ itr.first, itr.second } );
        }
    }


    template<class K, class V>
    template<class Kb, class Vb> DataMap<K, V>::DataMap( std::map<Kb, Vb> && move ) noexcept
    {
        for ( auto & itr : move )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ std::move( itr.first ), std::move( itr.second ) } );
        }
    }


    template<class K, class V>
    template<class Kb, class Vb> DataMap<K, V>::DataMap( const std::map<Kb, Vb> & copy )
    {
        for ( auto & itr : copy )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ itr.first, itr.second } );
        }
    }


    template<class K, class V>
    DataMap<K, V> & DataMap<K, V>::operator=( DataMap && move ) noexcept
    {
        data = std::move( move.data );
        return *this;
    }


    template<class K, class V>
    DataMap<K, V> & DataMap<K, V>::operator=( const DataMap & copy )
    {
        data = move.data;
        return *this;
    }


    template<class K, class V>
    template<class Kb, class Vb>
    DataMap<K, V> & DataMap<K, V>::operator=( DataMap<Kb, Vb> && move ) noexcept
    {
        data.clear( );
        for ( auto & itr : move.data )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ std::move( itr.first ), std::move( itr.second ) } );
        }
        return *this;
    }


    template<class K, class V>
    template<class Kb, class Vb>
    DataMap<K, V> & DataMap<K, V>::operator=( const DataMap<Kb, Vb> & copy )
    {
        data.clear( );
        for ( auto & itr : copy.data )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ itr.first, itr.second } );
        }
        return *this;
    }


    template<class K, class V>
    template<class Kb, class Vb>
    DataMap<K, V> & DataMap<K, V>::operator=( std::map<Kb, Vb> && move ) noexcept
    {
        data.clear( );
        for ( auto & itr : move )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ std::move( itr.first ), std::move( itr.second ) } );
        }
        return *this;
    }


    template<class K, class V>
    template<class Kb, class Vb>
    DataMap<K, V> & DataMap<K, V>::operator=( const std::map<Kb, Vb> & copy )
    {
        data.clear( );
        for ( auto & itr : copy )
        {
            data.emplace_hint( data.end( ), std::map<K, V>::value_type{ itr.first, itr.second } );
        }
        return *this;
    }


    template<class K, class V>
    bool DataMap<K, V>::isEmpty( ) const
    {
        return data.empty( );
    }
    
    
    template<class K, class V>
    bool DataMap<K, V>::notEmpty( ) const
    {
        return !data.empty( );
    }

    
    template<class K, class V>
    size_t DataMap<K, V>::size( ) const
    {
        return data.size( );
    }


    template<class K, class V>
    Memory DataMap<K, V>::get( const typename map_t::key_type & k ) const
    {
        auto itr = data.find( k );
        return ( itr != data.end( ) )
            ? Memory{ itr->second }
            : nullptr;
    }


    template<class K, class V>
    Memory DataMap<K, V>::operator[]( const typename map_t::key_type & k ) const
    {
        auto itr = data.find( k );
        return ( itr != data.end( ) )
            ? Memory{ itr->second }
            : nullptr;
    }


    template<class K, class V>
    template<class M> void DataMap<K, V>::set( typename map_t::key_type && k, M && value )
    {
        data.insert_or_assign( std::move( k ), std::forward<M>( value ) );
    }


    template<class K, class V>
    template<class M> void DataMap<K, V>::set( const typename map_t::key_type & k, M && value )
    {
        data.insert_or_assign( k, std::forward<M>( value ) );
    }


    template<class K, class V>
    void DataMap<K, V>::remove( Memory key )
    {
        data.erase( key );
    }


    template<class K, class V>
    void DataMap<K, V>::clear( )
    {
        data.clear( );
    }

}

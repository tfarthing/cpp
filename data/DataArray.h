#pragma once

/*

	DataArray abstracts a std::vector for binary data items, i.e. MemoryArray and StringArray.

	(1) Objects of each can convert to/from DataArray & StringArray.
	(2) Extends the interface of std::vector, except operator[] which is overridden as read-only.
	(3) Adds isEmpty(), notEmpty(), get(), set(), remove().
	(4) get() and operator[] return a Memory object which may be null.
	(5) toText() to encode the vector as a text string, a constructor to decode the vector from EncodedText (e.g. DataArray array = data.asText();).
	(6) toBinary() to encode the vector to a DataBuffer, fromBinary() to decode the vector from a DataBuffer.

*/

#include <vector>

#include "String.h"
#include "DataBuffer.h"


namespace cpp
{

	class DataBuffer;

    template<class T>
	struct DataArray
	{
        static DataArray<T>                 ofSize( size_t count, const T & value = T{} );

											DataArray( );
											DataArray( DataArray<T> && move ) noexcept;
											DataArray( const DataArray<T> & copy );
                                            DataArray( std::initializer_list<T> init );

		template<class Y>	                DataArray( std::vector<Y> && move ) noexcept;
		template<class Y>	                DataArray( const std::vector<Y> & copy );
        template<class Y>	                DataArray( DataArray<Y> && move ) noexcept;
        template<class Y>	                DataArray( const DataArray<Y> & copy );

											DataArray( const EncodedText & encodedText );     // from comma-separated list e.g. "one, two, three"
											DataArray( const EncodedBinary & encodedBinary ); // from length encoded list

        DataArray<T> &	                    operator=( std::vector<T> && move ) noexcept;
        DataArray<T> &	                    operator=( const std::vector<T> & copy );
        DataArray<T> &	                    operator=( DataArray<T> && move ) noexcept;
        DataArray<T> &	                    operator=( const DataArray<T> & copy );
        template<class Y> DataArray<T> &	operator=( std::vector<Y> && move ) noexcept;
        template<class Y> DataArray<T> &	operator=( const std::vector<Y> & copy );
        template<class Y> DataArray<T> &	operator=( DataArray<Y> && move ) noexcept;
        template<class Y> DataArray<T> &	operator=( const DataArray<Y> & copy );

        DataArray<T> &                      operator=( const EncodedText & encodedText );     // from comma-separated list e.g. "one, two, three"
        DataArray<T> &                      operator=( const EncodedBinary & encodedBinary ); // from length encoded list

        bool								isEmpty( ) const;
		bool								notEmpty( ) const;
        size_t                              size( ) const;

        Memory							    get( size_t index ) const;
        Memory								operator[]( size_t index ) const;

        template<class Y> void				add( Y && value );
        template<class Y> void				add( const Y & value );
        template<class Y> void				set( size_t index, const Y & value );
        template<class Y> void 			    set( size_t index, Y && value );
        void    						    remove( size_t index );
        void                                clear( );

        Memory								toText( DataBuffer & buffer = StringBuffer::writeTo( 1024 ) );	//	comma-separated list
        Memory								toBinary( DataBuffer & buffer = StringBuffer::writeTo( 1024 ), ByteOrder byteOrder = ByteOrder::Host );

		std::vector<T>					    data;
	};



    typedef DataArray<Memory> MemoryArray;
    typedef DataArray<String> StringArray;



    template<class T>
    DataArray<T> DataArray<T>::ofSize( size_t count, const T & value )
    {
        DataArray<T> result;
        result.data.resize( 10, value );
        return result;
    }


    template<class T>
    DataArray<T>::DataArray( )
        { }


    template<class T>
    DataArray<T>::DataArray( DataArray<T> && move ) noexcept
        : data( std::move( move.data ) ) { }


    template<class T>
    DataArray<T>::DataArray( const DataArray<T> & copy )
        : data( copy.data ) { }


    template<class T>
    template<class Y> DataArray<T>::DataArray( std::vector<Y> && move ) noexcept
    { 
        data.reserve( move.size( ) );
        for ( size_t i = 0; i < move.size( ); i++ )
        {
            data.emplace_back( std::move( move[i] ) );
        }
        move.clear( );
    }


    template<class T>
    template<class Y> DataArray<T>::DataArray( const std::vector<Y> & copy )
    {
        data.reserve( copy.size( ) );
        for ( size_t i = 0; i < copy.size( ); i++ )
        {
            data.emplace_back( copy[i] );
        }
    }


    template<class T>
    template<class Y> DataArray<T>::DataArray( DataArray<Y> && move ) noexcept
    {
        data.reserve( move.size( ) );
        for ( size_t i = 0; i < move.size( ); i++ )
        {
            data.emplace_back( std::move( move.data[i] ) );
        }
        move.clear( );
    }


    template<class T>
    template<class Y> DataArray<T>::DataArray( const DataArray<Y> & copy )
    {
        data.reserve( copy.size( ) );
        for ( size_t i = 0; i < copy.size( ); i++ )
        {
            data.emplace_back( copy.data[i] );
        }
    }


    template<class T>
    DataArray<T>::DataArray( std::initializer_list<T> init )
        : data( std::move( init ) ) { }


    template<class T>
    DataArray<T>::DataArray( const EncodedText & encodedText )
    {
        for ( auto & item : encodedText.data.split( ",", Memory::WhitespaceList, false ) )
        {
            data.push_back( EncodedText{ item } );
        }
    }


    template<class T>
    DataArray<T>::DataArray( const EncodedBinary & encodedBinary )
    {
        DataBuffer buffer{ encodedBinary.data };
        uint32_t len = buffer.getBinary<uint32_t>( );
        for ( size_t i = 0; i < len; i++ )
        {
            data.push_back( buffer.getBinary<Memory>( ) );
        }
    }


    template<class T>
    DataArray<T> & DataArray<T>::operator=( std::vector<T> && move ) noexcept
    {
        data = std::move( move );
        return *this;
    }


    template<class T>
    DataArray<T> & DataArray<T>::operator=( const std::vector<T> & copy )
    {
        data = copy;
        return *this;
    }


    template<class T>
    DataArray<T> & DataArray<T>::operator=( DataArray <T> && move ) noexcept
    {
        data = std::move( move.data );
        return *this;
    }


    template<class T>
    DataArray<T> & DataArray<T>::operator=( const DataArray<T> & copy )
    {
        data = move.data;
        return *this;
    }


    template<class T>
    template<class Y> DataArray<T> & DataArray<T>::operator=( std::vector<Y> && move ) noexcept
    {
        data.clear( );
        data.reserve( move.size( ) );
        for ( size_t i = 0; i < move.size( ); i++ )
        {
            data.emplace_back( std::move( move[i] ) );
        }
        move.clear( );
        return *this;
    }


    template<class T>
    template<class Y> DataArray<T> & DataArray<T>::operator=( const std::vector<Y> & copy )
    {
        data.clear( );
        data.reserve( copy.size( ) );
        for ( size_t i = 0; i < copy.size( ); i++ )
        {
            data.emplace_back( copy[i] );
        }
        return *this;
    }


    template<class T>
    template<class Y> DataArray<T> & DataArray<T>::operator=( DataArray<Y> && move ) noexcept
    {
        return operator=( std::move( move.data ) );
    }


    template<class T>
    template<class Y> DataArray<T> & DataArray<T>::operator=( const DataArray<Y> & copy )
    {
        return operator=( copy.data );
    }


    template<class T>
    DataArray<T> & DataArray<T>::operator=( const EncodedText & encodedText )
    {
        for ( auto & item : encodedText.data.split( ",", Memory::WhitespaceList, false ) )
        {
            data.push_back( EncodedText{ item } );
        }
        return *this;
    }


    template<class T>
    DataArray<T> & DataArray<T>::operator=( const EncodedBinary & encodedBinary )
    {
        DataBuffer buffer{ encodedBinary.data };
        uint32_t len = buffer.getBinary<uint32_t>( );
        for ( size_t i = 0; i < len; i++ )
        {
            data.push_back( buffer.getBinary<Memory>( ) );
        }
        return *this;
    }


    template<class T>
    Memory DataArray<T>::operator[]( size_t index ) const
    {
        return data[index];
    }


    template<class T>
    bool DataArray<T>::isEmpty( ) const
    {
        return data.empty( );
    }


    template<class T>
    bool DataArray<T>::notEmpty( ) const
    {
        return !data.empty( );
    }


    template<class T>
    size_t DataArray<T>::size( ) const
    {
        return data.size( );
    }


    template<class T>
    template<class Y> void DataArray<T>::add( const Y & value )
    {
        data.emplace_back( value );
    }


    template<class T>
    template<class Y> void DataArray<T>::add( Y && value )
    {
        data.emplace_back( std::move( value ) );
    }


    template<class T>
    Memory DataArray<T>::get( size_t index ) const
    {
        return ( index <= size( ) ) ? Memory{ data[index] } : nullptr;
    }


    template<class T>
    template<class Y> void DataArray<T>::set( size_t index, const Y & value )
    {
        data[index] = value;
    }


    template<class T>
    template<class Y> void DataArray<T>::set( size_t index, Y && value )
    {
        data[index] = std::move( value );
    }


    template<class T>
    void DataArray<T>::remove( size_t index )
    {
        Memory item = data[index]; data.erase( data.begin( ) + index ); return item;
    }


    template<class T>
    void DataArray<T>::clear( )
    {
        data.clear( );
    }


    template<class T>
    Memory DataArray<T>::toText( DataBuffer & buffer )
    {
        size_t pos = buffer.getable( ).length( );
        for ( auto & item : data )
        {
            if ( buffer.getPutPos( ) != pos )
            {
                buffer.put( "," );
            }
            buffer.put( item );
        }
        return buffer.getable( ).substr( pos );
    }


    template<class T>
    Memory DataArray<T>::toBinary( DataBuffer & buffer, ByteOrder byteOrder )
    {
        size_t pos = buffer.getable( ).length( );
        buffer.putBinary( data.size( ) );
        for ( size_t i = 0; i < data.size( ); i++ )
        {
            buffer.putBinary( data[i] );
        }
        return buffer.getable( ).substr( pos );
    }


    template<class T, class Y> int compare( const std::vector<T> & lhs, const std::vector<Y> & rhs )
    {
        for ( size_t i = 0; i < lhs.size( ); i++ )
        {
            if ( rhs.size( ) <= i )
                { return -1; }

            int itemCompare = Memory::compare( lhs[i], rhs[i] );
            if ( itemCompare == 0 )
                { continue; }
            return itemCompare;
        }

        return ( rhs.size( ) == lhs.size( ) ) ? 0 : 1;
    }


    template<class T, class Y> int compare( const DataArray<T> & lhs, const std::vector<Y> & rhs )
    {
        return compare( lhs.data, rhs );
    }


    template<class T, class Y> int compare( const std::vector<Y> & lhs, const DataArray<T> & rhs )
    {
        return compare( lhs, rhs.data );
    }


    template<class T, class Y> int compare( const DataArray<Y> & lhs, const DataArray<T> & rhs )
    {
        return compare( lhs.data, rhs.data );
    }

}






template<class T, class Y> bool operator==( const cpp::DataArray<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) == 0; }
template<class T, class Y> bool operator!=( const cpp::DataArray<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) != 0; }
template<class T, class Y> bool operator<( const cpp::DataArray<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) < 0; }
template<class T, class Y> bool operator>( const cpp::DataArray<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) > 0; }
template<class T, class Y> bool operator<=( const cpp::DataArray<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) <= 0; }
template<class T, class Y> bool operator>=( const cpp::DataArray<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) >= 0; }

template<class T, class Y> bool operator==( const cpp::DataArray<T> & lhs, const std::vector<Y> & rhs )
    { return cpp::compare( lhs, rhs ) == 0; }
template<class T, class Y> bool operator!=( const cpp::DataArray<T> & lhs, const std::vector<Y> & rhs )
    { return cpp::compare( lhs, rhs ) != 0; }
template<class T, class Y> bool operator<( const cpp::DataArray<T> & lhs, const std::vector<Y> & rhs )
    { return cpp::compare( lhs, rhs ) < 0; }
template<class T, class Y> bool operator>( const cpp::DataArray<T> & lhs, const std::vector<Y> & rhs )
    { return cpp::compare( lhs, rhs ) > 0; }
template<class T, class Y> bool operator<=( const cpp::DataArray<T> & lhs, const std::vector<Y> & rhs )
    { return cpp::compare( lhs, rhs ) <= 0; }
template<class T, class Y> bool operator>=( const cpp::DataArray<T> & lhs, const std::vector<Y> & rhs )
    { return cpp::compare( lhs, rhs ) >= 0; }

template<class T, class Y> bool operator==( const std::vector<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) == 0; }
template<class T, class Y> bool operator!=( const std::vector<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) != 0; }
template<class T, class Y> bool operator<( const std::vector<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) < 0; }
template<class T, class Y> bool operator>( const std::vector<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) > 0; }
template<class T, class Y> bool operator<=( const std::vector<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) <= 0; }
template<class T, class Y> bool operator>=( const std::vector<T> & lhs, const cpp::DataArray<Y> & rhs )
    { return cpp::compare( lhs, rhs ) >= 0; }

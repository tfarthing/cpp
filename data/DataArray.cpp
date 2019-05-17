#include "DataArray.h"
#include "DataBuffer.h"

namespace cpp
{

    DataArray::DataArray( )
        { }


    DataArray::DataArray( size_t count )
        : data( count ) { }


    DataArray::DataArray( DataArray && move ) noexcept
        : data( std::move( move.data ) ) { }


    DataArray::DataArray( const DataArray & copy )
        : data( copy.data ) { }


    DataArray::DataArray( std::vector<Memory> && move ) noexcept
        : data( std::move( move ) ) { }


    DataArray::DataArray( const std::vector<Memory> & copy )
        : data( std::move( copy ) ) { }


    DataArray::DataArray( const StringArray & copy )
        : data( copy.data.begin( ), copy.data.end( ) ) { }


    DataArray::DataArray( std::initializer_list<Memory> init )
        : data( init ) { }


    DataArray::DataArray( const EncodedText & encodedText )
    {
        for ( auto & item : encodedText.data.split( ",", Memory::WhitespaceList, false ) )
            { data.push_back( EncodedText{ item } ); }
    }


    DataArray::DataArray( const EncodedBinary & encodedBinary )
    {
        DataBuffer buffer{ encodedBinary.data };
        uint32_t len = buffer.getBinary<uint32_t>( );
        for ( size_t i = 0; i < len; i++ )
            { data.push_back( buffer.getBinary<Memory>( ) ); }
    }


    DataArray & DataArray::operator=( DataArray && move ) noexcept
        { data = std::move( move.data ); return *this; }


    DataArray & DataArray::operator=( const DataArray & copy )
        { data = copy.data; return *this; }


    DataArray & DataArray::operator=( std::vector<Memory> && move ) noexcept
        { data = std::move( move ); return *this; }


    DataArray & DataArray::operator=( const std::vector<Memory> & copy )
        { data = copy; return *this; }


    DataArray & DataArray::operator=( const StringArray & copy )
    {
        for ( auto & item : copy.data )
            { data.push_back( item ); }
        return *this;
    }


    Memory DataArray::operator[]( size_t index ) const
        { return data[index]; }


    bool DataArray::isEmpty( ) const
        { return data.empty( ); }


    bool DataArray::notEmpty( ) const
        { return !data.empty( ); }


    void DataArray::add( const Memory & value )
        { data.push_back( value ); }


    Memory DataArray::get( size_t index ) const
        { return data[index]; }


    void DataArray::set( size_t index, const Memory & value )
        { data[index] = value; }
    
    
    Memory DataArray::remove( size_t index )
        { Memory item = data[index]; data.erase( data.begin( ) + index ); return item; }


    Memory DataArray::toText( DataBuffer & buffer )
    {
        size_t pos = buffer.getable( ).length( );
        for ( auto & item : data )
        {
            if ( buffer.getPutPos( ) != pos )
                { buffer.put( "," ); }
            buffer.put( item );
        }
        return buffer.getable( ).substr( pos );
    }


    Memory DataArray::toBinary( DataBuffer & buffer, ByteOrder byteOrder )
    {
        size_t pos = buffer.getable( ).length( );
        buffer.putBinary( data.size( ) );
        for ( size_t i = 0; i < data.size( ); i++ )
            { buffer.putBinary( data[i] ); }
        return buffer.getable( ).substr( pos );
    }


    StringArray::StringArray( )
        { }


    StringArray::StringArray( size_t count )
        : data( count ) { }


    StringArray::StringArray( StringArray && move ) noexcept
        : data( std::move( move.data ) ) { }


    StringArray::StringArray( const StringArray & copy )
        : data( copy.data ) { }


    StringArray::StringArray( std::vector<String> && move ) noexcept
        : data( std::move( move ) ) { }


    StringArray::StringArray( const std::vector<String> & copy )
        : data( copy ) { }


    StringArray::StringArray( const DataArray & copy )
        : data( copy.data.begin(), copy.data.end() ) { }


    StringArray::StringArray( std::initializer_list<String> init )
        : data( init ) { }


    StringArray::StringArray( const EncodedText & encodedText )
    {
        for ( auto & item : encodedText.data.split( ",", Memory::WhitespaceList, false ) )
            { data.push_back( String{ EncodedText{ item } } ); }
    }


    StringArray::StringArray( const EncodedBinary & encodedBinary )
    {
        DataBuffer buffer{ encodedBinary.data };
        uint32_t len = buffer.getBinary<uint32_t>( );
        for ( size_t i = 0; i < len; i++ )
            { data.push_back( buffer.getBinary<Memory>( ) ); }
    }


    StringArray & StringArray::operator=( StringArray && move ) noexcept
    {
        data = std::move( move.data );
        return *this;
    }


    StringArray & StringArray::operator=( const StringArray & copy )
    {
        data = copy.data;
        return *this;
    }


    StringArray & StringArray::operator=( std::vector<String> && move ) noexcept
    {
        data = std::move( move );
        return *this;
    }


    StringArray & StringArray::operator=( const std::vector<String> & copy )
    {
        data =copy;
        return *this;
    }


    StringArray & StringArray::operator=( const DataArray & copy )
    {
        data.assign( copy.data.begin( ), copy.data.end( ) );
        return *this;
    }


    Memory StringArray::operator[]( size_t index ) const
    {
        return data[index];
    }


    bool StringArray::isEmpty( ) const
    {
        return data.empty( );
    }


    bool StringArray::notEmpty( ) const
    {
        return !data.empty( );
    }


    void StringArray::add( String value )
    {
        data.push_back( std::move( value ) );
    }


    Memory StringArray::get( size_t index ) const
    {
        return data[index];
    }


    void StringArray::set( size_t index, String value )
    {
        data[index] = std::move( value );
    }


    String StringArray::remove( size_t index )
    {
        String item = std::move( data[index] );
        data.erase( data.begin( ) + index ); 
        return item;
    }


    Memory StringArray::toString( DataBuffer & buffer ) const
    {
        size_t pos = buffer.getable( ).length( );
        for ( auto & item : data )
        {
            if ( buffer.getPutPos( ) != pos )
                { buffer.put( "," ); }
            buffer.put( item );
        }
        return buffer.getable( ).substr( pos );
    }


    Memory StringArray::toBinary( DataBuffer & buffer, ByteOrder byteOrder ) const
    {
        size_t pos = buffer.getable( ).length( );
        buffer.putBinary( data.size( ) );
        for ( size_t i = 0; i < data.size( ); i++ )
            { buffer.putBinary( data[i] ); }
        return buffer.getable( ).substr( pos );
    }

}
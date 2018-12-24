#pragma once

/*

	DataBuffer and StringBuffer are useful for reading and writing encoded data in a buffer.
	Each have the same interface but the StringBuffer owns the memory for the buffer and can 
	resize it.  A DataBuffer uses a buffer passed into it.

	(1) Writes are done using "put" operations, read using "get" operations.
	(2) Data to be read can be examined using DataBuffer::getable().
	(3) IO operations can write derectly into the DataBuffer::putable() and the 
		Databuffer::put() operation registered after the write completes.

*/

#include <cpp/data/ByteOrder.h>
#include <cpp/data/RegexMatch.h>
#include <cpp/data/Integer.h>
#include <cpp/data/Float.h>
#include <cpp/process/Exception.h>
#include <cpp/data/Memory.h>
#include <cpp/data/String.h>



namespace cpp
{

    class DataBuffer
    {
    public:
        static DataBuffer writeTo( Memory data );
        static DataBuffer readFrom( Memory data );

        DataBuffer( Memory data, bool isReadable = true );

        size_t size( ) const;
        void clear( );

        const Memory getable( ) const;
        const Memory putable( ) const;
        Memory putable( );

        size_t pos( ) const;
        void setPos( size_t pos );

        Memory getAll( );
        Memory get( size_t bytes );
        Memory getAt( size_t pos, size_t bytes );

        Memory put( size_t len );
        Memory put( const Memory & memory );
        void trim( );

        //  Reads a line (including delimiter), or null if not found.
        Memory getLine( Memory delim = "\n", size_t pos = Memory::npos );
        
        //  Reads a block of bytes whose length is prepended in the data (as binary int T, e.g. int32_t, int16_t).
        //  The returned block will include all data after the prepended size.  Returns null if unable to read the block.
        //  Throws DecodeException if the prepended size is larger than the buffer would allow.
        template<class T> Memory getBlock( bool inclusiveLength = true, ByteOrder byteOrder = ByteOrder::Host );
        template<class T> Memory putBlock( Memory block, bool inclusiveLength = true, ByteOrder byteOrder = ByteOrder::Host );

        //  Matches the specified regex to the front of the buffer.  
        //  If no match is found, no bytes are read.  If a match is found the results are read from the buffer.
        RegexMatch<Memory> getRegex( Memory regex );
        RegexMatch<Memory> getRegex( const std::regex & regex );

        template<class T> T getBinary( ByteOrder byteOrder = ByteOrder::Host );
        template<class T> void getBinary( T & value, ByteOrder byteOrder = ByteOrder::Host );
        template<class T> void getText( T & value );
        template<class T> void getDecimal( T & value );
        template<class T> void getHex( T & value );
        template<class T> void getBase64( T & value );

        template<class T> void putBinary( const T & value, ByteOrder byteOrder = ByteOrder::Host );
        template<class T> void putText( const T & value );
        template<class T> void putDecimal( const T & value );
        template<class T> void putHex( const T & value );
        template<class T> void putBase64( const T & value );

        size_t getPutPos( ) const;
        void setPutPos( size_t pos );

    protected:
        const Memory unusable( ) const;
        void checkRead( size_t index, size_t len ) const;
        void checkWrite( size_t index, size_t len ) const;

    protected:
        Memory m_getBuffer;
        size_t m_getIndex;
        size_t m_putIndex;
    };



    class StringBuffer
        : public DataBuffer
    {
    public:
        static StringBuffer readFrom( std::string data );
        static StringBuffer writeTo( size_t bufsize );

        StringBuffer( std::string data );
        StringBuffer( size_t bufsize = 0 );

        void resize( size_t bufsize );

    private:
        String m_buffer;
    };


    inline DataBuffer DataBuffer::writeTo( Memory data )
        { return DataBuffer{ data, false }; }

    inline DataBuffer DataBuffer::readFrom( Memory data )
        { return DataBuffer{ data, true }; }

    inline DataBuffer::DataBuffer( Memory data, bool isReadable )
        : m_getIndex( 0 ), m_putIndex( isReadable ? data.length( ) : 0 ), m_getBuffer( data ) { }

    //  resets the data buffer to an empty state
    inline void DataBuffer::clear( )
        { m_getIndex = m_putIndex = 0; }

    //  returns the size of the underlying data buffer (unusable, getable, and putable)
    inline size_t DataBuffer::size( ) const
        { return m_getBuffer.length( ); }

    //  unusable() returns the byte range already read, and unavailable for writing: byte range [0, getPos)
    inline const Memory DataBuffer::unusable( ) const
        { return m_getBuffer.substr( 0, m_getIndex ); }

    //  getable() returns the byte range that is available for reading: byte range [getPos, putPos)
    inline const Memory DataBuffer::getable( ) const
        { return Memory{ m_getBuffer.data( ) + m_getIndex, m_putIndex - m_getIndex }; }

    //  putable() returns the byte range that is available for writing: byte range [putPos, getBuffer.end())
    inline const Memory DataBuffer::putable( ) const
        { return Memory{ m_getBuffer.data( ) + m_putIndex, m_getBuffer.length( ) - m_putIndex }; }

    //  returns the byte index of the next read operation (get)
    inline size_t DataBuffer::pos( ) const
        { return m_getIndex; }

    //  sets the byte index of the next read operation (get)
    inline void DataBuffer::setPos( size_t pos )
        { checkRead( pos, 0 ); m_getIndex = pos; }

    //  Reads a chunk whose length is prepended in the data (as binary int).
    //  The returned chunk will not include the length data, and returns null if unable to read the chunck
    template<class len_t> Memory DataBuffer::getBlock( bool inclusiveLength, ByteOrder byteOrder )
    {
        size_t origin = pos( );
        len_t len;
        getBinary( len, byteOrder );
        if ( inclusiveLength )
            { len -= sizeof( len_t ); }
        if ( getable( ).length( ) < len )
        {
            setPos( origin );
            return Memory::Null;
        }
        return get( len );
    }

    template<class T> Memory DataBuffer::putBlock( Memory block, bool inclusiveLength, ByteOrder byteOrder )
    {
        checkWrite( m_putIndex, block.length( ) + sizeof( T ) );

        T blockLength = block.length( ) + ( inclusiveLength ? sizeof( T ) : 0 );
        Memory first = putBinary( blockLength, byteOrder );
        Memory second = put( block );
        return Memory{ first.begin( ), second.end( ) };
    }

    template<class T> T DataBuffer::getBinary( ByteOrder byteOrder )
        { T value; getBinary( value, byteOrder ); return value; }

    template<class T> void DataBuffer::getBinary( T & value, ByteOrder byteOrder )
        { decodeBinary( *this, value, byteOrder ); }

    template<class T> void DataBuffer::getText( T & value )
        { decodeText( *this, value ); }

    template<class T> void DataBuffer::getDecimal( T & value )
        { decodeDecimal( *this, value ); }

    template<class T> void DataBuffer::getHex( T & value )
        { decodeHex( *this, value ); }

    template<class T> void DataBuffer::getBase64( T & value )
        { decodeBase64( *this, value ); }

    template<class T> void DataBuffer::putBinary( const T & value, ByteOrder byteOrder )
        { encodeBinary( *this, value, byteOrder ); }

    template<class T> void DataBuffer::putText( const T & value )
        { encodeText( *this, value ); }

    template<class T> void DataBuffer::putDecimal( const T & value )
        { encodeDecimal( *this, value ); }

    template<class T> void DataBuffer::putHex( const T & value )
        { encodeHex( *this, value ); }

    template<class T> void DataBuffer::putBase64( const T & value )
        { encodeBase64( *this, value ); }

    inline size_t DataBuffer::getPutPos( ) const
        { return m_putIndex; }

    inline void DataBuffer::setPutPos( size_t pos )
        { checkWrite( pos, 0); }

    inline void DataBuffer::checkRead( size_t index, size_t len ) const
        { check<OutOfBoundsException>( index >= 0 && index + len <= m_getBuffer.length( ), "DataBuffer::checkRead() failed" ); }

    inline void DataBuffer::checkWrite( size_t index, size_t len ) const
        { check<OutOfBoundsException>( index >= 0 && index + len <= (int)m_getBuffer.length( ), "DataBuffer::checkWrite() failed" ); }
    


    inline StringBuffer StringBuffer::writeTo( size_t bufsize )
        { return StringBuffer{ bufsize }; }

    inline StringBuffer StringBuffer::readFrom( std::string data )
        { return StringBuffer{ std::move( data ) }; }

    inline void StringBuffer::resize( size_t size )
        { m_buffer.resize( size ); m_getBuffer = m_buffer; }




    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    void encodeBinary( DataBuffer & buffer, T value, ByteOrder byteOrder = ByteOrder::Host )
    { 
        value = Memory::trySwap( value, byteOrder );
        buffer.put( Memory::ofValue( value ) );
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    void decodeBinary( DataBuffer & buffer, T & value, ByteOrder byteOrder = ByteOrder::Host )
    { 
        Memory result = buffer.get( sizeof( value ) );
        Memory::copy( Memory::ofValue( value ), result );
        value = Memory::trySwap<T>( value, byteOrder );
    }

    inline void encodeBinary( DataBuffer & buffer, const cpp::String & value, ByteOrder byteOrder = ByteOrder::Host )
    { 
        buffer.putBinary( (uint32_t)value.length( ), byteOrder );
        buffer.put( Memory{ value } );
    }

    inline void decodeBinary( DataBuffer & buffer, cpp::String & value, ByteOrder byteOrder = ByteOrder::Host )
    { 
        uint32_t size;
        buffer.getBinary( size, byteOrder );
        value = buffer.get( size );
    }

    template<typename T> void encodeBinary( DataBuffer & buffer, const std::vector<T> & value, ByteOrder byteOrder = ByteOrder::Host )
    {
        buffer.putBinary( (uint32_t)value.size( ), byteOrder );
        for ( const T & item : value )
            { buffer.putBinary( item, byteOrder ); }
    }

    template<typename T> void decodeBinary( DataBuffer & buffer, std::vector<T> & value, ByteOrder byteOrder = ByteOrder::Host )
    {
        uint32_t size;
        buffer.getBinary( size, byteOrder );

        value.clear( );
        value.reserve( size );
        for ( uint32_t i = 0; i < size; i++ )
        {
            T item;
            buffer.getBinary( item, byteOrder );
            value.emplace_back( std::move( item ) );
        }
    }


    /*
    class StringBuffer
    {
    public:
        StringBuffer( size_t size = 0, ByteOrder byteOrder = hostByteOrder() )
            : m_getIndex(0), m_putIndex(0), m_buffer(size, '\0'), m_byteOrder(byteOrder) { }

        StringBuffer( Memory buffer, ByteOrder byteOrder = hostByteOrder( ) )
            : m_getIndex( 0 ), m_putIndex( buffer.length() ), m_buffer( buffer ), m_byteOrder( byteOrder ) { }

        StringBuffer( String buffer, ByteOrder byteOrder = hostByteOrder( ) )
            : m_getIndex( 0 ), m_putIndex( buffer.length() ), m_buffer( std::move(buffer) ), m_byteOrder( byteOrder ) { }

        void setByteOrder( ByteOrder byteOrder )
            { m_byteOrder = byteOrder; }

        void clear()
            { m_getIndex = m_putIndex = 0; }

        const Memory data() const
            { return Memory{ m_buffer }; }

        const Memory getable() const
            { return Memory{ m_buffer.c_str() + m_getIndex, m_putIndex - m_getIndex }; }

        const Memory putable() const
            { return Memory{ m_buffer.c_str( ) + m_putIndex, m_buffer.length() - m_putIndex }; }

        Memory putable( )
            { if ( m_putIndex == m_buffer.length( ) && m_getIndex != 0 ) { trim( ); } return Memory{ m_buffer.c_str( ) + m_putIndex, m_buffer.length( ) - m_putIndex }; }

        const Memory unusable() const
            { return Memory{ m_buffer.c_str(), m_getIndex }; }

        size_t size() const
            { return m_buffer.size(); }

        void resize( size_t size )
            { m_buffer.resize(size); }

        void trim()
        { 
            if ( m_getIndex != 0 )
            { 
                size_t len = m_putIndex - m_getIndex; 
                if ( len != 0 )
                    { memmove( (char *)m_buffer.c_str( ), m_buffer.c_str( ) + m_getIndex, len ); }
                m_putIndex = len;
                m_getIndex = 0;
            }
        }

        size_t tellPut( )
            { return m_putIndex; }

        size_t tellGet( )
            { return m_getIndex; }

        void seekPut( size_t pos )
            { m_putIndex = pos; }

        void seekGet( size_t pos )
            { m_getIndex = pos; }

        Memory put( size_t len )
        {
            Memory result = Memory{ m_buffer.c_str( ) + m_putIndex, len };
            m_putIndex += len;
            return result;
        }

        Memory put( Input & input, cpp::Duration timeout = cpp::Duration::infinite )
        { 
            Memory bytes = input.read( putable( ), timeout );
            m_putIndex += bytes.length( );
            return bytes;
        }

        Memory put( Input & input, size_t len )
        {
            checkWrite( m_putIndex, len );
            Memory result{ m_buffer.c_str( ) + m_putIndex, len };
            input.get( result );
            m_putIndex += len;
            return result;
        }

        Memory put( const Memory & memory )
            { Memory dst = putable( ); checkWrite( m_putIndex, memory.length( ) ); Memory result = Memory::copy( dst, memory ); m_putIndex += memory.length( ); return result; }

        Memory getAll( )
            { return get( m_putIndex - m_getIndex ); }

        Memory get( size_t bytes)
            { return getAt( m_getIndex, bytes ); }
        
        Memory getAt( size_t pos, size_t bytes )
            { checkRead( pos, bytes ); Memory result{ m_buffer.c_str( ) + pos, bytes }; m_getIndex = pos + bytes; return result; }


        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        void putValue( T value )
            { value = Memory::trySwap( value, m_byteOrder ); put( Memory::ofValue( value ) ); }

        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        void getValue( T & value )
            { Memory::copy( Memory::ofValue( value ), get( sizeof( value ) ) ); value = Memory::trySwap<T>( value, m_byteOrder ); }

        void putValue( const cpp::String & value )
            { putValue( (uint32_t)value.length() ); put( Memory{ value } ); }
        void getValue( cpp::String & value )
            { uint32_t size; getValue( size ); value.resize( size, 0 ); Memory::copy( Memory{ value }, get( size ) ); }

        template<typename T> void putValue( const std::vector<T> & value );
        template<typename T> void getValue( std::vector<T> & value );

    private:
        void checkRead( size_t index, size_t len ) const
            { check<IOException>( index >= 0 && index + len <= (int)m_buffer.size(), "StringBuffer read failed" ); }

        void checkWrite( size_t index, size_t len ) const
            { check<IOException>( index >= 0 && index + len <= (int)m_buffer.size(), "StringBuffer write failed" ); }

    private:
        size_t m_putIndex;
        size_t m_getIndex;
        String m_buffer;
        ByteOrder m_byteOrder;
    };

    template<typename T> void StringBuffer::putValue( const std::vector<T> & value )
    {
        putValue( (uint32_t)value.size( ) );
        for ( const T & item : value )
        {
            putValue( item );
        }
    }

    template<typename T> void StringBuffer::getValue( std::vector<T> & value )
    {
        uint32_t size; 
        getValue( size );

        value.clear( );
        value.reserve( size );
        for ( uint32_t i = 0; i < size; i++ )
        {
            T item;
            getValue( item );
            value.push_back( item );
        }
    }
    */

}

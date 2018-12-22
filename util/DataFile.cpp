#ifndef TEST

#include <cpp/io/file/File.h>

#include <cpp/util/DataFile.h>

namespace cpp
{

    DataFile::DataFile( )
    {
    }

    DataFile::DataFile( cpp::FilePath filename )
    {
        load( filename );
    }

    void DataFile::load( cpp::FilePath filename )
    {
        m_filename = filename;
        reload( );
    }

    void DataFile::reload( )
    {
        check<Exception>( !m_filename.isEmpty( ), "DataFile::write() : no filename specified" );
        if ( !File::exists( filename( ) ) )
            { return; }

        for ( int retries = 0; retries < 5; retries++ )
        {
            try
            {
                m_data.clear( );
                auto backupFilename = filename( ).withExtension( "backup", true );
                auto output = File::write( backupFilename );
                for ( auto cursor : File::read( filename( ) ).lines( ) )
                { 
                    try
                    {
                        auto record = BitData::decode( cursor.line( ) );
                        //  null record means remove from map
                        if ( record.data.empty( ) )
                        {
                            m_data.erase( record.key );
                        }
                        else
                        {
                            for ( auto & datum : record.data )
                            {
                                // null key means remove from map
                                if ( datum.second.value == Memory::Null )
                                {
                                    m_data[record.key].remove( datum.first );
                                }
                                else
                                {
                                    m_data[record.key].set(datum.first, datum.second.value);
                                }
                            }
                        }
                    }
                    catch ( cpp::Exception & )
                    {
                        output.put( cursor.line( ) ); output.put( "\n" );
                    }
                }
                
                for ( auto & itr : m_data )
                { 
                    output.put( BitData::encode( itr.first, itr.second ) );
                }

                output.close( );

                auto oldFilename = filename( ).withExtension( "old", true );
                File::rename( filename( ), oldFilename );
                File::rename( backupFilename, filename( ) );
                File::remove( oldFilename );

                return;
            }
            catch ( IOException & )
            {
                Thread::sleep( Duration::ofMillis( 200 ) );
            }
        }
        throw cpp::TimeoutException{};
    }

    bool DataFile::isOpen( ) const
    {
        return !m_filename.isEmpty( );
    }

    cpp::Memory DataFile::get( Memory key ) const
    {
        return m_data.get( key );
    }

    uint64_t DataFile::getUint( Memory key ) const
    {
        return BitData::decodeUint( get( key ) );
    }

    uint64_t DataFile::getHex( Memory key ) const
    {
        return BitData::decodeHex( get( key ) );
    }

    int64_t DataFile::getInt( Memory key ) const
    {
        return BitData::decodeInt( get( key ) );
    }

    f64_t DataFile::getFloat( Memory key ) const
    {
        return BitData::decodeF64( get( key ) );
    }

    bool DataFile::getBool( Memory key ) const
    {
        return BitData::decodeBool( get( key ) );
    }

    DateTime DataFile::getTime( Memory key ) const
    {
        return BitData::decodeTime( get( key ) );
    }
    
    void DataFile::set( cpp::Memory key, cpp::Memory value )
    {
        auto keys = RecordMap::splitKey( key );
        m_data.set( keys.first, keys.second, value );
        write( BitData::encode( keys.first, { { keys.second, value } } ) );
    }

    void DataFile::setUint( Memory key, uint64_t value )
    {
        set( key, BitData::encodeUint( value ) );
    }

    void DataFile::setHex( Memory key, uint64_t value )
    {
        set( key, BitData::encodeHex( value ) );
    }

    void DataFile::setInt( Memory key, int64_t value )
    {
        set( key, BitData::encodeInt( value ) );
    }

    void DataFile::setFloat( Memory key, f64_t value )
    {
        set( key, BitData::encodeF64( value ) );
    }

    void DataFile::setBool( Memory key, bool value )
    {
        set( key, BitData::encodeBool( value ) );
    }

    void DataFile::setTime( Memory key, DateTime value )
    {
        set( key, BitData::encodeTime( value ) );
    }

    void DataFile::remove( String key )
    {
        auto keys = RecordMap::splitKey( key );
        m_data.remove( keys.first, keys.second );
        write( BitData::encode( keys.first, { { keys.second, Memory::Null } } ) );
    }

    void DataFile::setRecord( Memory recordKey, const DataMap & data )
    {
        m_data.setRecord( recordKey, data );
        write( BitData::encode( recordKey, data ) );
    }

    DataMap DataFile::getRecord( Memory recordKey ) const
    {
        return m_data.getRecord( recordKey );
    }

    void DataFile::removeRecord( Memory recordKey )
    {
        m_data.removeRecord( recordKey );

        write( BitData::encode( recordKey, {} ) );
    }

    void DataFile::write( Memory line )
    {
        check<Exception>( !m_filename.isEmpty( ), "DataFile::write() : no filename specified" );

        for ( int retries = 0; retries < 5; retries++ )
        {
            try
            {
                auto output = File::append( filename( ) );
                output.put( line );
                return;
            }
            catch ( IOException & )
            {
                Thread::sleep( Duration::ofMillis( 200 ) );
            }
        }
        throw cpp::TimeoutException{};
    }

}

#else

#include <cpp/meta/Unittest.h>

#include "DataFile.h"

SUITE( DataFile )
{
    TEST( test1 )
    {
        cpp::DataFile file{".unittest/datafile.txt"};

        file.set( "server1.addr", "192.168.1.1" );
        file.set( "server1.port", "60" );

        auto data = file.getRecord("server1");
        CHECK( data.at( "addr" ) == "192.168.1.1" );
        CHECK( data.at( "port" ) == "60" );

        file.remove( "server1.port" );

        file.reload( );

        file.setRecord( "server2", { {"addr", "10.10.10.1"}, {"port", "61"} } );
        CHECK( file.get( "server2.addr" ) == "10.10.10.1" );
    }
}

#endif

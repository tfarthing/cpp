#include "../../cpp/file/SyncFile.h"
#include "../../cpp/util/BitFile.h"



namespace cpp::bit
{
    File::File( )
    {

    }


    File::File( FilePath filename, Handler handler )
        : m_filename{ std::move( filename ) }, m_handler{ std::move( handler ) }
    {
        reload( );
    }


    FilePath File::filename( )
    {
        return m_filename;
    }


    const Object & File::data( ) const
    {
        m_data;
    }


    void File::load( FilePath filename, Handler handler )
    {
        m_filename = std::move( filename );
        m_handler = std::move( handler );
        m_data.reset( );
    }


    void File::reload( )
    {
        check<IOException>( !m_filename.empty(), "bit::File::reload() : no filename specified" );
        if ( !Files::exists( filename( ) ) )
            { return; }

        for ( int retries = 0; retries < 5; retries++ )
        {
            try
            {
                m_data.reset( );
                auto backupFilename = m_filename.append( ".backup" );
                auto file = SyncFile::create( backupFilename );

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
                            for ( auto& datum : record.data )
                            {
                                // null key means remove from map
                                if ( datum.second.value == Memory::Null )
                                {
                                    m_data[record.key].remove( datum.first );
                                }
                                else
                                {
                                    m_data[record.key].set( datum.first, datum.second.value );
                                }
                            }
                        }
                    }
                    catch ( cpp::Exception& )
                    {
                        output.put( cursor.line( ) ); output.put( "\n" );
                    }
                }

                for ( auto& itr : m_data )
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
            catch ( IOException& )
            {
                Thread::sleep( Duration::ofMillis( 200 ) );
            }
        }
        throw cpp::TimeoutException{};
    }


    bool File::isOpen( ) const
    {
    }


    Memory File::get( Memory key ) const
    {
    }


    void File::set( Memory key, Memory value )
    {
    }


    void File::remove( String key )
    {
    }


    void File::write( Memory line )
    {
    }
}
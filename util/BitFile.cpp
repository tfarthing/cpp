#include "../../cpp/file/SyncFile.h"
#include "../../cpp/util/BitFile.h"
#include "../../cpp/process/Thread.h"


namespace cpp::bit
{
    File::File( )
    {

    }


    File::File( FilePath filename, Handler handler, bool shareWrite )
        : m_filename{ std::move( filename ) }, m_handler{ std::move( handler ) }, m_shareWrite( shareWrite )
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


    void File::load( FilePath filename, Handler handler, bool shareWrite )
    {
        m_filename = std::move( filename );
        m_handler = std::move( handler );
        m_shareWrite = shareWrite;
        m_data.reset( );

        reload( );
    }


    void File::reload( )
    {
        check<IOException>( !m_filename.empty(), "bit::File::reload() : no filename specified" );
        
        m_file.close( );
        if ( Files::exists( filename( ) ) )
        {
            for ( int retries = 0; retries < 5; retries++ )
            {
                try
                {
                    m_data.reset( );

                    auto backupFilename = m_filename.append( ".backup" );
                    auto file = SyncFile::create( backupFilename );

                    for ( auto cursor : SyncFile::read( filename( ) ).input( ).lines( ) )
                    {
                        try
                        {
                            auto record = bit::decode( cursor.line );
                            m_data += record;
                        }
                        catch ( cpp::Exception & )
                        {
                            // save bad lines in new file
                            file.write( cursor.line );
                        }
                    }

                    file.write( m_data.encodeRaw( ) );
                    file.close( );

                    auto oldFilename = m_filename.append( ".old" );
                    Files::rename( filename( ), oldFilename );
                    Files::rename( backupFilename, filename( ) );
                    Files::remove( oldFilename );

                    return;
                }
                catch ( IOException & )
                {
                    Thread::sleep( Duration::ofMillis( 200 ) );
                }
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
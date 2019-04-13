#include "../../cpp/file/SyncFile.h"
#include "../../cpp/util/BitFile.h"
#include "../../cpp/process/Thread.h"


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


    const FilePath & File::filename( ) const
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

        reload( );

        assert( m_file.isOpen( ) );
    }


    void File::reload( )
    {
        assert( !m_filename.empty() );
        
        m_file.seek( 0 );
        m_data.reset( );

        auto reloadFilename = m_filename.append( ".reload" );
        auto reloadFile = SyncFile::create( reloadFilename );

        for ( auto cursor : m_file.input( ).lines( ) )
        {
            try
            {
                auto record = bit::decode( cursor.line );
                m_data += record;
            }
            catch ( cpp::Exception & )
            {
                // save bad lines in new file
                reloadFile.write( cursor.line );
            }
        }

        reloadFile.write( m_data.encodeRaw( ) );
        reloadFile.close( );

        m_file.close( );

        auto oldFilename = m_filename.append( ".old" );
        Files::rename( filename( ), oldFilename );
        Files::rename( reloadFilename, filename( ) );
        Files::remove( oldFilename );
    }


    bool File::isOpen( ) const
    {
        return m_file.isOpen( );
    }


    Memory File::get( Memory key ) const
    {
        return m_data[key];
    }


    void File::set( Memory key, Memory value )
    {
        bit::Object data;
        data[key] = value;
        
        m_data += data;

        write( data.encodeRaw( ) );
    }


    void File::remove( Memory key )
    {
        bit::Object data;
        data[key].erase( );

        m_data += data;

        write( data.encodeRaw( ) );
    }


    void File::write( Memory line )
    {
        m_file.write( line );
    }
}
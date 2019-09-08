#include "../../cpp/util/BitFile.h"
#include "../../cpp/process/Thread.h"


namespace cpp::bit
{
    BitFile::BitFile( )
    {

    }


    BitFile::BitFile( FilePath filename, Handler handler )
        : m_filename{ std::move( filename ) }, m_handler{ std::move( handler ) }
    {
        reload( );
    }


    const FilePath & BitFile::filename( ) const
    {
        return m_filename;
    }


    const Object & BitFile::data( ) const
    {
        return m_data;
    }


    void BitFile::load( FilePath filename, Handler handler )
    {
        m_filename = std::move( filename );
        m_handler = std::move( handler );
        m_data.reset( );

        reload( );

        assert( m_file.isOpen( ) );
    }


    void BitFile::reload( )
    {
        assert( !m_filename.isEmpty( ) );
        
        m_file.seek( 0 );
        m_data.reset( );

        auto reloadFilename = m_filename.append( ".reload" );
        auto reloadFile = File::create( reloadFilename );

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


    bool BitFile::isOpen( ) const
    {
        return m_file.isOpen( );
    }


    Memory BitFile::get( Memory key ) const
    {
        return m_data[key];
    }


    void BitFile::set( Memory key, Memory value )
    {
        bit::Object data;
        data[key] = value;
        
        m_data += data;

        write( data.encodeRaw( ) );
    }


    void BitFile::remove( Memory key )
    {
        bit::Object data;
        data[key].erase( );

        m_data += data;

        write( data.encodeRaw( ) );
    }


    void BitFile::write( Memory line )
    {
        m_file.write( line );
    }
}
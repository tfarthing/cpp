#include <cpp/chrono/Timer.h>
#include <cpp/io/file/File.h>

#include <cpp/util/PropertyFile.h>

namespace cpp
{

    const cpp::Duration PropertyFile::defaultLoadTimeout = cpp::Duration::ofMillis( 1000 );

    const cpp::Properties & PropertyFile::properties( ) const
    {
        return m_properties;
    }

    void PropertyFile::load( cpp::FilePath filename, cpp::Duration timeout )
    {
        m_filename = filename;
        load( timeout );
    }

    void PropertyFile::load( cpp::Duration timeout )
    {
        if ( filename().isEmpty() || !File::exists( filename() ) )
            { return; }

        cpp::Timer timer;
        while ( !timer.test( timeout ) )
        {
            try
            {
                auto output = File::write( filename( ) );
                for ( auto cursor : File::read( filename( ) ).lines( ) )
                    { m_properties.addText( cursor.line( ) ); }

                const Properties::Map & map = m_properties.getMap( );
                for ( const auto & entry : map )
                    { output.print( entry.first + "\t" + entry.second + "\n" ); }
                
                output.truncate( );
                return;
            }
            catch ( IOException & )
            {
                Thread::sleep( Duration::ofMillis( 200 ) );
            }
        }
        throw cpp::TimeoutException{ };
    }

    bool PropertyFile::has( const cpp::Memory & key ) const
    {
        return m_properties.has( key );
    }

    cpp::Memory PropertyFile::get( const cpp::Memory & key ) const
    {
        return m_properties.get( key );
    }

    bool PropertyFile::set( cpp::String key, cpp::String value )
    {
        m_properties.set( key, value );
        return write( String::format( "%\t%\n", key, value ) );
    }

    bool PropertyFile::remove( const cpp::Memory & key )
    {
        m_properties.remove( key );
        return write( String::format( "[remove]\t%\n", key ) );
    }

    bool PropertyFile::removeAll( const cpp::Memory & key )
    {
        bool result = false;
        Properties all = m_properties.extract( key );
        for ( auto itr : all.getMap( ) )
        {
            remove( itr.first );
            result = true;
        }
        return result;
    }

    bool PropertyFile::write( const cpp::Memory & line )
    {
        if ( m_filename.isEmpty() )
            { return false; }

        for ( int retries = 0; retries < 3; retries++ )
        {
            try
            {
                auto output = File::append( filename() );
                output.put( line );
                return true;
            }
            catch ( IOException & )
            {
                Thread::sleep( Duration::ofMillis( 200 ) );
            }
        }
        return false;
    }

}
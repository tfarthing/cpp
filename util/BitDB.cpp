/*
#include "../../cpp/data/Integer.h"
#include "../../cpp/time/Duration.h"

#include "BitDB.h"

BitDB::BitDB( )
    : m_archiveIndex( 0 )
{

}

BitDB::~BitDB( )
{
    close( );
}

cpp::String BitDB::tableRegexHelper( cpp::Memory tableKey )
{
    cpp::String tableRegex = tableKey;
    tableRegex.replaceAll( ".", "\\." );
    tableRegex.replaceAll( "[*]", "\\[([^\\]]*)\\]" );
    return tableRegex;
}

void BitDB::addTable( cpp::Memory tableRegex, ReadHandler readHandle )
{
    m_tables[tableRegex] = std::move(readHandle);
}

void BitDB::removeTable( cpp::Memory tableRegex )
{
    m_tables.erase( tableRegex );
}

//
//filename.ext
//> filename.[archive-date].[archive-index].ext
//> filename.2017-10-07.000.ext
//

void BitDB::open( cpp::FilePath filename, LoadHandler handler, FlushHandler flushHandler )
{
    m_filename = filename;

    auto fileHistory = scanHistory( m_filename );
    bool hasExistingData = !fileHistory.empty( );

    cpp::String lastArchiveName;
    int lastArchiveIndex;

    if ( hasExistingData )
    {
        auto lastFile = *fileHistory.rbegin( );

        auto filenameElements = lastFile.filename( ).split( "." );
        lastArchiveName = filenameElements.at( 1 );
        lastArchiveIndex = (int)cpp::Integer::parse( filenameElements.at( 2 ) );

        //  determine archiveName from date and set index from previous archive
        m_archiveName = cpp::Date::fromTime( cpp::DateTime::now( ) ).toString( "%F" );
        m_archiveIndex = ( lastArchiveName == m_archiveName ) ? lastArchiveIndex + 1 : 0;
    }
    else
    {
        //  determine archiveName from date
        m_archiveName = cpp::Date::fromTime( cpp::DateTime::now( ) ).toString( "%F" );
        m_archiveIndex = 0;
    }

    auto archiveFilename = getArchiveFilename( m_archiveName, m_archiveIndex );
    cpp::check( cpp::File::exists( archiveFilename ) == false, "BitDB::open() : Archive file already exists." );

    //  load the latest archive into the tables
    if ( hasExistingData )
    {
        cpp::FilePath tempFile = m_filename.parent( ) + cpp::String::format( "%-temp.%", m_filename.name(), m_filename.extension() );

        m_output = cpp::File::write( tempFile );
        loadArchive( lastArchiveName, lastArchiveIndex );

        //  flush all tables
        flushHandler( );

        m_output.close( );
        cpp::File::rename( tempFile, archiveFilename );
    }

    m_output = cpp::File::append( archiveFilename );

}

void BitDB::close( )
{
    m_output.close( );
}

void BitDB::put( const BitData::Record & bitRecord )
{
    m_output.put( BitData::encode( bitRecord ) );
}

void BitDB::put( cpp::Memory bitRecord )
{
    m_output.put( bitRecord );
    if ( bitRecord.isEmpty( ) || bitRecord[bitRecord.length( ) - 1] != '\n' )
    {
        m_output.put( "\n" );
    }
}

//  Returns all files in the same directory as filename, and which share the same name (excluding extension) as filename
std::set<cpp::FilePath> BitDB::scanHistory( cpp::FilePath filename )
{
    auto parent = filename.parent( );
    auto name = filename.name( );

    std::set<cpp::FilePath> history;
    auto fileList = cpp::File::list( filename.parent( ) );
    for ( auto & file : fileList )
    {
        if ( file.parent() == parent && file.name( ) == name )
        {
            history.insert( file );
        }
    }
    return history;
}

//  Returns all new filename with the extension modified to reflect archiveName, archiveIndex, and fileIndex
//      archiveName: i.e. date of file creation, e.g. "2017-10-07"
//      archiveIndex: auto-incremented index of archive
cpp::FilePath BitDB::getArchiveFilename( cpp::String archiveName, int archiveIndex )
{
    cpp::String archiveFilename = cpp::String::format( "%.%.%.%",
        m_filename.name( ),
        archiveName,
        cpp::Integer::toDecimal( archiveIndex, 3, 0, true ),
        m_filename.extension( ) );
    return m_filename.parent( ) + archiveFilename;
}

void BitDB::loadArchive( cpp::String archiveName, int archiveIndex )
{
    cpp::FilePath archiveFile = getArchiveFilename( archiveName, archiveIndex );

    if ( cpp::File::exists( archiveFile ) )
    {
        for ( auto & cursor : cpp::File::read( archiveFile ).lines( ) )
        {
            try
            {
                auto record = BitData::decode( cursor.line( ) );

                ReadHandler * handler = nullptr;
                cpp::String tableKey;
                cpp::Memory::Match keyMatch;

                for ( auto & itr : m_tables )
                {
                    tableKey = itr.first;
                    keyMatch = record.key.match( tableKey );
                    if ( keyMatch )
                        { handler = &(itr.second); break; }
                }
                if ( !handler || !(*handler)(tableKey, keyMatch, record) )
                {
                    // write lines that are not handled
                    put( cursor.line( ) );
                }

            }
            catch ( cpp::Exception & e )
            {
                cpp::debug( "BitDB::loadArchive() : error handling record : error='%', line='%'", e.what( ), cursor.line( ) );
                // write lines that cause an error
                put( cursor.line( ) );
            }
        }
    }
}

*/

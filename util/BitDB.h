#pragma once

/*
#include <set>
#include <map>

#include "../../cpp/data/Memory.h"
#include "../../cpp/file/Files.h"

#include "BitData.h"

class BitDB
{
public:
    BitDB( );
    ~BitDB( );

    typedef std::function<void( float32, std::error_code )> LoadHandler;
    typedef std::function<bool( cpp::Memory key, cpp::Memory::Match & keyMatch, const BitData::DecodedRecord & bitRecord )> ReadHandler;
    typedef std::function<void( )> FlushHandler;

    static cpp::String tableRegexHelper( cpp::Memory tableKey );
    void addTable( cpp::Memory tableRegex, ReadHandler readHandle );
    void removeTable( cpp::Memory tableRegex );

    void open( cpp::FilePath filename, LoadHandler loadHandler, FlushHandler flushHandler );
    void close( );

    void put( const BitData::Record & bitRecord );
    void put( cpp::Memory bitRecord );

private:
    static std::set<cpp::FilePath> scanHistory( cpp::FilePath filename );
    cpp::FilePath getArchiveFilename( cpp::String archiveName, int archiveIndex );
    void loadArchive( cpp::String setName, int setIndex );

private:
    std::map<cpp::String, ReadHandler> m_tables;
    cpp::FilePath m_filename;
    cpp::String m_archiveName;
    int m_archiveIndex;
    cpp::File::Output m_output;
};
*/
#pragma once

#include <cpp/chrono/Time.h>
#include <cpp/io/file/FilePath.h>
#include <cpp/util/DataMap.h>
#include <cpp/util/BitData.h>

namespace cpp
{

    /*
    PropertyFile is a utility class for persistence of properties.  The file is not
    kept open after read/write operations.  All writes append to the end of the file.
    The load reads all entries, with newest entries overwriting older entries.  After
    loading the file is truncated.

    Multiple processes can load and write to the property file.  The load is a snapshot
    of the properties, and properties are not updated implicitly from external writes.
    The last value specified for a key is what will subsequently be read.
    */
    class DataFile
    {
    public:
        DataFile( );
        DataFile( FilePath filename );

        typedef std::map<cpp::String, StringMap> map_t;

        FilePath filename( );
        const map_t & data( ) const;

        void load( FilePath filename );
        void reload( );

        bool isOpen( ) const;

        Memory get( Memory key ) const;
        uint64_t getUint( Memory key ) const;
        uint64_t getHex( Memory key ) const;
        int64_t getInt( Memory key ) const;
        f64_t getFloat( Memory key ) const;
        bool getBool( Memory key ) const;
        DateTime getTime( Memory key ) const;

        void set( Memory key, Memory value );
        void setUint( Memory key, uint64_t value );
        void setHex( Memory key, uint64_t value );
        void setInt( Memory key, int64_t value );
        void setFloat( Memory key, f64_t value );
        void setBool( Memory key, bool value );
        void setTime( Memory key, DateTime value );

        void remove( String key );

        void setRecord( Memory recordKey, const DataMap & data );
        DataMap getRecord( Memory recordKey ) const;
        void removeRecord( Memory recordKey );

    private:
        void write( Memory line );

    private:
        FilePath m_filename;
        RecordMap m_data;
    };

    inline FilePath DataFile::filename( )
        { return m_filename; }

    inline const DataFile::map_t & DataFile::data( ) const
        { return m_data; }

}
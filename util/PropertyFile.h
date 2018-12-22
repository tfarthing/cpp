#pragma once

#include <cpp/chrono/Duration.h>
#include <cpp/io/file/FilePath.h>
#include <cpp/util/Properties.h>

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
    class PropertyFile
    {
    public:
        PropertyFile( )
            { }
        PropertyFile( cpp::FilePath filename, cpp::Duration timeout = defaultLoadTimeout )
            : m_filename( filename ) { load( timeout ); }
        
        cpp::FilePath filename()
            { return m_filename; }
        
        const cpp::Properties & properties( ) const;

        void load( cpp::Duration timeout = defaultLoadTimeout );
        void load( cpp::FilePath filename, cpp::Duration timeout = defaultLoadTimeout );

        bool has( const cpp::Memory & key ) const;
        cpp::Memory get( const cpp::Memory & key ) const;
        bool set( cpp::String key, cpp::String value );
        bool remove( const cpp::Memory & key );
        bool removeAll ( const cpp::Memory & key );

    private:
        bool write( const cpp::Memory & line );

    private:
        cpp::FilePath m_filename;
        cpp::Properties m_properties;

        static const cpp::Duration defaultLoadTimeout;
    };

}
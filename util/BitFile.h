#pragma once

#include <functional>
#include "../../cpp/file/Files.h"
#include "Bit.h"

namespace cpp::bit
{
    class File
    {
    public:
        typedef std::function<void( Object )> Handler;

                                File( );
                                File( FilePath filename, Handler handler = nullptr, bool shareWrite = false );

        FilePath                filename( );
        const Object &          data( ) const;

        void                    load( FilePath filename, Handler handler = nullptr, bool shareWrite = false );
        void                    reload( );

        bool                    isOpen( ) const;
        
        Memory                  get( Memory key ) const;
        void                    set( Memory key, Memory value );
        void                    remove( String key );

    private:
        void                    write( Memory line );

    private:
        FilePath                m_filename;
        Handler                 m_handler;
        bool                    m_shareWrite;
        SyncFile                m_file;
        Object                  m_data;
    };
}
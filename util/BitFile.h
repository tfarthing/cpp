#pragma once

#include <functional>
#include "../../cpp/file/File.h"
#include "Bit.h"

namespace cpp::bit
{
    class BitFile
    {
    public:
        typedef std::function<void( Object )> Handler;

                                            BitFile( );
                                            BitFile( FilePath filename, Handler handler = nullptr );

        const FilePath &                    filename( ) const;
        const Object &                      data( ) const;

        void                                load( FilePath filename, Handler handler = nullptr );
        void                                reload( );

        bool                                isOpen( ) const;
        
        Memory                              get( Memory key ) const;
        void                                set( Memory key, Memory value );
        void                                remove( Memory key );

    private:
        void                                write( Memory line );

    private:
        FilePath                            m_filename;
        Handler                             m_handler;
        File                                m_file;
        Object                              m_data;
    };
}
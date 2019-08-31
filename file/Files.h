#pragma once

#include <filesystem>

#include "../../cpp/data/String.h"

namespace cpp
{

    struct FilePath;

    struct File
    {
        enum class                          Access               
                                                { Create, Write, Read };
        enum class                          Share                
                                                { AllowNone, AllowRead, AllowWrite, AllowAll };
    };

    struct Files
    {
        static bool exists( const FilePath & path );
        static bool createDirectory( const FilePath & path );
        static bool createDirectories( const FilePath & path );
        static bool remove( const FilePath & path );
        static bool removeAll( const FilePath & path );
        static void rename( const FilePath & oldPath, const FilePath & newPath );
    };

}
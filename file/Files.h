#pragma once

#include <filesystem>

namespace cpp
{

    struct FilePath;

    struct FileBase
    {
        enum class                          Access               
                                                { Create, Write, Read };
        enum class                          Share                
                                                { None, Read, Write, All };
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
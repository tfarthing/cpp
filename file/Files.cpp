#include "../../cpp/file/Files.h"
#include "../../cpp/file/FilePath.h"



namespace cpp
{

    bool Files::exists( const FilePath & path )
        { return std::filesystem::exists( path.to_path( ) ); }


    bool Files::createDirectory( const FilePath & path )
        { return std::filesystem::create_directory( path.to_path( ) ); }


    bool Files::createDirectories( const FilePath & path )
        { return std::filesystem::create_directories( path.to_path( ) ); }


    bool Files::remove( const FilePath & path )
        { return std::filesystem::remove( path.to_path( ) ); }


    bool Files::removeAll( const FilePath & path )
        { return std::filesystem::remove_all( path.to_path( ) ); }


    void Files::rename( const FilePath & oldPath, const FilePath & newPath )
        { return std::filesystem::rename( oldPath.to_path( ), newPath.to_path( ) ); }

}
#pragma once

#include <memory>
#include <cpp/chrono/DateTime.h>
#include <cpp/io/file/FilePath.h>

namespace cpp
{
    class FileInfo
    {
    public:
        FileInfo( const FilePath & filePath );

        bool exists( ) const;
        size_t size( ) const;

        bool isDirectory( ) const;
        bool isReadOnly( ) const;
        bool isHidden( ) const;

        cpp::DateTime createTime( ) const;
        cpp::DateTime writeTime( ) const;
        cpp::DateTime accessTime( ) const;

    private:
        struct Detail;
        std::shared_ptr<Detail> m_detail;
    };
}
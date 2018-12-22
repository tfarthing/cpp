#ifndef TEST

#include <cpp/Platform.h>
#include <cpp/io/file/FileInfo.h>
#include <cpp/util/Utf16.h>

namespace cpp
{

    struct FileInfo::Detail
    {
        FilePath m_filepath;

        bool m_exists = false;
        bool m_isAccessible = false;
        size_t m_size = 0;

        bool m_isDirectory = false;
        bool m_isReadOnly = false;
        bool m_isHidden = false;

        cpp::DateTime m_createTime;
        cpp::DateTime m_writeTime;
        cpp::DateTime m_accessTime;
    };

    cpp::DateTime toTime( FILETIME & filetime )
    {
        ULARGE_INTEGER ull;
        ull.LowPart = filetime.dwLowDateTime;
        ull.HighPart = filetime.dwHighDateTime;
        return cpp::DateTime{ (time_t)(ull.QuadPart / 10000000ULL - 11644473600ULL) };
    }

    FileInfo::FileInfo( const FilePath & filePath )
        : m_detail( std::make_shared<Detail>() )
    {
        m_detail->m_filepath = filePath;

        WIN32_FILE_ATTRIBUTE_DATA data;
        cpp::Utf16::Text filename = toUtf16( filePath.toString( ) );
        if ( GetFileAttributesEx( filename, GetFileExInfoStandard, &data ) )
        {
            m_detail->m_exists = true;
            m_detail->m_isAccessible = true;
            m_detail->m_size = (uint64_t)(data.nFileSizeHigh) << 32 | data.nFileSizeLow;
            m_detail->m_isDirectory = ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
            m_detail->m_isReadOnly = ( data.dwFileAttributes & FILE_ATTRIBUTE_READONLY ) != 0;
            m_detail->m_isHidden = ( data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) != 0;
            m_detail->m_createTime = toTime( data.ftCreationTime );
            m_detail->m_writeTime = toTime( data.ftLastWriteTime );
            m_detail->m_accessTime = toTime( data.ftLastAccessTime );
        }
        else
        {
            DWORD err = GetLastError( );
            switch (err)
            {
            case ERROR_ACCESS_DENIED:
                m_detail->m_exists = true;
                break;
            case ERROR_FILE_NOT_FOUND:
            default:
                break;
            }
        }
    }

    bool FileInfo::exists( ) const
        { return m_detail->m_exists; }
    size_t FileInfo::size( ) const
        { return m_detail->m_size; }

    bool FileInfo::isDirectory( ) const
        { return m_detail->m_isDirectory; }
    bool FileInfo::isReadOnly( ) const
        { return m_detail->m_isReadOnly; }
    bool FileInfo::isHidden( ) const
        { return m_detail->m_isHidden; }

    cpp::DateTime FileInfo::createTime( ) const
        { return m_detail->m_createTime; }
    cpp::DateTime FileInfo::writeTime( ) const
        { return m_detail->m_writeTime; }
    cpp::DateTime FileInfo::accessTime( ) const
        { return m_detail->m_accessTime; }


}

#else

#include <cpp/chrono/Date.h>
#include <cpp/io/file/File.h>
#include <cpp/io/file/FileInfo.h>
#include <cpp/meta/Unittest.h>

SUITE( FileInfo )
{
    using namespace cpp;

    TEST( ctor )
    {
        FileInfo info{ R"(C:\Users\Tom\Desktop\Rick\blah.txt)" };

        String created = cpp::Date::fromTime( info.createTime( ) ).toString( );
        String writed = cpp::Date::fromTime( info.writeTime( ) ).toString( );
        String accessed = cpp::Date::fromTime( info.accessTime( ) ).toString( );

    }
}

#endif

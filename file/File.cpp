#ifndef TEST

#include <cpp/Platform.h>
#include <cpp/chrono/Timer.h>
#include <cpp/io/AsyncIO.h>
#include <cpp/io/file/File.h>
#include <cpp/util/DataBuffer.h>
#include <cpp/util/Utf16.h>
#include <cpp/platform/windows/WindowsException.h>
#include <cpp/platform/windows/WindowsShell.h>

#define ASIO_STANDALONE
#define ASIO_HAS_STD_CHRONO
#include <lib/asio/asio.hpp>


namespace cpp
{
    namespace File
    {

        Result getWinResult( bool isSuccess )
        {
            if ( !isSuccess )
            {
                int err = GetLastError( );
                switch ( err )
                {
                case ERROR_SHARING_VIOLATION:               return Result::Access;
                case ERROR_ACCESS_DENIED:                   return Result::Access;
                case ERROR_FILE_NOT_FOUND:                  return Result::NoEntry;
                case ERROR_PATH_NOT_FOUND:                  return Result::NoEntry;
                case ERROR_ALREADY_EXISTS:                  return Result::Exists;
                default:                                    return Result::Unknown;
                }
            }
            return Result::Success;
        }

        Result getStdResult( bool isSuccess )
        {
            if ( !isSuccess )
            {
                switch ( errno )
                {
                case EACCES:                return Result::Access;
                case ENOENT:                return Result::NoEntry;
                case EINVAL:                return Result::Invalid;
                case EEXIST:                return Result::Exists;
                case ENOTEMPTY:             return Result::NotEmpty;
                default:                    return Result::Unknown;
                }
            }
            return Result::Success;
        }

        class Handle::Detail
        {
        public:
            Detail( const FilePath & filepath, Access access, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize );
            ~Detail( );

            void doRecvDelay( );
            void doRecv( );
            void doSend( );

            bool isOpen( ) const;
            bool isEOF( ) const;
            Memory read( const Memory & dst, Duration timeout );
            Memory write( const Memory & src );
            void flush( );
            void close( );

            size_t readTell( ) const;
            void readSeek( size_t pos );
            size_t writeTell( ) const;
            void writeSeek( size_t pos );
            void truncate( );

        private:
            using SendItem = std::pair<uint64_t, String>;

            AsyncIO m_io;
            asio::windows::random_access_handle m_handle;
            AsyncTimer m_delayTimer;
            bool m_autoCloseFlag;
            //uint64_t m_autoCloseBytes = 0;
            std::error_code m_error;
            DataBuffer m_recvBuffer;
            std::list<SendItem> m_sendBuffers;
            uint64_t m_sendBytes;

            bool m_isEOF;
            bool m_isRecving;
            bool m_breakRead;
            uint64_t m_recvOffset;    //  file pos of next file input operation
            uint64_t m_readOffset;    //  read pos of next buffer read operation
            bool m_isSending;
            uint64_t m_writeOffset;   //  file pos of next write operation
        };

        Handle::Detail::Detail( const FilePath & filepath, Access access, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize )
            : m_handle(io.get()), m_io( io ), m_autoCloseFlag(false), /*m_autoCloseBytes( 0 ),*/ m_recvBuffer( recvBufferSize ), m_sendBytes( 0 ), m_isEOF(false), m_isRecving( false ), m_breakRead( false ), m_recvOffset(0), m_readOffset( 0 ), m_isSending( false ), m_writeOffset( 0 )
        {
            Utf16::Text filename = toUtf16( filepath.toString() );
            DWORD accessMode = 0;
            DWORD creationMode = 0;
            DWORD shareMode = 0;

            switch ( access )
            {
            case Access::Create:
                creationMode = CREATE_ALWAYS;
                accessMode = GENERIC_WRITE;
                break;
            case Access::Write:
                creationMode = OPEN_ALWAYS;
                accessMode = GENERIC_WRITE;
                break;
            case Access::Read:
            case Access::Tail:
                creationMode = OPEN_EXISTING;
                accessMode = GENERIC_READ;
                break;
            }

            switch ( share )
            {
            case Share::AllowAll:
                shareMode |= FILE_SHARE_DELETE;
            case Share::AllowWrite:
                shareMode |= FILE_SHARE_WRITE;
            case Share::AllowRead:
                shareMode |= FILE_SHARE_READ;
            default:
                break;
            }

            if ( access == Access::Create || access == Access::Write )
            {
                makePath( filepath.parent( ) );
            }

            HANDLE fileHandle = CreateFile( filename.begin( ), accessMode, shareMode, 0, creationMode, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0 );
            if ( fileHandle == INVALID_HANDLE_VALUE )
            {
                DWORD err = GetLastError( );
                throw IOException( String::format( "Unable to open file: error( % )", (uint32_t)err ) );
            }
            m_handle.assign( fileHandle );
                
            BY_HANDLE_FILE_INFORMATION fileInfo;
            GetFileInformationByHandle( m_handle.native_handle(), &fileInfo );
            size_t filesize = ( (uint64_t)fileInfo.nFileSizeHigh << 32 ) | fileInfo.nFileSizeLow;

            if ( origin == npos )
                { origin = filesize; }

            if ( access == Access::Read || access == Access::Tail )
            { 
                /*
                m_autoCloseBytes = filesize;
                */
                if ( access == Access::Read )
                    { m_autoCloseFlag = true; }
                m_recvOffset = origin;
                m_readOffset = origin;
                doRecv( );
            }
            else
            {
                m_writeOffset = origin;
            }
        }

        Handle::Detail::~Detail( )
        {
            close( );
        }

        void Handle::Detail::doRecvDelay( )
        {
            m_isEOF = true;
            m_isRecving = true;
            m_delayTimer = m_io.asyncWait( cpp::Duration::ofMillis( 50 ), [=]( const std::error_code & error )
                {
                    m_isRecving = false; 
                    if ( !error )
                        { doRecv( ); }
                } );
        }

        void Handle::Detail::doRecv( )
        {
            Memory dst = m_recvBuffer.putable( );
            if ( !dst || m_breakRead )
                { return; }

            m_isRecving = true;
            m_handle.async_read_some_at( m_recvOffset, asio::buffer( (void *)dst.begin( ), dst.length( ) ), [=]( const std::error_code & error, std::size_t bytes )
            {
                m_isRecving = false;
                if ( error )
                {
                    if ( error.value() == asio::error::operation_aborted )
                        { /* do nothing */ }
                    else if ( error.value( ) == asio::error::eof )
                        { doRecvDelay( ); }
                    else
                        { m_error = error; close( ); }
                }
                else
                {
                    if ( m_isEOF )
                        { m_isEOF = false; }
                    m_recvOffset += bytes;
                    m_recvBuffer.put( bytes );

                    doRecv( );
                }
            } );
        }

        void Handle::Detail::doSend( )
        {
            SendItem & sendItem = m_sendBuffers.front( );
            Memory src = Memory{ sendItem.second }.substr( (size_t)m_sendBytes );
            if ( !src )
                { return; }

            uint64_t pos = sendItem.first;
            m_isSending = true;
            m_handle.async_write_some_at( pos, asio::buffer( src.begin( ), src.length( ) ), [=]( const std::error_code & error, std::size_t bytes )
            {
                m_isSending = false;
                if ( error )
                {
                    m_error = error;
                    m_sendBuffers.clear( );
                    m_sendBytes = 0;
                    close( );
                }
                else
                {
                    m_sendBytes += bytes;
                    if ( m_sendBytes == m_sendBuffers.front( ).second.size( ) )
                        { m_sendBuffers.pop_front( ); m_sendBytes = 0; }
                    if ( !m_sendBuffers.empty( ) )
                        { doSend( ); }
                }
            } );
        }


        bool Handle::Detail::isOpen( ) const
        {
            if ( m_recvBuffer.getable( ) )
                { return true; }
            if ( !m_handle.is_open() )
                { return false; }
            if ( m_autoCloseFlag && m_isEOF )
                { return false; }
            return true;
        }

        bool Handle::Detail::isEOF( ) const
        {
            return m_isEOF;
        }

        Memory Handle::Detail::read( const Memory & dst, Duration timeout )
        {
            Timer timer;
            while ( isOpen( ) )
            {
                //  wait if the read should block
                if ( m_recvBuffer.getable( ).isEmpty( ) )
                {
                    if ( !m_isRecving )
                        { doRecv( ); }

                    if ( timer.test( timeout ) )
                        { break; }

                    if ( timeout.isInfinite() /*|| m_recvOffset < m_autoCloseBytes*/ )
                        { m_io.runOne( ); }
                    else
                        { m_io.runOne( timer.until( timeout ) ); }

                    continue;
                }
                if ( m_error )
                {
                    auto msg = String::format( "Unable to read file: error( % )", m_error.message( ) );
                    throw cpp::IOException( msg );
                }

                size_t len = std::min( dst.length( ), m_recvBuffer.getable( ).length( ) );
                m_readOffset += len;
                return Memory::copy( dst, m_recvBuffer.get( len ) );
            }
            return Memory::Null;
        }
        Memory Handle::Detail::write( const Memory & src )
        {
            if (src.isEmpty())
                { return src; }
            if ( m_sendBuffers.size() > 1 )
                { m_sendBuffers.back( ).second += src; }
            else
                { m_sendBuffers.emplace_back( SendItem{ m_writeOffset, src } ); }
            m_writeOffset += src.length( );
            if ( !m_isSending )
                { doSend( ); }
            m_io.poll( );

            if ( m_error )
            {
                auto msg = String::format( "Unable to write file: error( % )", m_error.message( ) );
                throw cpp::IOException( msg );
            }

            return src;
        }
        void Handle::Detail::flush( )
        {
            while ( m_handle.is_open() && !m_sendBuffers.empty( ) )
                { m_io.runOne( ); }
        }
        void Handle::Detail::close( )
        {
            flush( );

            m_delayTimer.cancel( );
            if ( m_handle.is_open() )
            {
                m_breakRead = true;
                m_handle.cancel( );
                while ( m_isRecving || m_isSending )
                    { m_io.runOne( ); }
                m_handle.close( ); 
            }

        }
        size_t Handle::Detail::readTell( ) const
        {
            return (size_t)m_readOffset;
        }
        void Handle::Detail::readSeek( size_t pos )
        {
            // break pending reads
            m_breakRead = true;
            m_delayTimer.cancel( );
            m_handle.cancel( );
            while ( m_isRecving )
                { m_io.runOne( ); }
            m_breakRead = false;

            //  clear read buffer
            m_recvBuffer.clear( );

            //  set new read position
            m_isEOF = false;
            m_recvOffset = pos;
            m_readOffset = pos;

            doRecv( );
        }
        size_t Handle::Detail::writeTell( ) const
        {
            return (size_t)m_writeOffset;
        }
        void Handle::Detail::writeSeek( size_t pos )
        {
            flush( );
            m_writeOffset = pos;
        }

        void Handle::Detail::truncate( )
        {
            _LARGE_INTEGER pos;
            pos.QuadPart = m_writeOffset;
            check<IOException>( SetFilePointerEx( m_handle.native(), pos, NULL, FILE_BEGIN ) != FALSE, "Unable to truncate file (SetFilePointerEx)" );
            check<IOException>( SetEndOfFile( m_handle.native( ) ) != FALSE, "Unable to truncate file (SetEndOfFile)" );
        }


        Handle::Handle( nullptr_t )
            : m_detail( nullptr ) { }

        Handle::Handle( )
            : m_detail( nullptr ) { }

        Handle::Handle(
                const FilePath & filepath,
                Access access,
                size_t origin,
                Share share,
                AsyncIO & io,
                size_t buflen )
            : m_detail( std::make_shared<Detail>( filepath, access, origin, share, io, buflen ) ) { }

        Handle::~Handle( )
            { m_detail->close(); }

        bool Handle::isOpen( ) const
            { return m_detail->isOpen(); }
        Memory Handle::read( const Memory & dst, Duration timeout )
            { return m_detail->read(dst, timeout); }
        Memory Handle::write( const Memory & src )
            { return m_detail->write(src); }
        void Handle::flush( )
            { m_detail->flush(); }
        void Handle::close( )
            { return m_detail->close(); }

        bool Handle::isEOF( ) const
            { return m_detail->isEOF( ); }
        size_t Handle::readTell( ) const
            { return m_detail->readTell(); }
        void Handle::readSeek( size_t pos )
            { return m_detail->readSeek( pos ); }
        size_t Handle::writeTell( ) const
            { return m_detail->writeTell(); }
        void Handle::writeSeek( size_t pos )
            { return m_detail->writeSeek( pos ); }
        void Handle::truncate( )
            { return m_detail->truncate(); }


        Input::Input( )
            : cpp::Input( ) { }
        Input::Input( const FilePath & filepath, bool autoClose, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize )
            : cpp::Input( std::make_shared<Handle>( filepath, autoClose ? Handle::Access::Read : Handle::Access::Tail, origin, share, io, recvBufferSize ) ) { }
        bool Input::isEOF( ) const
            { return std::dynamic_pointer_cast<Handle>( m_source )->isEOF( ); }
        uint64_t Input::tell( ) const
            { return std::dynamic_pointer_cast<Handle>(m_source)->readTell(); }
        void Input::seek( uint64_t pos )
            { return std::dynamic_pointer_cast<Handle>(m_source)->readSeek( (size_t)pos ); }

        Output::Output( )
            : cpp::Output( ) { }
        Output::Output( const FilePath & filepath, bool overwrite, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize )
            : cpp::Output( std::make_shared<Handle>( filepath, overwrite ? Handle::Access::Create : Handle::Access::Write, origin, share, io, recvBufferSize ) ) { }
        uint64_t Output::tell( ) const
            { return std::dynamic_pointer_cast<Handle>( m_sink )->writeTell(); }
        void Output::seek( uint64_t pos )
            { std::dynamic_pointer_cast<Handle>( m_sink )->writeSeek( (size_t)pos ); }
        void Output::truncate( )
            { std::dynamic_pointer_cast<Handle>( m_sink )->flush(); std::dynamic_pointer_cast<Handle>( m_sink )->truncate(); }


        FileInfo info( const FilePath & filepath )
        {
            return FileInfo{ filepath };
        }

        bool exists( const FilePath & filepath )
        {
            return info( filepath ).exists( );
        }

        size_t size( const FilePath & filepath )
        {
            return info( filepath ).size( );
        }

        Output create( const FilePath & filepath, Share share, AsyncIO & io )
        {
            return Output{ filepath, true, 0, share, io };
        }

        Output write( const FilePath & filepath, size_t origin, Share share, AsyncIO & io )
        {
            return Output{ filepath, false, origin, share, io };
        }

        Output append( const FilePath & filepath, Share share, AsyncIO & io )
        {
            return Output{ filepath, false, npos, share, io };
        }

        Input read( const FilePath & filepath, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize )
        {
            return Input{ filepath, true, origin, share, io };
        }

        Input tail( const FilePath & filepath, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize )
        {
            return Input{ filepath, false, origin, share, io };
        }

        Result remove( const FilePath & filepath )
        {
            std::wstring wpath = toUtf16( filepath.toString( '\\' ) ); wpath.append( 2, L'\0' );  // double null termination

            if ( info(filepath).isDirectory() ) 
            {
                SHFILEOPSTRUCT fileop{ 0 };
                fileop.wFunc = FO_DELETE;
                fileop.pFrom = wpath.c_str();
                fileop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;

                Result result = getWinResult( SHFileOperation( &fileop ) == 0 );
                return result == Result::NoEntry ? Result::Success : result;
            }
            else
            {
                return getWinResult( DeleteFile( wpath.c_str( ) ) != 0 );
            }

        }

        Result rename( const FilePath & src, const FilePath & dst )
        {
            return getStdResult( ::rename( src.toString( ).c_str( ), dst.toString( ).c_str( ) ) == 0);
        }

        Result move( const FilePath & src, const FilePath & dst )
        {
            std::wstring wsrc = toUtf16( src.toString( '\\' ) ); wsrc.append( 2, L'\0' );
            std::wstring wdst = toUtf16( dst.toString( '\\' ) ); wdst.append( 2, L'\0' );

            SHFILEOPSTRUCT fileop;
            memset( &fileop, 0, sizeof( fileop ) );
            fileop.hwnd = 0;
            fileop.wFunc = FO_MOVE;
            fileop.pFrom = wsrc.c_str();
            fileop.pTo = wdst.c_str();
            fileop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

            return getWinResult( SHFileOperation( &fileop ) == 0 );
        }

        Result copy( const FilePath & src, const FilePath & dst )
        {
            std::wstring wsrc = toUtf16( src.toString( '\\' ) ); wsrc.append( 2, L'\0' );
            std::wstring wdst = toUtf16( dst.toString( '\\' ) ); wdst.append( 2, L'\0' );

            SHFILEOPSTRUCT fileop;
            memset( &fileop, 0, sizeof( fileop ) );
            fileop.hwnd = 0;
            fileop.wFunc = FO_COPY;
            fileop.pFrom = wsrc.c_str();
            fileop.pTo = wdst.c_str();
            fileop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

            return getWinResult( SHFileOperation( &fileop ) == 0 );
        }

        bool isIdentical( const FilePath & lhs, const FilePath & rhs )
        {
            if ( File::size(lhs) != File::size(rhs) )
                { return false; }

            String buffer;
            auto rhsInput = read( rhs );
            for ( auto cursor : File::read( lhs ).bytes( ) )
            {
                buffer.resize( cursor.data( ).length( ) );
                if ( cursor.data( ) != rhsInput.get( buffer ) )
                    { return false; }
            }
            return true;
        }

        Result makePath( const FilePath & path )
        {
            FileInfo info = File::info( path );
            if ( info.exists() )
                { return info.isDirectory( ) ? Result::Success : Result::Exists; }

            auto nodes = path.nodes( );
            if ( nodes.size( ) > 1 && !File::exists( path.parent( ) ) )
            {
                Result result = makePath( path.parent( ) );
                if ( result != Result::Success )
                    { return result; }
            }

            return getWinResult( CreateDirectory( toUtf16( path.toString('\\') ).data(), NULL ) != 0 );
        }

        FilePath::Array list( const FilePath & filepath, bool isRecursive, Memory nameRegex, bool includeDirectories, bool includeFiles )
        {
            FilePath::Array results;

            FilePath fullPattern{ filepath, "*" };
            WIN32_FIND_DATA findData;
            HANDLE findHandle = FindFirstFileEx( 
                cpp::toUtf16( fullPattern.toString( ) ).c_str( ), 
                FindExInfoBasic,
                &findData,
                FindExSearchNameMatch,
                nullptr,
                FIND_FIRST_EX_LARGE_FETCH);

            if ( findHandle != INVALID_HANDLE_VALUE )
            {
                do
                {
                    String filename = toUtf8( findData.cFileName );
                    if ( filename == "." || filename == ".." )
                        { continue; }

                    if ( nameRegex && !filename.match( nameRegex ) )
                        { continue; }
                    
                    if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                    {
                        if ( includeDirectories)
                            { results.push_back( filepath + filename ); }
                        if ( isRecursive )
                        {
                            FilePath::Array subfileList = list( filepath + filename, isRecursive, nameRegex, includeDirectories, includeFiles );
                            results.insert( results.end( ), subfileList.begin( ), subfileList.end( ) );
                        }
                    }
                    else if ( includeFiles )
                    {
                        results.push_back( filepath + filename );
                    }
                } while ( FindNextFile( findHandle, &findData ) );
                FindClose( findHandle );
            }
            else
            {
                cpp::windows::Exception e;
                if ( e.error( ) != ERROR_FILE_NOT_FOUND )
                    { throw e; }
            }

            return results;
        }

        FilePath::Array listDirectories( const FilePath & filepath, bool isRecursive, Memory nameRegex )
        {
            return list( filepath, isRecursive, nameRegex, true, false );
        }

        Result setHidden( const FilePath & filepath, bool value )
        {
            Utf16::Text wfilepath = toUtf16( filepath.toString( '\\' ) );
            DWORD attrs = GetFileAttributes( wfilepath );
            if ( value )
                { attrs |= FILE_ATTRIBUTE_HIDDEN; }
            else
                { attrs &= ~( FILE_ATTRIBUTE_HIDDEN ); }
            return getWinResult( SetFileAttributes( wfilepath, attrs ) != 0 );
        }

        Result setReadOnly( const FilePath & filepath, bool value )
        {
            Utf16::Text wfilepath = toUtf16( filepath.toString( '\\' ) );
            DWORD attrs = GetFileAttributes( wfilepath );
            if ( value )
                { attrs |= FILE_ATTRIBUTE_READONLY; }
            else
                { attrs &= ~(FILE_ATTRIBUTE_READONLY); }
            return getWinResult(SetFileAttributes( wfilepath, attrs) != 0);
        }

        String toString( Result result )
        {
            switch ( result )
            {
            case Result::Success:
                return "Success";
            case Result::Access:
                return "Access";
            case Result::NoEntry:
                return "NoEntry";
            case Result::Invalid:
                return "Invalid";
            case Result::Exists:
                return "Exists";
            case Result::NotEmpty:
                return "NotEmpty";
            case Result::Unknown:
            default:
                return "Unknown";
            }
        }

        String toString( Share share )
        {
            switch ( share )
            {
            case Share::AllowNone:
                return "AllowNone";
            case Share::AllowRead:
                return "AllowRead";
            case Share::AllowWrite:
                return "AllowWrite";
            case Share::AllowAll:
            default:
                return "AllowAll";
            }
        }

    }
}

#else

#pragma warning (disable : 4996)

#include <cpp/Float.h>
#include <cpp/chrono/timer.h>
#include <cpp/io/file/File.h>
#include <cpp/meta/Unittest.h>

SUITE( File )
{
    using namespace cpp;

    TEST( output )
    {
        try
        {
            auto output = File::create( "./test.txt" );
            output.print( "line 1\n" );
            output.print( "line 2\n" );
            output.print( "line 3\n" );
            output.print( "line 4\n" );
            output.close( );

            output = File::append( "./test.txt" );
            output.print( "line 5\n" );
            output.print( "line 6\n" );
            output.close( );

            output = File::write( "./test.txt" );
            output.seek( 7 );
            output.print( "xxxx x\n" );
            output.close( );

            auto lines = File::read( "./test.txt" ).lines( ).getAll( );
            CHECK( lines.size( ) == 6 );
            CHECK( lines[1] == "xxxx x" );            
        }
        catch ( IOException & )
        {
            CHECK( false );
        }
    }

    TEST( write )
    {
        cpp::Timer timer;
        std::string buffer( 1024, 'a' );
        size_t blockCount = 10 * 1000;
        size_t byteCount = blockCount * buffer.size( );

        FILE * file = fopen( "./test.txt", "wb" );
        timer.reset( );
        
        ////
        for ( int i = 0; i < blockCount; i++ )
        {
            fwrite( buffer.c_str( ), 1, 1024, file );
        }
        fclose( file );
        ////

        auto duration1 = timer.get( );
        CHECK( cpp::File::size( "./test.txt" ) == byteCount );

        auto output = cpp::File::write( "./test.txt" );
        timer.reset( );

        ////
        for (int i = 0; i < blockCount; i++ )
        {
            output.write( buffer );
        }
        output.close( );
        ////

        auto duration2 = timer.get( );
        CHECK( cpp::File::size( "./test.txt" ) == byteCount );

        double mbps1 = (double)byteCount / duration1.millis( ) / 1000.0;
        double mbps2 = (double)byteCount / duration2.millis( ) / 1000.0;
        double ratio = mbps2 / mbps1 * 100;
        cpp::log( "File::write : %MBps (%% of fwrite %MBps)", cpp::Float::toString( mbps2, 3 ), cpp::Float::toString( ratio, 1 ), "%", cpp::Float::toString( mbps1, 3 ) );
    }

    TEST( read )
    {
        cpp::Timer timer;
        std::string buffer( 1024, 'b' );
        size_t blockCount = 10 * 1000;
        size_t byteCount = blockCount * buffer.size( );

        CHECK( cpp::File::size( "./test.txt" ) == byteCount );

        FILE * file = fopen( "./test.txt", "rb" );
        timer.reset( );
        
        ////
        for ( int i = 0; i < blockCount; i++ )
        {
            cpp::Timer timer2;
            fread( (char *)buffer.data( ), 1, 1024, file );
        }
        fclose( file );
        ////

        auto duration1 = timer.get( );

        auto output = cpp::File::read( "./test.txt" );
        timer.reset( );

        ////
        for (int i = 0; i < blockCount; i++ )
        {
            cpp::Timer timer2;
            output.read( buffer );
        }
        output.close( );
        ////

        auto duration2 = timer.get( );

        double mbps1 = (double)byteCount / duration1.millis( ) / 1000.0;
        double mbps2 = (double)byteCount / duration2.millis( ) / 1000.0;
        double ratio = mbps2 / mbps1 * 100;
        cpp::log( "File::read : %MBps (%% of fread %MBps)", cpp::Float::toString( mbps2, 3 ), cpp::Float::toString( ratio, 1 ), "%", cpp::Float::toString( mbps1, 3 ) );
    }

    TEST( read_with_timeout )
    {
        cpp::Timer timer;
        std::string buffer( 1024, 'b' );
        size_t blockCount = 10 * 1000;
        size_t byteCount = blockCount * buffer.size( );

        CHECK( cpp::File::size( "./test.txt" ) == byteCount );

        auto output = cpp::File::read( "./test.txt" );
        timer.reset( );
        
        ////
        for ( int i = 0; i < blockCount; i++ )
        {
            cpp::Timer timer2;
            output.read( buffer );
        }
        output.close( );
        ////

        auto duration1 = timer.get( );

        output = cpp::File::read( "./test.txt" );
        timer.reset( );

        ////
        for (int i = 0; i < blockCount; i++ )
        {
            cpp::Timer timer2;
            CHECK( output.read( buffer, cpp::Duration::ofMillis(200) ) );
        }
        output.close( );
        ////

        auto duration2 = timer.get( );

        double mbps1 = (double)byteCount / duration1.millis( ) / 1000.0;
        double mbps2 = (double)byteCount / duration2.millis( ) / 1000.0;
        double ratio = mbps2 / mbps1 * 100;
        cpp::log( "File::read_with_timeout : %MBps (%% of without timeout %MBps)", 
            cpp::Float::toString( mbps2, 3 ), 
            cpp::Float::toString( ratio, 1 ), 
            "%",
            cpp::Float::toString( mbps1, 3 ) );
    }

    TEST( readAndSeek )
    {
        auto output = File::create( "./test.txt" );
        output.print( "hello" );
        output.close( );

        CHECK( File::size("./test.txt") == 5 );

        String buffer( 1024, '\0' );
        auto input = File::read( "./test.txt" );
        input.seek( 1 );
        input.get( Memory{ buffer }.substr(0, 4) );
        input.close( );

        CHECK( buffer.substr( 0, 4 ) == "ello" );
    }
}

#endif

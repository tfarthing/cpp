#ifndef TEST

#include <cassert>

#include "File.h"
#include "../../cpp/process/Platform.h"
#include "../../cpp/process/Exception.h"



namespace cpp
{

    class File::Detail
        : public Input::Source, public Output::Sink
    {
    public:
        static std::shared_ptr<Detail>      create( const FilePath & filepath, Access access, Share share );

                                            Detail( );
                                            ~Detail( ) override;

        bool							    isOpen( ) const override;
        size_t							    length( ) const;

        size_t                              tell( ) const;
        void                                seek( size_t pos );
        void                                seekToEnd( );

        Memory 					            readsome( Memory buffer, std::error_code & errorCode ) override;

        Memory 					            read( Memory buffer );

        Memory                              write( const Memory src, std::error_code & errorCode ) override;
        void							    write( const Memory data );

        void							    truncate( size_t length = 0 );
        void                                flush( ) override;
        void							    close( ) override;

    private:
        HANDLE                              m_handle;
        std::error_code                     m_error;
    };

    
    std::shared_ptr<File::Detail> File::Detail::create( const FilePath & filepath, Access access, Share share )
    {
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
            creationMode = OPEN_EXISTING;
            accessMode = GENERIC_READ;
            break;
        }

        switch ( share )
        {
        case Share::All:
            shareMode |= FILE_SHARE_DELETE;
        case Share::Write:
            shareMode |= FILE_SHARE_WRITE;
        case Share::Read:
            shareMode |= FILE_SHARE_READ;
        default:
            break;
        }

        if ( access == Access::Create || access == Access::Write )
        {
            Files::createDirectories( filepath.parent( ) );
        }

        HANDLE fileHandle = CreateFile( filepath.toWindows( ), accessMode, shareMode, 0, creationMode, FILE_ATTRIBUTE_NORMAL, 0 );
        if ( fileHandle == INVALID_HANDLE_VALUE )
        {
            DWORD err = GetLastError( );
            throw IOException( cpp::format( "Unable to open file: error( % )", (uint32_t)err ) );
        }

        std::shared_ptr<Detail> detail = std::make_shared<Detail>( );
        detail->m_handle = fileHandle;
        return detail;
    }


    File::Detail::Detail( )
        : m_handle( INVALID_HANDLE_VALUE )
    {
    }


    File::Detail::~Detail( )
    {
        close( );
    }


    bool File::Detail::isOpen( ) const
    {
        return m_handle != INVALID_HANDLE_VALUE;
    }


    size_t File::Detail::length( ) const
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        check<IOException>( GetFileSizeEx( m_handle, &bytes ),
            "SyncFile::length() failed" );
        return bytes.QuadPart;
    }


    size_t File::Detail::tell( ) const
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        bytes.QuadPart = 0;

        check<IOException>( SetFilePointerEx( m_handle, bytes, &bytes, FILE_CURRENT ),
            "SyncFile::tell() failed" );
        return bytes.QuadPart;
    }


    void File::Detail::seek( size_t pos )
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        bytes.QuadPart = pos;

        check<IOException>( SetFilePointerEx( m_handle, bytes, NULL, FILE_BEGIN ),
            "SyncFile::seek() failed" );
    }


    void File::Detail::seekToEnd( )
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        bytes.QuadPart = 0;

        check<IOException>( SetFilePointerEx( m_handle, bytes, NULL, FILE_END ),
            "SyncFile::seekToEnd() failed" );
    }


    Memory File::Detail::readsome( Memory buffer, std::error_code & errorCode )
    {
        DWORD bytes = 0;
        if ( m_error )
        { 
            errorCode = m_error; 
        }
        else if ( isOpen( ) )
        {
            bytes = (DWORD)buffer.length( );
            if ( !ReadFile( m_handle, buffer.data( ), bytes, &bytes, NULL ) )
            {
                DWORD error = GetLastError( );
                m_error = std::make_error_code( std::errc::io_error );
                errorCode = m_error;
            }
			if ( bytes == 0 )
			{
				close( );
			}
        }
        return buffer.substr( 0, bytes );
    }


    Memory File::Detail::read( Memory buffer )
    {
        check<Input::Exception>( !m_error, m_error );

        DWORD bytes = 0;
        if ( isOpen( ) )
        {
            bytes = (DWORD)buffer.length( );
            if ( !ReadFile( m_handle, buffer.data( ), bytes, &bytes, NULL ) )
            {
                DWORD error = GetLastError( );
                m_error = std::make_error_code( std::errc::io_error );
                throw Input::Exception{ m_error };
            }
        }
        return buffer.substr( 0, bytes );
    }


    Memory File::Detail::write( const Memory src, std::error_code & errorCode )
    {
        return src;
    }

    
    void File::Detail::write( const Memory data )
    {
        assert( isOpen( ) );
        DWORD bytes = 0;
        check<IOException>( WriteFile( m_handle, data.data( ), (DWORD)data.length(), &bytes, NULL ),
            "SyncFile::read() failed" );
        assert( bytes == data.length( ) );
    }


    void File::Detail::truncate( size_t length )
    {
        assert( isOpen( ) );
        _LARGE_INTEGER pos;
        pos.QuadPart = length;
        check<IOException>( SetFilePointerEx( m_handle, pos, NULL, FILE_BEGIN ) != FALSE,
            "SyncFile::truncate() : SetFilePointerEx() failed" );
        check<IOException>( SetEndOfFile( m_handle ) != FALSE,
            "SyncFile::truncate() : SetEndOfFile() failed" );
    }


    void File::Detail::flush( )
    {
        FlushFileBuffers( m_handle );
    }

    void File::Detail::close( )
    {
        if ( isOpen( ) )
        {
            if ( !m_error )
                { m_error = std::make_error_code( std::errc::connection_aborted ); };
            
            CloseHandle( m_handle );
            m_handle = INVALID_HANDLE_VALUE;
        }
    }



    File File::read( const FilePath & filepath, Share share )
    {
        return File{ filepath, Access::Read, share };
    }


    File File::open( const FilePath & filepath, Share share )
    {
        return File{ filepath, Access::Write, share };
    }


    File File::create( const FilePath & filepath, Share share )
    {
        return File{ filepath, Access::Create, share };
    }


    File File::append( const FilePath & filepath, Share share )
    {
        File result{ filepath, Access::Write, share };
        result.seekToEnd( );
        return result;
    }


    File::File( )
        : m_detail( )
    {

    }


    File::File( const FilePath & filepath, Access access, Share share )
        : m_detail( Detail::create( filepath, access, share ) )
    {
        
    }


    bool File::isOpen( ) const
    {
        return ( m_detail ) ? m_detail->isOpen( ) : false;
    }


    size_t File::length( ) const
    {
        assert( m_detail );
        return m_detail->length( );
    }
    

    size_t File::tell( ) const
    {
        assert( m_detail );
        return m_detail->tell( );
    }


    void File::seek( size_t pos )
    {
        assert( m_detail );
        m_detail->seek( pos );
    }


    void File::seekToEnd( )
    {
        assert( m_detail );
        m_detail->seekToEnd( );
    }


    Memory File::read( Memory buffer )
    {
        assert( m_detail );
        return m_detail->read( buffer );
    }


    void File::write( Memory data )
    {
        assert( m_detail );
        m_detail->write( data );
    }


	void File::flush( )
	{
		if ( m_detail )
			{ m_detail->flush( ); }
	}


    void File::truncate( size_t length )
    {
        assert( m_detail );
        m_detail->truncate( length );
    }


    void File::close( )
    {
		if ( m_detail )
			{ m_detail->close( ); }
    }


    Input File::input( )
    {
        return Input{ m_detail };
    }


    Output File::output( )
    {
        return Output{ m_detail };
    }


}

#else

#include <catch2/catch.hpp>

#include "File.h"

TEST_CASE( "File" )
{
    cpp::FilePath filename = "test.txt";
    auto file = cpp::File::create( filename );
	file.write( "Hello World!\n" );
	file.close( );

    REQUIRE( cpp::Files::exists( filename ) );

	file = cpp::File::read( filename );
	CHECK( file.input( ).readAll( ) == "Hello World!\n" );

    cpp::Files::remove( filename );
}


#endif

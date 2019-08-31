#ifndef TEST

#include <cassert>

#include "SyncFile.h"
#include "../../cpp/process/Platform.h"
#include "../../cpp/process/Exception.h"



namespace cpp
{

    class SyncFile::Detail
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

    
    std::shared_ptr<SyncFile::Detail> SyncFile::Detail::create( const FilePath & filepath, Access access, Share share )
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


    SyncFile::Detail::Detail( )
        : m_handle( INVALID_HANDLE_VALUE )
    {
    }


    SyncFile::Detail::~Detail( )
    {
        close( );
    }


    bool SyncFile::Detail::isOpen( ) const
    {
        return m_handle != INVALID_HANDLE_VALUE;
    }


    size_t SyncFile::Detail::length( ) const
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        check<IOException>( GetFileSizeEx( m_handle, &bytes ),
            "SyncFile::length() failed" );
        return bytes.QuadPart;
    }


    size_t SyncFile::Detail::tell( ) const
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        bytes.QuadPart = 0;

        check<IOException>( SetFilePointerEx( m_handle, bytes, &bytes, FILE_CURRENT ),
            "SyncFile::tell() failed" );
        return bytes.QuadPart;
    }


    void SyncFile::Detail::seek( size_t pos )
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        bytes.QuadPart = pos;

        check<IOException>( SetFilePointerEx( m_handle, bytes, NULL, FILE_BEGIN ),
            "SyncFile::seek() failed" );
    }


    void SyncFile::Detail::seekToEnd( )
    {
        assert( isOpen( ) );

        _LARGE_INTEGER bytes;
        bytes.QuadPart = 0;

        check<IOException>( SetFilePointerEx( m_handle, bytes, NULL, FILE_END ),
            "SyncFile::seekToEnd() failed" );
    }


    Memory SyncFile::Detail::readsome( Memory buffer, std::error_code & errorCode )
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
        }
        return buffer.substr( 0, bytes );
    }


    Memory SyncFile::Detail::read( Memory buffer )
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


    Memory SyncFile::Detail::write( const Memory src, std::error_code & errorCode )
    {
        return src;
    }

    
    void SyncFile::Detail::write( const Memory data )
    {
        assert( isOpen( ) );
        DWORD bytes = 0;
        check<IOException>( WriteFile( m_handle, data.data( ), (DWORD)data.length(), &bytes, NULL ),
            "SyncFile::read() failed" );
        assert( bytes == data.length( ) );
    }


    void SyncFile::Detail::truncate( size_t length )
    {
        assert( isOpen( ) );
        _LARGE_INTEGER pos;
        pos.QuadPart = length;
        check<IOException>( SetFilePointerEx( m_handle, pos, NULL, FILE_BEGIN ) != FALSE,
            "SyncFile::truncate() : SetFilePointerEx() failed" );
        check<IOException>( SetEndOfFile( m_handle ) != FALSE,
            "SyncFile::truncate() : SetEndOfFile() failed" );
    }


    void SyncFile::Detail::flush( )
    {
        FlushFileBuffers( m_handle );
    }

    void SyncFile::Detail::close( )
    {
        if ( isOpen( ) )
        {
            if ( !m_error )
                { m_error = std::make_error_code( std::errc::connection_aborted ); };
            
            CloseHandle( m_handle );
            m_handle = INVALID_HANDLE_VALUE;
        }
    }



    SyncFile SyncFile::read( const FilePath & filepath, Share share )
    {
        return SyncFile{ filepath, Access::Read, share };
    }


    SyncFile SyncFile::open( const FilePath & filepath, Share share )
    {
        return SyncFile{ filepath, Access::Write, share };
    }


    SyncFile SyncFile::create( const FilePath & filepath, Share share )
    {
        return SyncFile{ filepath, Access::Create, share };
    }


    SyncFile SyncFile::append( const FilePath & filepath, Share share )
    {
        SyncFile result{ filepath, Access::Write, share };
        result.seekToEnd( );
        return result;
    }


    SyncFile::SyncFile( )
        : m_detail( )
    {

    }


    SyncFile::SyncFile( const FilePath & filepath, Access access, Share share )
        : m_detail( Detail::create( filepath, access, share ) )
    {
        
    }


    SyncFile::~SyncFile( )
    {
        if (m_detail )
            { close( ); }
    }


    bool SyncFile::isOpen( ) const
    {
        return ( m_detail ) ? m_detail->isOpen( ) : false;
    }


    size_t SyncFile::length( ) const
    {
        assert( m_detail );
        return m_detail->length( );
    }
    

    size_t SyncFile::tell( ) const
    {
        assert( m_detail );
        return m_detail->tell( );
    }


    void SyncFile::seek( size_t pos )
    {
        assert( m_detail );
        m_detail->seek( pos );
    }


    void SyncFile::seekToEnd( )
    {
        assert( m_detail );
        m_detail->seekToEnd( );
    }


    Memory SyncFile::read( Memory buffer )
    {
        assert( m_detail );
        return m_detail->read( buffer );
    }


    void SyncFile::write( Memory data )
    {
        assert( m_detail );
        m_detail->write( data );
    }


	void SyncFile::flush( )
	{
		assert( m_detail );
		m_detail->flush( );
	}


    void SyncFile::truncate( size_t length )
    {
        assert( m_detail );
        m_detail->truncate( length );
    }


    void SyncFile::close( )
    {
        assert( m_detail );
        m_detail->close( );
    }

    Input SyncFile::input( )
    {
        return Input{ m_detail };
    }


}

#else

#include <catch2/catch.hpp>

#include "SyncFile.h"

TEST_CASE( "SyncFile" )
{
    cpp::FilePath filename = "test.txt";
    auto file = cpp::SyncFile::create( (const char *)"test.txt" );

    REQUIRE( cpp::Files::exists( filename ) );

    file.close( );
    cpp::Files::remove( filename );
}


#endif

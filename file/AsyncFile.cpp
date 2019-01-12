#include "../data/DataBuffer.h"
#include "../time/Timer.h"
#include "AsyncFile.h"

namespace cpp
{

	class AsyncFile::Detail
		: public std::enable_shared_from_this<AsyncFile::Detail>
	{
	public:
		std::shared_ptr<Detail> create( const FilePath & filepath, Access access, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize );

		Detail( );
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
		StringBuffer m_recvBuffer;
		std::list<SendItem> m_sendBuffers;
		uint64_t m_sendBytes;

		bool m_isEOF;
		bool m_breakRead;
		uint64_t m_recvOffset;    //  file pos of next file input operation
		uint64_t m_readOffset;    //  read pos of next buffer read operation
		uint64_t m_writeOffset;   //  file pos of next write operation
	};

	AsyncFile::Detail::Detail( const FilePath & filepath, Access access, size_t origin, Share share, AsyncIO & io, size_t recvBufferSize )
		: m_handle( io.context( ) ), m_io( io ), m_autoCloseFlag( false ), /*m_autoCloseBytes( 0 ),*/ m_recvBuffer( recvBufferSize ), m_sendBytes( 0 ), m_isEOF( false ), m_isRecving( false ), m_breakRead( false ), m_recvOffset( 0 ), m_readOffset( 0 ), m_isSending( false ), m_writeOffset( 0 )
	{
		Utf16::Text filename = toUtf16( filepath.toString( ) );
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
			Files::create_directories( filepath.parent_path( ) );
		}

		AsyncFile fileAsyncFile = CreateFile( filename, accessMode, shareMode, 0, creationMode, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0 );
		if ( fileAsyncFile == INVALID_AsyncFile_VALUE )
		{
			DWORD err = GetLastError( );
			throw IOException( String::format( "Unable to open file: error( % )", (uint32_t)err ) );
		}
		m_handle.assign( fileAsyncFile );

		BY_AsyncFile_FILE_INFORMATION fileInfo;
		GetFileInformationByAsyncFile( m_handle.native_AsyncFile( ), &fileInfo );
		size_t filesize = ( (uint64_t)fileInfo.nFileSizeHigh << 32 ) | fileInfo.nFileSizeLow;

		if ( origin == npos )
		{
			origin = filesize;
		}

		if ( access == Access::Read || access == Access::Tail )
		{
			/*
			m_autoCloseBytes = filesize;
			*/
			if ( access == Access::Read )
			{
				m_autoCloseFlag = true;
			}
			m_recvOffset = origin;
			m_readOffset = origin;
			doRecv( );
		}
		else
		{
			m_writeOffset = origin;
		}
	}

	AsyncFile::Detail::~Detail( )
	{
		close( );
	}

	void AsyncFile::Detail::doRecvDelay( )
	{
		m_isEOF = true;
		auto self = std::m
		m_delayTimer = m_io.timer( cpp::Duration::ofMillis( 50 ), [this, self]( )
			{
				doRecv( );
			} );
	}

	void AsyncFile::Detail::doRecv( )
	{
		Memory dst = m_recvBuffer.putable( );
		if ( !dst || m_breakRead )
		{
			return;
		}

		m_isRecving = true;
		m_handle.async_read_some_at( m_recvOffset, asio::buffer( (void *)dst.begin( ), dst.length( ) ), [=]( const std::error_code & error, std::size_t bytes )
			{
				m_isRecving = false;
				if ( error )
				{
					if ( error.value( ) == asio::error::operation_aborted )
					{ /* do nothing */
					}
					else if ( error.value( ) == asio::error::eof )
					{
						doRecvDelay( );
					}
					else
					{
						m_error = error; close( );
					}
				}
				else
				{
					if ( m_isEOF )
					{
						m_isEOF = false;
					}
					m_recvOffset += bytes;
					m_recvBuffer.put( bytes );

					doRecv( );
				}
			} );
	}

	void AsyncFile::Detail::doSend( )
	{
		SendItem & sendItem = m_sendBuffers.front( );
		Memory src = Memory{ sendItem.second }.substr( (size_t)m_sendBytes );
		if ( !src )
		{
			return;
		}

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
					{
						m_sendBuffers.pop_front( ); m_sendBytes = 0;
					}
					if ( !m_sendBuffers.empty( ) )
					{
						doSend( );
					}
				}
			} );
	}


	bool AsyncFile::Detail::isOpen( ) const
	{
		if ( m_recvBuffer.getable( ) )
		{
			return true;
		}
		if ( !m_handle.is_open( ) )
		{
			return false;
		}
		if ( m_autoCloseFlag && m_isEOF )
		{
			return false;
		}
		return true;
	}

	bool AsyncFile::Detail::isEOF( ) const
	{
		return m_isEOF;
	}

	Memory AsyncFile::Detail::read( const Memory & dst, Duration timeout )
	{
		Timer timer;
		while ( isOpen( ) )
		{
			//  wait if the read should block
			if ( m_recvBuffer.getable( ).isEmpty( ) )
			{
				if ( !m_isRecving )
				{
					doRecv( );
				}

				if ( timer.test( timeout ) )
				{
					break;
				}

				if ( timeout.isInfinite( ) /*|| m_recvOffset < m_autoCloseBytes*/ )
				{
					m_io.runOne( );
				}
				else
				{
					m_io.runOne( timer.until( timeout ) );
				}

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
	Memory AsyncFile::Detail::write( const Memory & src )
	{
		if ( src.isEmpty( ) )
		{
			return src;
		}
		if ( m_sendBuffers.size( ) > 1 )
		{
			m_sendBuffers.back( ).second += src;
		}
		else
		{
			m_sendBuffers.emplace_back( SendItem{ m_writeOffset, src } );
		}
		m_writeOffset += src.length( );
		if ( !m_isSending )
		{
			doSend( );
		}
		m_io.poll( );

		if ( m_error )
		{
			auto msg = String::format( "Unable to write file: error( % )", m_error.message( ) );
			throw cpp::IOException( msg );
		}

		return src;
	}
	void AsyncFile::Detail::flush( )
	{
		while ( m_handle.is_open( ) && !m_sendBuffers.empty( ) )
		{
			m_io.runOne( );
		}
	}
	void AsyncFile::Detail::close( )
	{
		flush( );

		m_delayTimer.cancel( );
		if ( m_handle.is_open( ) )
		{
			m_breakRead = true;
			m_handle.cancel( );
			while ( m_isRecving || m_isSending )
			{
				m_io.runOne( );
			}
			m_handle.close( );
		}

	}
	size_t AsyncFile::Detail::readTell( ) const
	{
		return (size_t)m_readOffset;
	}
	void AsyncFile::Detail::readSeek( size_t pos )
	{
		// break pending reads
		m_breakRead = true;
		m_delayTimer.cancel( );
		m_handle.cancel( );
		while ( m_isRecving )
		{
			m_io.runOne( );
		}
		m_breakRead = false;

		//  clear read buffer
		m_recvBuffer.clear( );

		//  set new read position
		m_isEOF = false;
		m_recvOffset = pos;
		m_readOffset = pos;

		doRecv( );
	}
	size_t AsyncFile::Detail::writeTell( ) const
	{
		return (size_t)m_writeOffset;
	}
	void AsyncFile::Detail::writeSeek( size_t pos )
	{
		flush( );
		m_writeOffset = pos;
	}

	void AsyncFile::Detail::truncate( )
	{
		_LARGE_INTEGER pos;
		pos.QuadPart = m_writeOffset;
		check<IOException>( SetFilePointerEx( m_handle.native( ), pos, NULL, FILE_BEGIN ) != FALSE, "Unable to truncate file (SetFilePointerEx)" );
		check<IOException>( SetEndOfFile( m_handle.native( ) ) != FALSE, "Unable to truncate file (SetEndOfFile)" );
	}


	AsyncFile::AsyncFile( nullptr_t )
		: m_detail( nullptr ) { }

	AsyncFile::AsyncFile( )
		: m_detail( nullptr ) { }

	AsyncFile::AsyncFile(
		const FilePath & filepath,
		Access access,
		size_t origin,
		Share share,
		AsyncIO & io,
		size_t buflen )
		: m_detail( std::make_shared<Detail>( filepath, access, origin, share, io, buflen ) ) { }

	AsyncFile::~AsyncFile( )
	{
		m_detail->close( );
	}

	bool AsyncFile::isOpen( ) const
	{
		return m_detail->isOpen( );
	}
	Memory AsyncFile::read( const Memory & dst, Duration timeout )
	{
		return m_detail->read( dst, timeout );
	}
	Memory AsyncFile::write( const Memory & src )
	{
		return m_detail->write( src );
	}
	void AsyncFile::flush( )
	{
		m_detail->flush( );
	}
	void AsyncFile::close( )
	{
		return m_detail->close( );
	}

	bool AsyncFile::isEOF( ) const
	{
		return m_detail->isEOF( );
	}
	size_t AsyncFile::readTell( ) const
	{
		return m_detail->readTell( );
	}
	void AsyncFile::readSeek( size_t pos )
	{
		return m_detail->readSeek( pos );
	}
	size_t AsyncFile::writeTell( ) const
	{
		return m_detail->writeTell( );
	}
	void AsyncFile::writeSeek( size_t pos )
	{
		return m_detail->writeSeek( pos );
	}
	void AsyncFile::truncate( )
	{
		return m_detail->truncate( );
	}

}
#ifndef TEST

#include "../data/DataBuffer.h"
#include "../time/Timer.h"
#include "AsyncFile.h"



namespace cpp
{

	class AsyncFile::Detail
		: public std::enable_shared_from_this<AsyncFile::Detail>
	{
	public:
		static std::shared_ptr<Detail> create( asio::io_context & io, const FilePath & filepath, Access access, Share share );

		Detail( asio::io_context & io );
		~Detail( );

		bool isOpen( ) const;
		size_t length( ) const;

		void read( size_t pos, size_t length, ReadHandler handler );
		void write( size_t pos, std::string data, WriteHandler handler );

		void truncate( size_t length);
		void close( );

		struct ReadContext
		{
			typedef std::shared_ptr<ReadContext> ptr_t;
			size_t readPos;
			size_t readLen;
			std::string buffer;
			size_t bytes;
			ReadHandler handler;
			std::shared_ptr<Detail> self;
		};
		typedef std::function<void( std::error_code error, ReadContext::ptr_t readContext )> ReadSomeHandler;

		void readSome( ReadContext::ptr_t context, ReadSomeHandler handler );
		void readAll( ReadContext::ptr_t context, ReadSomeHandler handler );
		void writeSome( size_t pos, std::string data, WriteHandler handler );

	private:
		mutable asio::windows::random_access_handle handle;
	};


	std::shared_ptr<AsyncFile::Detail> AsyncFile::Detail::create( asio::io_context & io, const FilePath & filepath, Access access, Share share )
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
			Files::create_directories( filepath.parent( ) );
		}

		HANDLE fileHandle = CreateFile( filepath.toWindows( ), accessMode, shareMode, 0, creationMode, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0 );
		if ( fileHandle == INVALID_HANDLE_VALUE )
		{
			DWORD err = GetLastError( );
			throw IOException( cpp::format( "Unable to open file: error( % )", (uint32_t)err ) );
		}

		std::shared_ptr<Detail> detail= std::make_shared<Detail>( io );
		detail->handle.assign( fileHandle );
		return detail;
	}


	AsyncFile::Detail::Detail( asio::io_context & io )
		: handle{ io }
	{
	}


	AsyncFile::Detail::~Detail( )
	{
		close( );
	}


	bool AsyncFile::Detail::isOpen( ) const
	{
		return handle.is_open( );
	}


	size_t AsyncFile::Detail::length( ) const
	{
		BY_HANDLE_FILE_INFORMATION fileInfo;
		GetFileInformationByHandle( handle.native_handle( ), &fileInfo );
		size_t filesize = ( (uint64_t)fileInfo.nFileSizeHigh << 32 ) | fileInfo.nFileSizeLow;
		return filesize;
	}


	void AsyncFile::Detail::read( size_t pos, size_t length, ReadHandler handler )
	{
		ReadContext::ptr_t readContext = std::make_shared<ReadContext>( );
		readContext->readPos = pos;
		readContext->readLen = length;
		readContext->handler = std::move( handler );
		readContext->buffer.resize( length );
		readContext->bytes = 0;
		readContext->self = shared_from_this( );

		readAll( std::move( readContext ), [this]( std::error_code error, ReadContext::ptr_t readContext )
			{ 
				readContext->handler( error, readContext->buffer );
			} );
	}


	void AsyncFile::Detail::write( size_t pos, std::string data, WriteHandler handler )
	{
	}

	void AsyncFile::Detail::readSome( ReadContext::ptr_t readContext, ReadSomeHandler handler )
	{
		size_t readPos = readContext->readPos + readContext->bytes;
		char * buffer = readContext->buffer.data( ) + readContext->bytes;
		size_t buflen = readContext->buffer.length( ) - readContext->bytes;
		handle.async_read_some_at( readPos, asio::buffer( buffer, buflen ), [this, context = std::move( readContext ), handler = std::move( handler )]( std::error_code error, std::size_t bytes )
			{
				if ( !error )
					{ context->bytes += bytes; }

				handler( error, std::move( context ) );
			} );
	}

	void AsyncFile::Detail::readAll( ReadContext::ptr_t readContext, ReadSomeHandler handler )
	{
		readSome( std::move( readContext ), [this, handler = std::move( handler )]( std::error_code error, ReadContext::ptr_t context )
			{
				if ( error || context->bytes == context->readLen )
					{ handler( error, std::move( context ) ); }
				else
					{ readAll( std::move( context ), std::move( handler ) ); }
			} );
	}


	void AsyncFile::Detail::writeSome( size_t pos, std::string data, WriteHandler handler )
	{
	}


	void AsyncFile::Detail::truncate( size_t length )
	{
		_LARGE_INTEGER pos;
		pos.QuadPart = length;
		check<IOException>( SetFilePointerEx( handle.native_handle( ), pos, NULL, FILE_BEGIN ) != FALSE, 
			"AsyncFile::truncate() : SetFilePointerEx() failed" );
		check<IOException>( SetEndOfFile( handle.native_handle( ) ) != FALSE, 
			"AsyncFile::truncate() : SetEndOfFile() failed" );
	}


	void AsyncFile::Detail::close( )
	{
		handle.close( );
	}

/*
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
					{
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

				if ( timeout.isInfinite( ) )
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
	*/



	AsyncFile AsyncFile::readAll( asio::io_context & io, const FilePath & filepath, ReadHandler handler, Share share )
	{
		auto file = AsyncFile{ io, filepath, Access::Read, share };
		file.read( 0, file.length( ), std::move( handler ) );
		return file;
	}


	AsyncFile AsyncFile::stream( asio::io_context & io, const FilePath & filepath, ReadHandler handler, Share share )
	{
		auto file = AsyncFile{ io, filepath, Access::Read, share };
		return file;
	}


	AsyncFile::AsyncFile( nullptr_t )
		: m_detail( nullptr ) { }


	AsyncFile::AsyncFile( )
		: m_detail( nullptr ) { }


	AsyncFile::AsyncFile( asio::io_context & io, const FilePath & filepath, Access access, Share share )
		: m_detail( Detail::create( io, filepath, access, share ) ) { }


	AsyncFile::~AsyncFile( )
	{
		m_detail->close( );
	}


	bool AsyncFile::isOpen( ) const
	{
		return m_detail->isOpen( );
	}


	size_t AsyncFile::length( ) const
	{
		return m_detail->length( );
	}


	void AsyncFile::read( size_t pos, size_t length, ReadHandler handler )
	{
		m_detail->read( pos, length, handler );
	}


	void AsyncFile::write( size_t pos, std::string data, WriteHandler handler )
	{
		m_detail->write( pos, data, handler );
	}


	void AsyncFile::readSome( size_t pos, size_t length, ReadHandler handler )
	{
		Detail::ReadContext::ptr_t readContext = std::make_shared<Detail::ReadContext>( );
		readContext->readPos = pos;
		readContext->readLen = length;
		readContext->handler = std::move( handler );
		readContext->buffer.resize( length );
		readContext->bytes = 0;
		readContext->self = m_detail;

		m_detail->readSome( std::move( readContext ), []( std::error_code error, Detail::ReadContext::ptr_t context )
			{
				context->buffer.resize( context->bytes );
				context->handler( error, context->buffer );
			} );
	}


	void AsyncFile::writeSome( size_t pos, std::string data, WriteHandler handler )
	{
		m_detail->writeSome( pos, data, handler );
	}

	
	void AsyncFile::truncate( size_t length )
	{
		m_detail->truncate( length );
	}


	void AsyncFile::close( )
	{
		m_detail->close( );
	}


}


#else

#include "../meta/Test.h"
#include "../../cpp/file/AsyncFile.h"

TEST_CASE( "AsyncFile" ) 
{
    //cpp::AsyncFile{};

	REQUIRE( 1 == 1 );
	REQUIRE( 2 == 2 );
}

#endif

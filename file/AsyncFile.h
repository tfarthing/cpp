#pragma once

/*

AsyncFile provides file operations through asio.  Aysnchronous file operations are useful for:
	(a) Async operations for file readAll and file stream.
	(b) Asynd read and write operations.
	(c) Blocking length and truncate operations.

*/


#include "../process/AsyncIO.h"
#include "Files.h"

namespace cpp
{

	class AsyncFile
	{
	public:
		enum class Access { Create, Write, Read };
		enum class Share { AllowNone, AllowRead, AllowWrite, AllowAll };
		static const size_t DefaultReadBufferSize = 64 * 1024;

		typedef std::function<void( std::error_code error, std::string & buffer )> ReadHandler;
		typedef std::function<void( std::error_code error, Memory written )> WriteHandler;

		static AsyncFile				readAll( 
											asio::io_context & io, 
											const FilePath & filepath, 
											ReadHandler handler, 
											Share share = Share::AllowAll );
		static AsyncFile				stream( 
											asio::io_context & io, 
											const FilePath & filepath, 
											ReadHandler handler, 
											Share share = Share::AllowAll );

										AsyncFile( nullptr_t );
										AsyncFile( );
										AsyncFile(
											asio::io_context & io,
											const FilePath & filepath,
											Access access = Access::Read,
											Share share = Share::AllowAll );
										~AsyncFile( );

		bool							isOpen( ) const;
		size_t							length( ) const;

		void							read( size_t pos, size_t length, ReadHandler handler );
		void							write( size_t pos, std::string data, WriteHandler handler );

		void							readSome( size_t pos, size_t length, ReadHandler handler );
		void							writeSome( size_t pos, std::string data, WriteHandler handler );

		void							truncate( size_t length = 0 );
		void							close( );

	private:
		class Detail;
		std::shared_ptr<Detail>			m_detail;
	};


}
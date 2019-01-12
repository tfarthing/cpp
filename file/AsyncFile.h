#pragma once

#include "../process/AsyncIO.h"
#include "Files.h"

namespace cpp
{

	class AsyncFile
	{
	public:
		enum class Access { Create, Write, Read, Tail };
		enum class Share { AllowNone, AllowRead, AllowWrite, AllowAll };
		static const size_t DefaultReadBufferSize = 64 * 1024;

		AsyncFile( nullptr_t );
		AsyncFile( );
		AsyncFile(
			const FilePath & filepath,
			Access access = Access::Read,
			size_t origin = 0,
			Share share = Share::AllowWrite,
			AsyncIO & io = AsyncIO{ },
			size_t recvBufferSize = DefaultReadBufferSize );
		~AsyncFile( );

		bool isOpen( ) const;
		Memory read( const Memory & dst, Duration timeout );
		Memory write( const Memory & src );
		void flush( );
		void close( );

		bool isEOF( ) const;
		size_t readTell( ) const;
		void readSeek( size_t pos );
		size_t writeTell( ) const;
		void writeSeek( size_t pos );
		void truncate( );

	private:
		class Detail;
		std::shared_ptr<Detail> m_detail;
	};


}
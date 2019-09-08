#include "MemoryFile.h"



namespace cpp
{
	struct MemoryFile::Detail
		: public cpp::Input::Source
	{
		Detail(
			const FilePath & filepath,
			size_t maxSize,
			Access access = Access::Read,
			Share share = Share::All );

		~Detail( ) override;
		bool								isOpen( ) const override;
		Memory								readsome( Memory dst, std::error_code & errorCode ) override;
		void								close( ) override;

		void								flush( );
		size_t								length( ) const;

		HANDLE					            fileHandle = nullptr;
		HANDLE					            mapHandle = nullptr;
		char * mapView = nullptr;
		size_t								iterator = 0;
	};


	MemoryFile::Detail::Detail(
		const FilePath & filepath,
		size_t maxSize,
		Access access,
		Share share )
	{
		fileHandle = CreateFile(
			filepath.toWindows( ),
			( access != Access::Read ) ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ,
			( share == Share::Write ) ? FILE_SHARE_READ | FILE_SHARE_WRITE : FILE_SHARE_READ,
			NULL,
			( access == Access::Create || !cpp::Files::exists( filepath ) ) ? CREATE_ALWAYS : OPEN_EXISTING,
			NULL,
			NULL );
		check<Input::Exception>( fileHandle != (HANDLE)INVALID_HANDLE_VALUE, std::error_code( GetLastError( ), std::system_category( ) ) );

		mapHandle = CreateFileMapping(
			fileHandle,
			NULL,
			PAGE_READWRITE,
			0,
			0,
			NULL );
		check<Input::Exception>( mapHandle != nullptr, std::error_code( GetLastError( ), std::system_category( ) ) );

		mapView = (char *)MapViewOfFile(
			mapHandle,
			( access != Access::Read ) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
			0,
			0,
			0 );
		cpp::windows::check( mapView != nullptr );
	}


	MemoryFile::Detail::~Detail( )
	{
		close( );
	}


	bool MemoryFile::Detail::isOpen( ) const
	{
		return ( mapView || mapHandle || fileHandle );
	}


	Memory MemoryFile::Detail::readsome( Memory dst, std::error_code & errorCode )
	{
		if ( !mapView )
		{
			errorCode = std::make_error_code( std::errc::connection_aborted );
			return Memory::Empty;
		}

		size_t len = length( );
		if ( iterator == len )
		{
			close( );
			return Memory::Empty;
		}

		Memory result = Memory::copy( dst, Memory{ mapView + iterator, std::min<size_t>( len - iterator, dst.length( ) ) } );
		iterator += result.length( );
		return result;
	}


	void MemoryFile::Detail::close( )
	{
		if ( mapView )
		{
			UnmapViewOfFile( mapView );
			mapView = nullptr;
		}
		if ( mapHandle )
		{
			CloseHandle( mapHandle );
			mapHandle = nullptr;
		}
		if ( fileHandle )
		{
			CloseHandle( fileHandle );
			fileHandle = nullptr;
		}
	}


	void MemoryFile::Detail::flush( )
	{
		FlushViewOfFile( mapHandle, 0 );
	}


	size_t MemoryFile::Detail::length( ) const
	{
		LARGE_INTEGER size;
		cpp::windows::check( GetFileSizeEx( fileHandle, &size ) != 0 );
		return (size_t)size.QuadPart;
	}



	MemoryFile::MemoryFile( const FilePath & filepath, size_t maxSize, Access access, Share share )
		: m_detail( std::make_shared<Detail>( filepath, maxSize, access, share ) )
	{
		
	}


	bool MemoryFile::isOpen( ) const
	{
		return m_detail && m_detail->isOpen( );
	}


	void MemoryFile::flush( )
	{
		m_detail->flush( );
	}


	void MemoryFile::close( )
	{
		m_detail->close( );
	}


	size_t MemoryFile::length( ) const
	{
		return m_detail->length( );
	}


	Memory MemoryFile::data( )
	{
		return Memory{ m_detail->mapView, length( ) };
	}


	const Memory MemoryFile::data( ) const
	{
		return Memory{ m_detail->mapView, length( ) };
	}

	Input MemoryFile::input( )
	{
		return Input{ m_detail };
	}

}

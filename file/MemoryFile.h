#pragma once

#include "../data/Memory.h"
#include "Files.h"
#include "../text/Utf16.h"
#include "../platform/windows/WindowsException.h"


namespace cpp
{

	class MemoryFile
	{
	public:
		MemoryFile( );
		MemoryFile( FilePath filename );
		MemoryFile( FilePath filename, size_t maxSize, bool overwrite );
		~MemoryFile( );

		MemoryFile & open( FilePath filename );
		MemoryFile & open( FilePath filename, size_t maxSize, bool overwrite );
		void flush( );
		void close( );

		Memory get( );
		const Memory get( ) const;

		size_t length( ) const;

	private:
		void mapFile( );

	private:
		HANDLE m_fileHandle = nullptr;
		HANDLE m_mapHandle = nullptr;
		char * m_mapView = nullptr;
	};



	inline MemoryFile::MemoryFile( )
	{}


	inline MemoryFile::MemoryFile( FilePath filename )
	{
		open( filename );
	}


	inline MemoryFile::MemoryFile( FilePath filename, size_t maxSize, bool overwrite )
	{
		open( filename, maxSize, overwrite );
	}


	inline MemoryFile::~MemoryFile( )
	{
		close( );
	}


	inline void MemoryFile::mapFile( )
	{
		m_mapHandle = CreateFileMapping( m_fileHandle,
			NULL,
			PAGE_READWRITE,
			0,
			0,
			NULL );
		cpp::windows::check( m_mapHandle != NULL );


		m_mapView = (char *)MapViewOfFile( m_mapHandle,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0 );
		cpp::windows::check( m_mapView != NULL );
	}


	inline MemoryFile & MemoryFile::open( cpp::FilePath filename )
	{
		m_fileHandle = CreateFile(
			filename.c_str( ),
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL );
		cpp::windows::check( m_fileHandle != (HANDLE)INVALID_HANDLE_VALUE );

		mapFile( );

		return *this;
	}


	inline MemoryFile & MemoryFile::open( cpp::FilePath filename, size_t maxSize, bool overwrite )
	{
		m_fileHandle = CreateFile(
			filename.c_str( ),
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			( overwrite || !cpp::Files::exists( filename ) ) ? CREATE_ALWAYS : OPEN_EXISTING,
			NULL,
			NULL );
		cpp::windows::check( m_fileHandle != (HANDLE)INVALID_HANDLE_VALUE );


		mapFile( );

		return *this;
	}


	inline void MemoryFile::flush( )
	{
		FlushViewOfFile( m_mapHandle, 0 );
	}


	inline void MemoryFile::close( )
	{
		if ( m_mapView )
		{
			UnmapViewOfFile( m_mapView );
			m_mapView = nullptr;
		}
		if ( m_mapHandle )
		{
			CloseHandle( m_mapHandle );
			m_mapHandle = nullptr;
		}
		if ( m_fileHandle )
		{
			CloseHandle( m_fileHandle );
			m_fileHandle = nullptr;
		}
	}


	inline size_t MemoryFile::length( ) const
	{
		LARGE_INTEGER size;
		cpp::windows::check( GetFileSizeEx( m_fileHandle, &size ) != 0 );
		return (size_t)size.QuadPart;
	}


	inline cpp::Memory MemoryFile::get( )
	{
		return cpp::Memory{ m_mapView, length( ) };
	}


	inline const cpp::Memory MemoryFile::get( ) const
	{
		return cpp::Memory{ m_mapView, length( ) };
	}

}

#pragma once

#include "../data/Memory.h"
#include "Files.h"
#include "../platform/windows/WindowsException.h"


namespace cpp
{

	class MemoryFile
        : public File
    {
	public:
        static MemoryFile				    read(
                                                const FilePath & filepath,
                                                Share share = Share::AllowAll );
        static MemoryFile				    create(
                                                const FilePath & filepath,
                                                size_t maxSize,
                                                Share share = Share::AllowRead );
        static MemoryFile				    open(
                                                const FilePath & filepath,
                                                size_t maxSize,
                                                Share share = Share::AllowRead );

                                            MemoryFile( );
                                            MemoryFile(
                                                const FilePath & filepath,
                                                size_t maxSize,
                                                Access access = Access::Read,
                                                Share share = Share::AllowAll );
                                            ~MemoryFile( );

        bool							    isOpen( ) const;
        size_t							    length( ) const;

        Memory					            data( );
        const Memory			            data( ) const;

        void                                flush( );
        void							    close( );

        Input			                    input( );

	private:
		void					            mapFile( );

	private:
        class Detail;
        std::shared_ptr<Detail>			    m_detail;

		HANDLE					            m_fileHandle = nullptr;
		HANDLE					            m_mapHandle = nullptr;
		char *					            m_mapView = nullptr;
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
		cpp::windows::check( m_mapHandle != nullptr );


		m_mapView = (char *)MapViewOfFile( m_mapHandle,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0 );
		cpp::windows::check( m_mapView != nullptr );
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

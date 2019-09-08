#pragma once

#include "../data/Memory.h"
#include "Files.h"
#include "FilePath.h"
#include "../platform/windows/WindowsException.h"
#include "../../cpp/io/Input.h"

namespace cpp
{

	class MemoryFile
        : public FileBase
    {
	public:
        static MemoryFile				    read(
                                                const FilePath & filepath,
                                                Share share = Share::All );
        static MemoryFile				    create(
                                                const FilePath & filepath,
                                                size_t maxSize,
                                                Share share = Share::Read );
        static MemoryFile				    open(
                                                const FilePath & filepath,
                                                size_t maxSize,
                                                Share share = Share::Read );

                                            MemoryFile( );
                                            MemoryFile(
                                                const FilePath & filepath,
                                                size_t maxSize,
                                                Access access = Access::Read,
                                                Share share = Share::All );

        bool							    isOpen( ) const;
        size_t							    length( ) const;

        Memory					            data( );
        const Memory			            data( ) const;

        void                                flush( );
        void							    close( );

        Input			                    input( );

	private:
        struct Detail;
        std::shared_ptr<Detail>			    m_detail;
	};



	inline MemoryFile MemoryFile::read(
		const FilePath & filepath,
		Share share )
	{
		return MemoryFile{ filepath, 0, Access::Read, share };
	}


	inline MemoryFile MemoryFile::create(
		const FilePath & filepath,
		size_t maxSize,
		Share share )
	{
		return MemoryFile{ filepath, maxSize, Access::Create, share };
	}


	inline MemoryFile MemoryFile::open(
		const FilePath & filepath,
		size_t maxSize,
		Share share )
	{
		return MemoryFile{ filepath, maxSize, Access::Write, share };
	}


	inline MemoryFile::MemoryFile( )
	{
	}

}

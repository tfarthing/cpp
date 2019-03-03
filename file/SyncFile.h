#pragma once

/*

SyncFile provides standard blocking file operations.
	* for ( auto cursor : SyncFile::read( "file.txt" ).lines( ) )
*/

#include "Files.h"
#include "../../cpp/io/LineReader.h"


namespace cpp
{

    class SyncFile
    {
    public:
        enum class Access { Create, Write, Read };
        enum class Share { AllowNone, AllowRead, AllowWrite, AllowAll };

        static SyncFile				    read(
                                            const FilePath & filepath,
                                            Share share = Share::AllowAll );
        static SyncFile				    create(
                                            const FilePath & filepath,
                                            Share share = Share::AllowRead );
        static SyncFile				    open(
                                            const FilePath & filepath,
                                            Share share = Share::AllowRead );
        static SyncFile				    append(
                                            const FilePath & filepath,
                                            Share share = Share::AllowRead );

                                        SyncFile( );
                                        SyncFile(
                                            const FilePath & filepath,
                                            Access access = Access::Read,
                                            Share share = Share::AllowAll );
                                        ~SyncFile( );

        bool							isOpen( ) const;
        size_t							length( ) const;

        size_t                          tell( ) const;
        void                            seek( size_t pos );
        void                            seekToEnd( );

        Memory 					        read( Memory buffer );
        void							write( Memory data );

        void							truncate( size_t length = 0 );
        void							close( );

        LineReader<SyncFile>			lines( size_t buflen = 64*1024 );

    private:
        class Detail;
        std::shared_ptr<Detail>			m_detail;
    };


}
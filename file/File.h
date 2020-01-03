#pragma once

/*

File provides standard blocking file operations.
	* for ( auto cursor : File::read( "file.txt" ).input( ).lines( ) )
*/

#include "../../cpp/file/Files.h"
#include "../../cpp/file/FilePath.h"
#include "../../cpp/io/Input.h"
#include "../../cpp/io/Output.h"


namespace cpp
{

    class File
        : public FileBase
    {
    public:
        static File				            read(
                                                const FilePath & filepath,
                                                Share share = Share::All );
        static File				            create(
                                                const FilePath & filepath,
                                                Share share = Share::Read );
        static File				            open(
                                                const FilePath & filepath,
                                                Share share = Share::Read );
        static File				            append(
                                                const FilePath & filepath,
                                                Share share = Share::Read );


                                            File( );
                                            File(
                                                const FilePath & filepath,
                                                Access access = Access::Read,
                                                Share share = Share::All );

        bool							    isOpen( ) const;
        size_t							    length( ) const;

        size_t                              tell( ) const;
        void                                seek( size_t pos );
        void                                seekToEnd( );

        Memory 					            read( Memory buffer );
        void							    write( Memory data );
		void								flush( );

        void							    truncate( size_t length = 0 );
        void							    close( );

        Input			                    input( );
        Output			                    output( );

    private:
        class Detail;
        std::shared_ptr<Detail>			    m_detail;
    };


}
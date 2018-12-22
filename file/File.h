#pragma once

#include <cpp/String.h>
#include <cpp/Exception.h>
#include <cpp/Program.h>
#include <cpp/io/Input.h>
#include <cpp/io/Output.h>
#include <cpp/io/file/FilePath.h>
#include <cpp/io/file/FileInfo.h>

namespace cpp
{

    namespace File
    {

        enum class Result { Success, Access, NoEntry, Invalid, Exists, NotEmpty, Unknown };
        enum class Share { AllowNone, AllowRead, AllowWrite, AllowAll };
        const size_t npos = (size_t)-1;
        const size_t defaultReadBufferSize = 64 * 1024;

        class Input
            : public cpp::Input
        {
        public:
            Input( );
            Input( 
                const FilePath & filepath, 
                bool autoClose = true, 
                size_t origin = 0, 
                Share share = Share::AllowWrite, 
                AsyncIO & io = AsyncIO{ },
                size_t recvBufferSize = defaultReadBufferSize );

            bool isEOF( ) const;
            uint64_t tell( ) const;
            void seek( uint64_t pos );
        };

        class Output
            : public cpp::Output
        {
        public:
            Output( );
            Output( 
                const FilePath & filepath, 
                bool overwrite = false, 
                size_t origin = 0, 
                Share share = Share::AllowRead, 
                AsyncIO & io = AsyncIO{ },
                size_t recvBufferSize = defaultReadBufferSize );

            uint64_t tell( ) const;
            void seek( uint64_t pos );

            void truncate( );
        };

        FileInfo info( const FilePath & filepath );
        bool exists( const FilePath & filepath );
        size_t size( const FilePath & filepath );

        File::Output create( const FilePath & filepath, Share share = Share::AllowRead, AsyncIO & io = AsyncIO{ } );
        File::Output write( const FilePath & filepath, size_t origin = 0, Share share = Share::AllowRead, AsyncIO & io = AsyncIO{ } );
        File::Output append( const FilePath & filepath, Share share = Share::AllowRead, AsyncIO & io = AsyncIO{ } );
        File::Input read( const FilePath & filepath, size_t origin = 0, Share share = Share::AllowWrite, AsyncIO & io = AsyncIO{ }, size_t recvBufferSize = defaultReadBufferSize );
        File::Input tail( const FilePath & filepath, size_t origin = npos, Share share = Share::AllowWrite, AsyncIO & io = AsyncIO{ }, size_t recvBufferSize = defaultReadBufferSize );

        Result makePath( const FilePath & path );
        Result remove( const FilePath & filepath );
        Result rename( const FilePath & src, const FilePath & dst );
        Result move( const FilePath & src, const FilePath & dst );
        Result copy( const FilePath & src, const FilePath & dst );
        bool isIdentical( const FilePath & lhs, const FilePath & rhs );
        
        FilePath::Array list( const FilePath & filepath, bool isRecursive = true, Memory nameRegex = Memory::Null, bool includeDirectories = false, bool includeFiles = true );
        FilePath::Array listDirectories( const FilePath & filepath, bool isRecursive = true, Memory nameRegex = Memory::Null );

        Result setHidden( const FilePath & filepath, bool value );
        Result setReadOnly( const FilePath & filepath, bool value );

        String toString( Result result );
        String toString( Share share );


        class Handle
            : public Input::Source_t, public Output::Sink_t
        {
        public:
            enum class Access { Create, Write, Read, Tail };
            Handle( nullptr_t );
            Handle( );
            Handle(
                const FilePath & filepath,
                Access access = Access::Read,
                size_t origin = 0,
                Share share = Share::AllowWrite,
                AsyncIO & io = AsyncIO{ },
                size_t recvBufferSize = defaultReadBufferSize );
            ~Handle( );

            bool isOpen( ) const override;
            Memory read( const Memory & dst, Duration timeout ) override;
            Memory write( const Memory & src ) override;
            void flush( ) override;
            void close( ) override;

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


        class Exception
            : public IOException
        {
        public:
            Exception( Result result )
                : IOException( toString( result ) ), m_result( result ) { }

            File::Result result()
                { return m_result; }

        private:
            Result m_result;
        };

    };

    inline bool isSuccess( File::Result result )
        { return result == File::Result::Success; }
    
    inline void check( File::Result result )
    {
        if ( !isSuccess( result ) )
            { throw File::Exception( result ); }
    }

}

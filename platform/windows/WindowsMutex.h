#pragma once

#include "../../../cpp/time/Duration.h"
#include "../../../cpp/process/Platform.h"
#include "../../../cpp/text/Utf16.h"
#include "../../../cpp/platform/windows/WindowsException.h"

#undef cpp

namespace cpp
{
    namespace windows
    {

        class Mutex
        {
        public:
            static Mutex create( String name )
                { return Mutex{ CreateMutex( NULL, TRUE, cpp::toUtf16( name ).c_str() ) }; }
            static Mutex open( String name )
                { return Mutex{ OpenMutex( NULL, TRUE, cpp::toUtf16( name ).c_str( ) ) }; }

            ~Mutex( )
                { if (m_handle) { CloseHandle(m_handle); } }

            bool waitFor( Duration duration )
                { return WaitForSingleObject( m_handle, (DWORD)duration.millis() ) == WAIT_OBJECT_0; }
            void release( )
                { ReleaseMutex( m_handle ); }

        private:
            Mutex( HANDLE handle )
                : m_handle( handle ) { check( m_handle != NULL ); }
            HANDLE m_handle;
        };

    }
}
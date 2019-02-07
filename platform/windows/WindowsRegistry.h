#pragma once

#include "../../../cpp/data/Memory.h"
#include "../../../cpp/text/Utf16.h"
#include "../../../cpp/process/Platform.h"
#include "../../../cpp/platform/windows/WindowsException.h"

namespace cpp
{

    namespace windows
    {

        class Registry
        {
        public:
            class Key
            {
            public:
                Key( HKEY key = nullptr, bool managedFlag = false )
                    : m_key( key ), m_managedFlag( managedFlag ) { }
                Key( const Key & copy ) = delete;
                Key( Key && move )
                    : m_key( move.m_key ), m_managedFlag( move.m_managedFlag ) { if (move.m_managedFlag) { move.m_key = nullptr; } }
                ~Key( )
                    { if ( m_key && m_managedFlag ) { close(); } }

                Key & operator=( const Key & copy ) = delete;
                Key & operator=( Key && move ) 
                    { close(); m_key = move.m_key; m_managedFlag = move.m_managedFlag; if ( move.m_managedFlag ) { move.m_key = nullptr; } return *this; }

                Key open( Memory subkey )
                {
                    HKEY hkey;
                    check( RegOpenKeyEx( m_key, toUtf16( subkey ).data(), 0, KEY_ALL_ACCESS, &hkey ) == ERROR_SUCCESS );
                    return Key{ hkey, true };
                }

                Key create( Memory subkey )
                {
                    HKEY hkey;
                    DWORD disp;
                    check( RegCreateKeyEx( m_key, toUtf16( subkey ).data(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hkey, &disp ) == ERROR_SUCCESS );
                    return Key{ hkey, true };
                }

                bool exists( Memory subkey )
                {
                    try
                        { return open( subkey ).isOpen( );  }
                    catch ( std::exception & )
                        { }
                    return false;
                }

                bool isOpen() const
                    { return m_key != nullptr; }

                void deleteKey( const Memory & subkey )
                {
                    check( RegDeleteKey( m_key, toUtf16( subkey ).data() ) == ERROR_SUCCESS );
                }

                void flush( )
                    { if ( m_key && m_managedFlag ) { RegFlushKey( m_key ); } }

                void close( )
                    { if ( m_key && m_managedFlag ) { RegCloseKey( m_key ); m_key = nullptr; } }

                void set( Memory name, Memory value )
                {
					Utf16::Text wname = toUtf16( name );
                    Utf16::Text wvalue = toUtf16( value );
                    RegSetValueEx( m_key, wname, 0, REG_SZ, (LPBYTE)wvalue.begin(), ( (DWORD)wvalue.size( ) + 1 )*sizeof( wchar_t ) );
                }

                void set( const Memory & name, DWORD value )
                {
                    Utf16::Text wname = toUtf16( name );
                    RegSetValueEx( m_key, wname, 0, REG_DWORD, (LPBYTE)&value, sizeof( value ) );
                }

				void set( Memory name, Utf16::Text value )
				{
					Utf16::Text wname = toUtf16( name );
					RegSetValueEx( m_key, wname, 0, REG_SZ, (LPBYTE)value.begin( ), ( (DWORD)value.size( ) + 1 ) * sizeof( wchar_t ) );
				}

                bool has( const Memory & name ) const
                {
                    DWORD typeInfo = 0;
                    DWORD size = 0;

                    Utf16::Text wname = toUtf16( name );
                    long result = RegGetValue( m_key, nullptr, wname, RRF_RT_ANY, &typeInfo, nullptr, &size );
                    return ( result == ERROR_SUCCESS || result == ERROR_MORE_DATA );
                }

                String get( const Memory & name, uint32_t maxlen = 1024 ) const
                {
                    DWORD typeInfo = 0;
                    std::wstring buffer( maxlen, '\0' );
                    DWORD size = (DWORD)buffer.length() * sizeof( wchar_t );

                    Utf16::Text wname = toUtf16( name );
                    long result = RegGetValue( m_key, nullptr, wname, RRF_RT_ANY, &typeInfo, (LPBYTE)buffer.c_str( ), &size );
                    if ( result == ERROR_FILE_NOT_FOUND )
                        { size = 0; }
                    else if ( result == ERROR_MORE_DATA )
                        { throw cpp::Exception( String::format( "RegGetValue() failed for key [%], value is too large [% bytes]", name, (uint32_t)size) ); }
                    else if ( result != ERROR_SUCCESS )
                        { throw cpp::Exception( String::format( "RegGetValue() failed for key [%]", name ) ); }
                    
                    buffer.resize( size ? size/sizeof(wchar_t) - 1 : 0 );
                    return toUtf8( buffer );
                }

                void deleteValue( const Memory & name )
                {
                    check( RegDeleteValue( m_key, toUtf16( name ).data() ) == ERROR_SUCCESS );
                }

            private:
                HKEY m_key;
                bool m_managedFlag;
            };

            static Key currentUser()
                { return Key{ HKEY_CURRENT_USER }; }

        };


    }

}
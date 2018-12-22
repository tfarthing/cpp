#include <conio.h>
#include <cpp/util/Utf16.h>
#include <cpp/platform/windows/WindowsConsole.h>
#include <cpp/platform/windows/WindowsException.h>

namespace cpp
{
    namespace windows
    {

        ConsoleUI::ConsoleUI( )
            : m_handle( GetStdHandle( STD_OUTPUT_HANDLE ) ), m_cursor( m_handle )
        {
            check( GetConsoleScreenBufferInfo( m_handle, &m_bufferInfo ) != 0 );
        }

        String ConsoleUI::title( ) const
        {
            std::wstring title( 64, L'\0' );
            title.resize( GetConsoleTitle( (LPWSTR)title.c_str( ), (DWORD)title.size( ) ) );
            return toUtf8( title );
        }

        void ConsoleUI::setTitle( const Memory & title )
        {
            SetConsoleTitle( toUtf16( title ).data() );
        }

        XY<int> ConsoleUI::bufferSize( ) const
        {
            return XY<int>{ m_bufferInfo.dwSize.X, m_bufferInfo.dwSize.Y };
        }

        void ConsoleUI::setBufferSize( XY<int> size )
        {
            COORD dwSize = { (SHORT)size.x(), (SHORT)size.y() };
            SetConsoleScreenBufferSize( m_handle, dwSize );
            GetConsoleScreenBufferInfo( m_handle, &m_bufferInfo );
        }

        Rect<int> ConsoleUI::view( ) const
        {
            auto & rect = m_bufferInfo.srWindow;
            return Rect<int>{ rect.Left, rect.Top, rect.Right - rect.Left, rect.Bottom - rect.Top };
        }

        void ConsoleUI::setViewSize( XY<int> size )
        {
            auto rect = m_bufferInfo.srWindow;
            rect.Right = rect.Left + size.x( );
            rect.Bottom = rect.Top + size.y( );
            SetConsoleWindowInfo( m_handle, TRUE, &rect );
        }

        void ConsoleUI::setViewPosition( XY<int> pos )
        {
            auto rect = m_bufferInfo.srWindow;
            XY<int> diff{ rect.Left - pos.x(), rect.Top - pos.y() };
            rect.Left += diff.x( );
            rect.Right += diff.x( );
            rect.Top += diff.y( );
            rect.Bottom += diff.y( );
            SetConsoleWindowInfo( m_handle, TRUE, &rect );
        }

        ConsoleUI::Cursor & ConsoleUI::cursor( )
        {
            return m_cursor;
        }

        void ConsoleUI::put( XY<int> pos, unicode_t ch, Color color, Color backColor )
        {
            Utf16::Text text{ ch };

            ConsoleUI::Cursor cursor = m_cursor;
            m_cursor.setPosition( pos );
            m_cursor.setColor( color );
            m_cursor.setBackColor( backColor );

            DWORD count = 0;
            WriteConsole( m_handle, text, (DWORD)text.length( ), &count, NULL );

            m_cursor = cursor;
        }

        void ConsoleUI::put( unicode_t ch, Color color, Color backColor )
        {
            Utf16::Text text{ ch };

            ConsoleUI::Cursor cursor = m_cursor;
            m_cursor.setColor( color );
            m_cursor.setBackColor( backColor );

            DWORD count = 0;
            WriteConsole( m_handle, text, (DWORD)text.length(), &count, NULL );

            m_cursor = cursor;
        }

        void ConsoleUI::print( const Memory & string )
        {
            Utf16::Text text = toUtf16( string );

            ConsoleUI::Cursor cursor = m_cursor;

            DWORD count = 0;
            WriteConsole( m_handle, text, (DWORD)text.length( ), &count, NULL );

            m_cursor = cursor;
        }

        //
        void ConsoleUI::clear( Color backColor, Color color, unicode_t ch )
        {
            DWORD cCharsWritten;
            COORD coordScreen = { 0, 0 };
            DWORD length = m_bufferInfo.dwSize.X * m_bufferInfo.dwSize.Y;

            FillConsoleOutputAttribute( m_handle, toTextAttr( color, backColor), length, coordScreen, &cCharsWritten );
            FillConsoleOutputCharacter( m_handle, (wchar_t)ch, length, coordScreen, &cCharsWritten );
            SetConsoleCursorPosition( m_handle, coordScreen );
        }

        unicode_t ConsoleUI::get( )
        {
            return _getch();
        }

        int ConsoleUI::toTextAttr( Color color, Color backColor )
        {
            int result = 0;

            switch ( color )
            {
            case Color::DarkRed:
                result += FOREGROUND_RED; 
                break;
            case Color::DarkGreen:
                result += FOREGROUND_GREEN;
                break;
            case Color::DarkYellow:
                result += FOREGROUND_RED | FOREGROUND_GREEN;
                break;
            case Color::DarkBlue:
                result += FOREGROUND_BLUE;
                break;
            case Color::DarkMagenta:
                result += FOREGROUND_RED | FOREGROUND_BLUE;
                break;
            case Color::DarkCyan:
                result += FOREGROUND_GREEN | FOREGROUND_BLUE;
                break;
            case Color::Grey:
                result += FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                break;
            case Color::Red:
                result += FOREGROUND_RED | FOREGROUND_INTENSITY;
                break;
            case Color::Green:
                result += FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case Color::Yellow:
                result += FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case Color::Blue:
                result += FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            case Color::Magenta:
                result += FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            case Color::Cyan:
                result += FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            case Color::White:
                result += FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            default:
                break;
            };

            switch ( backColor )
            {
            case Color::DarkRed:
                result += BACKGROUND_RED;
                break;
            case Color::DarkGreen:
                result += BACKGROUND_GREEN;
                break;
            case Color::DarkYellow:
                result += BACKGROUND_RED | BACKGROUND_GREEN;
                break;
            case Color::DarkBlue:
                result += BACKGROUND_BLUE;
                break;
            case Color::DarkMagenta:
                result += BACKGROUND_RED | BACKGROUND_BLUE;
                break;
            case Color::DarkCyan:
                result += BACKGROUND_GREEN | BACKGROUND_BLUE;
                break;
            case Color::Grey:
                result += BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
                break;
            case Color::Red:
                result += BACKGROUND_RED | BACKGROUND_INTENSITY;
                break;
            case Color::Green:
                result += BACKGROUND_GREEN | BACKGROUND_INTENSITY;
                break;
            case Color::Yellow:
                result += BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
                break;
            case Color::Blue:
                result += BACKGROUND_BLUE | BACKGROUND_INTENSITY;
                break;
            case Color::Magenta:
                result += BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
                break;
            case Color::Cyan:
                result += BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
                break;
            case Color::White:
                result += BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
                break;
            default:
                break;
            };

            return result;
        }

        ConsoleUI::Cursor::Cursor( HANDLE handle)
            : m_handle( handle ), m_color( Color::White ), m_backColor( Color::Black ), m_isHidden( false )
        {

        }

        XY<int> ConsoleUI::Cursor::position( ) const
        {
            CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
            GetConsoleScreenBufferInfo( m_handle, &bufferInfo );
            return XY<int>{ bufferInfo.dwCursorPosition.X, bufferInfo.dwCursorPosition.Y };
        }

        void ConsoleUI::Cursor::setPosition( XY<int> position )
        {
            SetConsoleCursorPosition( m_handle, COORD{ (SHORT)position.x(), (SHORT)position.y() } );
        }

        cpp::ConsoleUI::Color ConsoleUI::Cursor::color( ) const
        {
            return m_color;
        }

        void ConsoleUI::Cursor::setColor( Color color )
        {
            m_color = color;
            SetConsoleTextAttribute( m_handle, ConsoleUI::toTextAttr( m_color, m_backColor ) );
        }

        cpp::ConsoleUI::Color ConsoleUI::Cursor::backColor( ) const
        {
            return m_backColor;
        }

        void ConsoleUI::Cursor::setBackColor( Color color )
        {
            m_backColor = color;
            SetConsoleTextAttribute( m_handle, ConsoleUI::toTextAttr( m_color, m_backColor ) );
        }

        void ConsoleUI::Cursor::hide( )
        {
            if ( !m_isHidden )
            {
                m_isHidden = true;

                CONSOLE_CURSOR_INFO info;
                GetConsoleCursorInfo( m_handle, &info );
                info.bVisible = false;
                SetConsoleCursorInfo( m_handle, &info );
            }
        }

        void ConsoleUI::Cursor::show( )
        {
            if ( m_isHidden )
            {
                m_isHidden = false;

                CONSOLE_CURSOR_INFO info;
                GetConsoleCursorInfo( m_handle, &info );
                info.bVisible = true;
                SetConsoleCursorInfo( m_handle, &info );
            }
        }

    }
}

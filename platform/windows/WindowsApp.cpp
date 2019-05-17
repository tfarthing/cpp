#include "../../../cpp/platform/windows/WindowsApp.h"

namespace cpp
{
	namespace windows
	{

		/*
		Windows cmdline parsing standards: https://msdn.microsoft.com/en-us/library/17w5ykft.aspx
		* Arguments are delimited by white space, which is either a space or a tab.
		* A string surrounded by double quotation marks ("string") is interpreted as a single
		  argument, regardless of white space contained within. A quoted string can be embedded
		  in an argument.
		* A double quotation mark preceded by a backslash (\") is interpreted as a literal double
		  quotation mark character (").
		* Backslashes are interpreted literally, unless they immediately precede a double
		  quotation mark.
		* If an even number of backslashes is followed by a double quotation mark, one backslash
		  is placed in the argv array for every pair of backslashes, and the double quotation
		  mark is interpreted as a string delimiter.
		* If an odd number of backslashes is followed by a double quotation mark, one backslash
		  is placed in the argv array for every pair of backslashes, and the double quotation
		  mark is "escaped" by the remaining backslash, causing a literal double quotation mark
		  (") to be placed in argv.
		*/
		StringArray App::parseCommandLine( const String & cmdline )
		{
			StringArray arguments;

			size_t pos = 0;
			size_t argIndex = 0;
			bool isQuoted = false;
			size_t bslashCount = 0;
			String arg;

			while ( pos < cmdline.length( ) + 1 )
			{
				char ch = ( pos < cmdline.length( ) ) ? cmdline[pos] : 0;
				switch ( ch )
				{
				case ' ':
				case '\t':
				case '\0':
					//  quoted string ignores space and tab
					if ( isQuoted && ch != '\0' )
					{
						pos += 1; break;
					}

					if ( arg.notEmpty( ) || argIndex < pos )
					{
						arg += cmdline.substr( argIndex, pos - argIndex );
						arguments.add( arg );
						arg.clear( );
					}
					bslashCount = 0;
					pos += 1;
					argIndex = pos;
					break;

				case '\"':
					//  append everything upto quote or first backslash before quote
					arg += cmdline.substr( argIndex, pos - argIndex - bslashCount );

					//  append a backslash for every two before quote
					for ( size_t i = 0; i < bslashCount / 2; i++ )
					{
						arg += "\\";
					}

					// odd backslash count == literal quote
					if ( ( bslashCount % 2 ) != 0 )
					{
						arg += "\"";
					}
					// else quote is a string delimiter
					else
					{
						isQuoted ^= true;
					}

					bslashCount = 0;
					pos += 1;
					argIndex = pos;
					break;

				default:
					bslashCount = ( ch == '\\' ) ? bslashCount + 1 : 0;
					pos += 1;
					break;
				}
			}

			return arguments;
		}

	}

	/*
    Program::Console::Console( )
        : Program::Standard( ), m_ui( ) { }

    Program::Console::Console( int argc, const char ** argv )
        : Program::Standard( argc, argv ), m_ui( ) { }

    Program::Console::Console( int argc, const wchar_t ** argv )
        : Program::Standard( argc, argv ), m_ui( ) { }



    class Program::WinApp::Detail
    {
    public:
        Detail( HINSTANCE hInstance )
        { 
            bool useOle = true;
            if ( useOle )
            {
                //  Drag and Drop in win32 require Ole
                OleInitialize( NULL );
            }
            else
            {
                CoInitializeEx( NULL, COINIT_MULTITHREADED ); 
            }
            m_module.Init( NULL, hInstance );  
        }
        ~Detail( )
        {  
            m_module.Term();  
        }

        CAppModule m_module;
		std::vector<HWND> m_modelessDialogs;
    };

    Program::WinApp::WinApp( void * instance, wchar_t * cmdline )
        : Program::Standard( cmdline ), m_detail( std::make_shared<Detail>( (HINSTANCE)instance ) )
    {
        
    }

	void Program::WinApp::registerModelessDialog(HWND hwnd)
	{
		WinApp & winapp = (WinApp &)program();
		winapp.m_detail->m_modelessDialogs.push_back(hwnd);
	}

    bool Program::WinApp::update( )
    {
        MSG msg;
        while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            BOOL retval = ::GetMessage( &msg, NULL, 0, 0 );

            if ( !retval ) // WM_QUIT, exit message loop
            {
                m_isRunning = false;
				return false;
            }
            else if ( retval != -1 )
            {
				bool handled = false;
				for (HWND dlg : m_detail->m_modelessDialogs)
				{
					if (IsWindow(dlg) && IsDialogMessage(dlg, &msg))
						{ handled = true; break; }
				}
				if ( !handled )
				{
					::TranslateMessage( &msg );
					::DispatchMessage( &msg );
				}
            }
        }
        return Program::Standard::update( );
    }
    
    bool Program::WinApp::updateUntil( const cpp::Time & time )
    {
        MSG msg;
        cpp::Duration timeout;
        while ( Time::now( ) < time && MsgWaitForMultipleObjects( 0, NULL, FALSE, (DWORD)cpp::Duration{ time - Time::now() }.millis(), QS_ALLINPUT ) == WAIT_OBJECT_0 )
        {
            while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
            {
                BOOL retval = ::GetMessage( &msg, NULL, 0, 0 );

                if ( !retval ) // WM_QUIT, exit message loop
                {
                    m_isRunning = false;
                    return false;
                }
                else if ( retval != -1 )
                {
					for (HWND dlg : m_detail->m_modelessDialogs)
					{
						if (IsWindow(dlg) && IsDialogMessage(dlg, &msg))
							{ continue; }
					}
                    ::TranslateMessage( &msg );
                    ::DispatchMessage( &msg );
                }
            }
        }
        return Program::Standard::update( );
    }
	*/
}
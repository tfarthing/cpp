#include <memory>
#include <cpp/Program.h>
#include <cpp/platform/windows/WindowsGraphics.h>

namespace cpp
{

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

}
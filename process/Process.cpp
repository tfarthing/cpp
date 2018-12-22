#include <cpp/Platform.h>
#include <cpp/process/Process.h>
#include <cpp/io/IOPipe.h>
#include <cpp/platform/windows/WindowsException.h>

namespace cpp
{

    struct Process::Detail
    {
        Detail( FilePath exe, String cmdline, HANDLE handle )
            : m_exe(exe), m_cmdline(cmdline), m_handle( handle ), m_input( ), m_output( ), m_error( ) { }
        Detail( FilePath exe, String cmdline, HANDLE handle, IOPipe input, IOPipe output, IOPipe error )
            : m_exe( exe ), m_cmdline( cmdline ), m_handle( handle ), m_input( std::move( input ) ), m_output( std::move( output ) ), m_error( std::move( error ) ) {}
        ~Detail()
            { close(); }

        void close()
        {
            if ( isRunning( ) )
            {
                if ( !TerminateProcess( m_handle, 0 ) )
                {
                    cpp::windows::Exception e;
                    cpp::log( "TerminateProcess(% %) - failed: %", m_exe, m_cmdline, e.what( ) );
                }
                WaitForSingleObject( m_handle, INFINITE );
                detach( );
            }
        }

        void detach()
        {
            if ( m_handle != INVALID_HANDLE_VALUE )
            {
                CloseHandle( m_handle );
                m_handle = INVALID_HANDLE_VALUE;

                m_input.output().close( );
                m_output.input().close( );
                m_error.input().close( );
            }
        }

        bool isRunning()
        {
            if ( m_handle != INVALID_HANDLE_VALUE )
            {
                DWORD exitCode;
                GetExitCodeProcess( m_handle, &exitCode );
                if ( exitCode == STILL_ACTIVE )
                    { return true; }

                m_handle = INVALID_HANDLE_VALUE;
                m_exitCode = exitCode;
            }
            return false;
        }

        static HANDLE createProgramJob() {
            HANDLE job = CreateJobObject( NULL, NULL );
            check( job != NULL, "cpp::Process::createProgramJob() - Unable to create a job" );

            // Configure all child processes associated with the job to terminate when the
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            BOOL ret = SetInformationJobObject( job, JobObjectExtendedLimitInformation, &jeli, sizeof( jeli ) );
            check( ret != FALSE, "cpp::Process::createProgramJob() - Unable to set limit information for the Program's job" );

            return job;
        }

        static HANDLE job() {
            static HANDLE s_job = createProgramJob( );
            return s_job;
        }

        FilePath m_exe;
        String m_cmdline;

        HANDLE m_handle;
        IOPipe m_input;
        IOPipe m_output;
        IOPipe m_error;
        int64_t m_exitCode;

        static Mutex ProgramMutex;    // mutex used to serialize runChild() functions, preventing handle inheritance by the wrong process
    };

    Mutex Process::Detail::ProgramMutex;

    Process::Process( std::shared_ptr<Detail> && detail )
        : m_detail( std::move( detail ) ) { }

    Process::Process( )
        : m_detail( nullptr ) { }

    Process Process::run( FilePath exe, String cmdline, FilePath workingPath )
    {
        BOOL bSuccess = FALSE;
        PROCESS_INFORMATION processInfo = { 0 };
        STARTUPINFO startInfo = { 0 };
        startInfo.cb = sizeof( STARTUPINFO );

        Utf16::Text cmd = toUtf16( String::format( "\"%\" %", exe.toString( '\\' ), cmdline ) );
        Utf16::Text dir = toUtf16( workingPath.isEmpty() ? exe.parent( ).toString( '\\' ) : workingPath.toString( '\\' ) );

        BOOL result = CreateProcess( 
            NULL,
            (LPWSTR)cmd.begin( ),       // command line 
            NULL,                       // process security attributes 
            NULL,                       // primary thread security attributes 
            FALSE,                      // handles are inherited 
            NORMAL_PRIORITY_CLASS,      // creation flags 
            NULL,                       // use parent's environment 
            dir.begin(),                // use parent's current directory 
            &startInfo,                 // STARTUPINFO pointer 
            &processInfo );             // receives PROCESS_INFORMATION 

        windows::check( result != FALSE );
        CloseHandle( processInfo.hThread );
        return Process{ std::make_shared<Detail>( exe, cmdline, processInfo.hProcess ) };
    }

    Process Process::runChild( FilePath exe, String cmdline, FilePath workingPath )
    {
        auto lock = Detail::ProgramMutex.lock( );

        BOOL bSuccess = FALSE;
        PROCESS_INFORMATION piProcInfo = { 0 };
        STARTUPINFO siStartInfo = { 0 };

        IOPipe stdoutPipe = IOPipe::createChildOutput( );
        IOPipe stderrPipe = IOPipe::createChildOutput( );
        IOPipe stdinPipe = IOPipe::createChildInput( );
        
        // Set up members of the STARTUPINFO structure. 
        // This structure specifies the STDIN and STDOUT handles for redirection.
        siStartInfo.cb = sizeof( STARTUPINFO );
        siStartInfo.hStdError = stderrPipe.getOutputHandle();
        siStartInfo.hStdOutput = stdoutPipe.getOutputHandle();
        siStartInfo.hStdInput = stdinPipe.getInputHandle();
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process. 

        Utf16::Text cmd = toUtf16( String::format( "\"%\" %", exe.toString( '\\' ), cmdline ) );
        Utf16::Text dir = toUtf16( workingPath.isEmpty( ) ? exe.parent( ).toString( '\\' ) : workingPath.toString( '\\' ) );

        BOOL result = CreateProcess(
            NULL,
            (LPWSTR)cmd.begin( ),       // command line 
            NULL,                       // process security attributes 
            NULL,                       // primary thread security attributes 
            TRUE,                       // handles are inherited 
            CREATE_NO_WINDOW,           // creation flags 
            NULL,                       // use parent's environment 
            dir.begin( ),               // use parent's current directory 
            &siStartInfo,               // STARTUPINFO pointer 
            &piProcInfo );              // receives PROCESS_INFORMATION 

        windows::check( result != FALSE );

        stdoutPipe.output( ).close( );
        stderrPipe.output( ).close( );
        stdinPipe.input( ).close( );

        //  change remaining pipe handles to inherit = false so subsequenct children won't inherit
        SetHandleInformation( stderrPipe.getInputHandle(), HANDLE_FLAG_INHERIT, FALSE );
        SetHandleInformation( stdoutPipe.getInputHandle(), HANDLE_FLAG_INHERIT, FALSE );
        SetHandleInformation( stdinPipe.getOutputHandle(), HANDLE_FLAG_INHERIT, FALSE );
        
        //ResumeThread( piProcInfo.hThread );
        CloseHandle( piProcInfo.hThread );

        lock.unlock( );

        stdoutPipe.startInput( );
        stderrPipe.startInput( );

        AssignProcessToJobObject( Detail::job( ), piProcInfo.hProcess );

        return Process{ std::make_shared<Detail>( exe, cmdline, piProcInfo.hProcess, std::move(stdinPipe), std::move(stdoutPipe), std::move(stderrPipe) ) };
    }

    Process::~Process( )
    {
        close( );
    }

    bool Process::isRunning( ) const
    {
        return m_detail ? m_detail->isRunning( ) : false;
    }

    void Process::wait( )
    {
        while ( WaitForSingleObject( m_detail->m_handle, 200 ) == WAIT_TIMEOUT )
            { Thread::checkInterrupt(); }
    }

    bool Process::waitFor( Duration timeout )
    {
        return waitUntil( Time::now( ) + timeout );
    }
    
    bool Process::waitUntil( Time time )
    {
        while ( Time::now() < time && WaitForSingleObject( m_detail->m_handle, 200 ) == WAIT_TIMEOUT )
            { Thread::checkInterrupt(); }
        return Time::now( ) < time;
    }

    void Process::detach( )
    {
        if ( m_detail )
            { m_detail->detach(); }
    }

    void Process::close( )
    {
        if ( m_detail )
            { m_detail->close( ); }
    }

    Output Process::input( )
    {
        return isRunning( ) ? m_detail->m_input.output( ) : nullptr;
    }

    Input Process::output( )
    {
        return ( m_detail ) ? m_detail->m_output.input( ) : nullptr;
    }

    Input Process::error( )
    {
        return ( m_detail ) ? m_detail->m_error.input( ) : nullptr;
    }

    FilePath Process::exe( ) const
    {
        return ( m_detail ) ? m_detail->m_exe : FilePath{ };
    }

    String Process::cmdline( ) const
    {
        return ( m_detail ) ? m_detail->m_cmdline : String{ };
    }


}

#include <deque>

#include "Exception.h"
#include "Process.h"
#include "Process-Pipe.h"



BOOL CreateProcessWithExplicitHandles(
	__in_opt     LPCTSTR lpApplicationName,
	__inout_opt  LPTSTR lpCommandLine,
	__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in         BOOL bInheritHandles,
	__in         DWORD dwCreationFlags,
	__in_opt     LPVOID lpEnvironment,
	__in_opt     LPCTSTR lpCurrentDirectory,
	__in         LPSTARTUPINFO lpStartupInfo,
	__out        LPPROCESS_INFORMATION lpProcessInformation,
	// here is the new stuff
	__in         DWORD cHandlesToInherit,
	__in_ecount( cHandlesToInherit ) HANDLE *rgHandlesToInherit );



Process Process::run(
	asio::io_context & io,
	std::string cmdline,
	std::string workingPath,
	RecvHandler onStdout,
	RecvHandler onStderr,
	ExitHandler onExit )
{
	return Process{ std::make_shared<Detail>( io, cmdline, workingPath, onStdout, onStderr, onExit ) };
}


Process Process::runDetached(
	asio::io_context & io,
	std::string cmdline,
	std::string workingPath,
	ExitHandler onExit )
{
	return Process{ std::make_shared<Detail>( io, cmdline, workingPath, nullptr, nullptr, onExit ) };
}



std::wstring toUtf16( const std::string & utf8 )
{
	std::wstring utf16( utf8.size( ), L'\0' );
	int len = MultiByteToWideChar( CP_UTF8, 0, utf8.data( ), (int)utf8.size( ), (LPWSTR)utf16.data( ), (int)utf16.size( ) );
	utf16.resize( len );
	return utf16;
}



struct Process::Detail
	: public std::enable_shared_from_this<Process::Detail>
{
	Detail( 
		asio::io_context & io,
		std::string cmdline,
		std::string workingPath,
		RecvHandler onStdout,
		RecvHandler onStderr,
		ExitHandler onExit );
	~Detail( );

	void open( );
	void recv( asio::windows::stream_handle & stream, std::string & buffer, RecvHandler & handler );
	void send( std::string data );
	void send( );
	void wait( );
	void waitUntil( Time time );
	void close( bool wait = true );
	void detach( );
	bool isRunning( ) const;
	
	asio::io_context & io;
	std::string cmdline;
	std::string workingPath;
	RecvHandler outputHandler;
	RecvHandler errorHandler;
	ExitHandler exitHandler;

	bool runningFlag = false;
	HANDLE processHandle = INVALID_HANDLE_VALUE;
	Pipe inputPipe;
	Pipe outputPipe;
	Pipe errorPipe;
	int32_t exitCode = -1;

	asio::windows::object_handle processEvent;
	asio::windows::stream_handle inputStream;
	asio::windows::stream_handle outputStream;
	asio::windows::stream_handle errorStream;
	std::deque<std::string> inputBuffers;
	std::string outputBuffer;
	std::string errorBuffer;
	bool isSending = false;

	static HANDLE currentProcessGroup( );
};



Process::Detail::Detail( 
	asio::io_context & io_,
	std::string cmdline_,
	std::string workingPath_,
	RecvHandler onStdout,
	RecvHandler onStderr,
	ExitHandler onExit ) : 
		io( io_ ), 
		cmdline( cmdline_ ), 
		workingPath( workingPath_ ), 
		outputHandler( onStdout ), 
		errorHandler( onStderr ), 
		exitHandler( onExit ), 
		processEvent( io ),
		inputStream( io ),
		outputStream( io ),
		errorStream( io )
{ 
}

Process::Detail::~Detail( )
{
	try
	{
		close( );
	}
	catch ( ... )
	{
	}
}

void Process::Detail::open( )
{
	PROCESS_INFORMATION piProcInfo = { 0 };
	STARTUPINFO siStartInfo = { 0 };
	siStartInfo.cb = sizeof( STARTUPINFO );

	bool isDetached = ( !outputHandler && !errorHandler );

	if ( !isDetached )
	{
		inputPipe = Pipe{ 4096 };
		outputPipe = Pipe{ 4096 };
		errorPipe = Pipe{ 4096 };

		Pipe::setNoInherit( inputPipe.output );
		Pipe::setNoInherit( outputPipe.input );
		Pipe::setNoInherit( errorPipe.input );

		// Set up members of the STARTUPINFO structure. 
		// This structure specifies the STDIN and STDOUT handles for redirection.
		siStartInfo.hStdInput = inputPipe.input;
		siStartInfo.hStdOutput = outputPipe.output;
		siStartInfo.hStdError = errorPipe.output;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	}

	// Create the child process. 
	std::wstring cmd = toUtf16( cmdline );
	std::wstring dir = toUtf16( workingPath );

	HANDLE childHandles[3] = { inputPipe.input, outputPipe.output, errorPipe.output };
	BOOL result = CreateProcessWithExplicitHandles(
		NULL,
		(LPWSTR)cmd.data( ),		// command line 
		NULL,                       // process security attributes 
		NULL,                       // primary thread security attributes 
		TRUE,                       // handles are inherited 
		isDetached ? CREATE_BREAKAWAY_FROM_JOB|CREATE_NEW_CONSOLE : CREATE_NO_WINDOW,           // creation flags 
		NULL,                       // use parent's environment 
		dir.empty() ? NULL : dir.data( ),
		&siStartInfo,
		&piProcInfo,
		isDetached ? 0 : 3,
		isDetached ? nullptr : childHandles );

	check<std::exception>( result, "Process::CreateProcess() failed" );

	if ( !isDetached )
	{
		// prevent any subsequent child from inheriting remaining handles
		Pipe::setNoInherit( inputPipe.input );
		Pipe::setNoInherit( outputPipe.output );
		Pipe::setNoInherit( errorPipe.output );
		// release the remote side of the pipes
		inputPipe.releaseInput( );
		outputPipe.releaseOutput( );
		errorPipe.releaseOutput( );
	}

	//ResumeThread( piProcInfo.hThread );
	CloseHandle( piProcInfo.hThread );

	processHandle = piProcInfo.hProcess;

	//	add this child process to the current process group (job)
	if ( !isDetached )
		{ AssignProcessToJobObject( Detail::currentProcessGroup( ), processHandle ); }

	auto self = shared_from_this( );
	processEvent = asio::windows::object_handle{ io, processHandle };
	processEvent.async_wait( [this, self]( std::error_code ec )
	{
		runningFlag = false;
			
		DWORD value;
		GetExitCodeProcess( processHandle, &value );
		exitCode = (int32_t)value;

		if ( exitHandler )
		{ 
			exitHandler( exitCode ); 
		}

		exitHandler = nullptr;
		outputHandler = nullptr;
		errorHandler = nullptr;
	} );

	inputStream.assign( inputPipe.output );

	if ( outputHandler )
	{
		outputStream.assign( outputPipe.input );
		recv( outputStream, outputBuffer, outputHandler );
	}
	else
	{
		outputPipe.close( );
	}

	if ( errorHandler )
	{
		errorStream.assign( errorPipe.input );
		recv( errorStream, errorBuffer, errorHandler );
	}
	else
	{
		errorPipe.close( );
	}

	runningFlag = true;
}


void Process::Detail::recv( asio::windows::stream_handle & stream, std::string & buffer, RecvHandler & handler )
{
	using namespace std::placeholders;

	size_t offset = buffer.length( );
	if ( buffer.length( ) - offset < 4096 )
		{ buffer.resize( buffer.length( ) + 4096 ); }

	auto self = shared_from_this( );
	asio::mutable_buffer buf{ (char *)buffer.data( ) + offset, buffer.length( ) - offset };
	stream.async_read_some( buf, [this, self, offset, &stream, &buffer, &handler]( const std::error_code & error, std::size_t bytes )
		{
			if ( !error )
			{
				buffer.resize( offset + bytes );
				handler( buffer );

				recv( stream, buffer, handler );
			}
		} );
}


void Process::Detail::send( std::string data )
{
	inputBuffers.push_back( std::move( data ) );
	send( );
}

void Process::Detail::send( )
{
	using namespace std::placeholders;

	if ( isSending || inputBuffers.empty( ) )
		{ return; }

	isSending = true;
	auto self = shared_from_this( );
	asio::const_buffer buf{ (const char *)inputBuffers[0].data( ), inputBuffers[0].length( ) };
	inputStream.async_write_some( buf, [this, self]( const std::error_code & error, std::size_t bytes )
		{
			isSending = false;
			if ( !error )
			{
				if ( bytes == inputBuffers[0].length( ) )
					{ inputBuffers.pop_front( ); }
				else
					{ inputBuffers[0].erase( 0, bytes ); }
				send( );
			}
		} );
}

void Process::Detail::wait( )
{
	// blocks until exitHandler is called
	while ( exitHandler )
	{
		io.run_one( );
	}
	// blocks until processHandler is signalled
	WaitForSingleObject( processHandle, INFINITE );
}


void Process::Detail::waitUntil( Time time )
{
	TODO
	bool timeoutFlag = false;
	asio::steady_timer timer{ io, time };
	timer.async_wait( [&timeoutFlag]( std::error_code error )
		{

		} );
	// blocks until exitHandler is called
	while ( exitHandler )
	{
		io.run_one( );
	}
	timer.cancel( );
	// blocks until processHandler is signalled
	WaitForSingleObject( processHandle, INFINITE );
}


void Process::Detail::close( bool waitFlag )
{
	std::error_code result;

	if ( isRunning( ) )
	{
		check<std::exception>( TerminateProcess( processHandle, 0 ), "Process::TerminateProcess() failed" );
		if ( waitFlag )
		{
			wait( );
		}
		detach( );
	}
}

void Process::Detail::detach( )
{
	if ( processHandle != INVALID_HANDLE_VALUE )
	{
		processEvent.close( );  // CloseHandle( processHandle )
		inputStream.close( );	// CloseHandle( inputPipe.output )
		outputStream.close( );	// CloseHandle( outputPipe.input )
		errorStream.close( );   // CloseHandle( errorPipe.input )

		processHandle = INVALID_HANDLE_VALUE;

		runningFlag = false;
	}
}

bool Process::Detail::isRunning( ) const
{
	return runningFlag;
}

HANDLE createJob( ) {
	HANDLE job = CreateJobObject( NULL, NULL );
	check<std::exception>( job != NULL, "cpp::Process::createProgramJob() - Unable to create a job" );

	// Configure all child processes associated with the job to terminate when the
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	BOOL ret = SetInformationJobObject( job, JobObjectExtendedLimitInformation, &jeli, sizeof( jeli ) );
	check<std::exception>( ret != FALSE, "cpp::Process::createProgramJob() - Unable to set limit information for the Program's job" );

	return job;
}

HANDLE Process::Detail::currentProcessGroup( ) {
	static HANDLE s_job = createJob( );
	return s_job;
}


Process::Process( )
{
}

Process::Process( std::shared_ptr<Detail> && detail_ )
	: detail( std::move( detail_ ) )
{
	detail->open( );
}

Process::Process( Process && move )
	: detail( std::move( move.detail ) )
{
}

Process::~Process( )
{
	try
		{ close( ); }
	catch ( ... )
		{ }
}

Process & Process::operator=( Process && move )
{
	detail = std::move( move.detail );
	return *this;
}

bool Process::isRunning( ) const
{
	return detail ? detail->isRunning( ) : false;
}

void Process::send( std::string msg )
{
	if ( detail )
	{
		detail->send( std::move( msg ) );
	}
}

void Process::detach( )
{
	if ( detail )
	{
		detail->detach( );
	}
}

void Process::close( )
{
	if ( detail )
	{
		detail->close( );
	}
}

void Process::wait( )
{
	if ( detail )
	{
		detail->wait( );
	}
}

bool Process::waitFor( Duration timeout )
{
	if ( detail )
	{
		detail->waitUntil( Time::clock::now() + timeout );
	}
	return !isRunning( );
}

bool Process::waitUntil( Time time )
{
	if ( detail )
	{
		detail->waitUntil( time );
	}
	return !isRunning( );
}


int32_t Process::exitValue( ) const
{
	return detail ? detail->exitCode : 0;
}




BOOL CreateProcessWithExplicitHandles(
	__in_opt     LPCTSTR lpApplicationName,
	__inout_opt  LPTSTR lpCommandLine,
	__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in         BOOL bInheritHandles,
	__in         DWORD dwCreationFlags,
	__in_opt     LPVOID lpEnvironment,
	__in_opt     LPCTSTR lpCurrentDirectory,
	__in         LPSTARTUPINFO lpStartupInfo,
	__out        LPPROCESS_INFORMATION lpProcessInformation,
	// here is the new stuff
	__in         DWORD cHandlesToInherit,
	__in_ecount( cHandlesToInherit ) HANDLE *rgHandlesToInherit )
{
	BOOL fSuccess;
	BOOL fInitialized = FALSE;
	SIZE_T size = 0;
	LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList = NULL;

	fSuccess = cHandlesToInherit < 0xFFFFFFFF / sizeof( HANDLE ) && lpStartupInfo->cb == sizeof( *lpStartupInfo );

	if ( cHandlesToInherit )
	{
		if ( !fSuccess )
		{
			SetLastError( ERROR_INVALID_PARAMETER );
		}

		if ( fSuccess )
		{
			fSuccess = InitializeProcThreadAttributeList( NULL, 1, 0, &size ) || GetLastError( ) == ERROR_INSUFFICIENT_BUFFER;
		}

		if ( fSuccess )
		{
			lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>( HeapAlloc( GetProcessHeap( ), 0, size ) );
			fSuccess = lpAttributeList != NULL;
		}

		if ( fSuccess )
		{
			fSuccess = InitializeProcThreadAttributeList( lpAttributeList, 1, 0, &size );
		}

		if ( fSuccess )
		{
			fInitialized = TRUE;
			fSuccess = UpdateProcThreadAttribute( lpAttributeList,
				0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
				rgHandlesToInherit,
				cHandlesToInherit * sizeof( HANDLE ), NULL, NULL );
		}
	}

	if ( fSuccess ) 
	{
		STARTUPINFOEX info;
		ZeroMemory( &info, sizeof( info ) );
		info.StartupInfo = *lpStartupInfo;
		info.StartupInfo.cb = sizeof( info );
		info.lpAttributeList = lpAttributeList;
		fSuccess = CreateProcess( lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags | EXTENDED_STARTUPINFO_PRESENT,
			lpEnvironment,
			lpCurrentDirectory,
			&info.StartupInfo,
			lpProcessInformation );
	}

	if ( fInitialized )
		{ DeleteProcThreadAttributeList( lpAttributeList ); }
	
	if ( lpAttributeList )
		{ HeapFree( GetProcessHeap( ), 0, lpAttributeList ); }
	
	return fSuccess;
}
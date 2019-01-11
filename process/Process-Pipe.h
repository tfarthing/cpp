#pragma once

#include <atomic>

#include "Exception.h"
#include "Platform.h"



BOOL APIENTRY MyCreatePipeEx( OUT LPHANDLE lpReadPipe, OUT LPHANDLE lpWritePipe, IN LPSECURITY_ATTRIBUTES lpPipeAttributes, IN DWORD nSize, DWORD dwReadMode, DWORD dwWriteMode );

namespace cpp
{

	struct Pipe
	{
		Pipe( )
			: input( INVALID_HANDLE_VALUE ), output( INVALID_HANDLE_VALUE )
		{
		}

		Pipe( size_t buflen )
			: input( INVALID_HANDLE_VALUE ), output( INVALID_HANDLE_VALUE )
		{
			SECURITY_ATTRIBUTES attrs;
			attrs.nLength = sizeof( SECURITY_ATTRIBUTES );
			attrs.bInheritHandle = TRUE;
			attrs.lpSecurityDescriptor = NULL;

			check<std::exception>( MyCreatePipeEx( &input, &output, &attrs, (DWORD)buflen, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED ),
				"Pipe::MyCreatePipeEx() failed" );
		}

		void close( )
		{
			releaseInput( );
			releaseOutput( );
		}

		void releaseInput( )
		{
			if ( input != INVALID_HANDLE_VALUE )
			{
				CloseHandle( input );
				input = INVALID_HANDLE_VALUE;
			}
		}

		void releaseOutput( )
		{
			if ( output != INVALID_HANDLE_VALUE )
			{
				CloseHandle( output );
				output = INVALID_HANDLE_VALUE;
			}
		}

		static void setNoInherit( HANDLE handle )
		{
			check<std::exception>( SetHandleInformation( handle, HANDLE_FLAG_INHERIT, 0 ), "Pipe::SetHandleInformation() failed" );
		}

		HANDLE input;
		HANDLE output;
	};

}

/*++
Routine Description:
	The CreatePipeEx API is used to create an anonymous pipe I/O device.
	Unlike CreatePipe FILE_FLAG_OVERLAPPED may be specified for one or
	both handles.
	Two handles to the device are created.  One handle is opened for
	reading and the other is opened for writing.  These handles may be
	used in subsequent calls to ReadFile and WriteFile to transmit data
	through the pipe.
Arguments:
	lpReadPipe - Returns a handle to the read side of the pipe.  Data
		may be read from the pipe by specifying this handle value in a
		subsequent call to ReadFile.
	lpWritePipe - Returns a handle to the write side of the pipe.  Data
		may be written to the pipe by specifying this handle value in a
		subsequent call to WriteFile.
	lpPipeAttributes - An optional parameter that may be used to specify
		the attributes of the new pipe.  If the parameter is not
		specified, then the pipe is created without a security
		descriptor, and the resulting handles are not inherited on
		process creation.  Otherwise, the optional security attributes
		are used on the pipe, and the inherit handles flag effects both
		pipe handles.
	nSize - Supplies the requested buffer size for the pipe.  This is
		only a suggestion and is used by the operating system to
		calculate an appropriate buffering mechanism.  A value of zero
		indicates that the system is to choose the default buffering
		scheme.
Return Value:
	TRUE - The operation was successful.
	FALSE/NULL - The operation failed. Extended error status is available
		using GetLastError.
--*/
BOOL APIENTRY MyCreatePipeEx(
	OUT LPHANDLE lpReadPipe,
	OUT LPHANDLE lpWritePipe,
	IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
	IN DWORD nSize,
	DWORD dwReadMode,
	DWORD dwWriteMode )
{
	static std::atomic<ULONG> PipeSerialNumber = 0;

	HANDLE ReadPipeHandle, WritePipeHandle;
	DWORD dwError;
	char PipeNameBuffer[MAX_PATH];

	//
	// Only one valid OpenMode flag - FILE_FLAG_OVERLAPPED
	//

	if ( ( dwReadMode | dwWriteMode ) & ( ~FILE_FLAG_OVERLAPPED ) ) 
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return FALSE;
	}

	//
	//  Set the default timeout to 120 seconds
	//

	if ( nSize == 0 ) 
	{
		nSize = 4096;
	}

	snprintf( PipeNameBuffer, MAX_PATH,
		"\\\\.\\Pipe\\Anon.%08x.%08x",
		GetCurrentProcessId( ),
		PipeSerialNumber++
	);

	ReadPipeHandle = CreateNamedPipeA(
		PipeNameBuffer,
		PIPE_ACCESS_INBOUND | dwReadMode,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1,             // Number of pipes
		nSize,         // Out buffer size
		nSize,         // In buffer size
		120 * 1000,    // Timeout in ms
		lpPipeAttributes
	);

	if ( !ReadPipeHandle ) {
		return FALSE;
	}

	WritePipeHandle = CreateFileA(
		PipeNameBuffer,
		GENERIC_WRITE,
		0,                         // No sharing
		lpPipeAttributes,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | dwWriteMode,
		NULL                       // Template file
	);

	if ( INVALID_HANDLE_VALUE == WritePipeHandle ) 
	{
		dwError = GetLastError( );
		CloseHandle( ReadPipeHandle );
		SetLastError( dwError );
		return FALSE;
	}

	*lpReadPipe = ReadPipeHandle;
	*lpWritePipe = WritePipeHandle;
	
	return( TRUE );
}
#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>

#define PIPE_BEFFERSIZE 19

struct ThreadData 
{ 
	HANDLE write_thread, 
		   read_pipe; 
};

void ThreadFunction(LPVOID lp_void)
{
	HANDLE read_pipe    = ((ThreadData*)lp_void)->read_pipe;
	HANDLE write_thread = ((ThreadData*)lp_void)->write_thread;

	// Reading data from pipe.
	for (int i = 0; i < 50; i++)
	{
		char  read_buffer[50] = {'\0'};
		BOOL  ret = FALSE;
		DWORD read;

		if ((i >= 0) && (i <= 9))
			ret = ReadFile(read_pipe, &read_buffer, 18, &read, NULL);
		else 
			ret = ReadFile(read_pipe, &read_buffer, 19, &read, NULL);

		if (ret == FALSE) 
		{
			assert(ret != FALSE);
			break;
		}

		std::cout << "Thread had Read: " << read_buffer;
		if ((read == 18) || (read == 19))
		{
			// This acts as a toggle between the reader & writer thread.
			ResumeThread(write_thread);
			SuspendThread(GetCurrentThread());
		}
	}
	CloseHandle(read_pipe);
	ExitThread(0);
}

int main(void)
{
	HANDLE read_pipe	= NULL;	// read end of pipe
	HANDLE write_pipe	= NULL;	// write end of pipe
	HANDLE read_thread	= NULL;	// to reader thread
	HANDLE write_thread = NULL;	// to writer thread
	HANDLE write_dupl   = NULL;	// duplicate to writer thread

	BOOL  Return		= FALSE;
	DWORD thread_id;

	ThreadData thread_data;	// to be sent to reader thread

	// unnamed pipe
	Return = CreatePipe(&read_pipe, &write_pipe, NULL, PIPE_BEFFERSIZE);

	if (Return)
	{
		// sorting a HANDLE to a read end of pipe
		thread_data.read_pipe = read_pipe;
		write_thread		  = GetCurrentThread();

		Return = DuplicateHandle(GetCurrentProcess(), write_thread, 
								 GetCurrentProcess(), &write_dupl, 
								 NULL, FALSE, DUPLICATE_SAME_ACCESS);

		thread_data.write_thread = write_dupl;

		if (Return)
		{
			// creates a reader thread, which act as a pipe client and
			// passing the ThreadData structure to the reader thread.
			read_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunction,
									   (LPVOID)&thread_data, CREATE_SUSPENDED, &thread_id);

			// writing data on the pipe
			for (int i = 0; i < 50; i++)
			{
				char  buffer[50] = {'\0'};
				int   lenght;
				DWORD write;  // data writen to pipe
				BOOL  ret = FALSE;

				lenght = sprintf_s(buffer, "This is line no:%i\n", i);
				ret	   = WriteFile(write_pipe, buffer, lenght, &write, NULL);
				assert(ret != NULL);

				if ((write == 18) || (write == 19))
				{
					// This acts as a toggle between the reader & writer thread.
					ResumeThread(read_thread);
					SuspendThread(GetCurrentThread());
				}
			}
		}
		CloseHandle(write_pipe);
		WaitForSingleObject(read_thread, INFINITE);
	}
	else assert(Return != FALSE);
	return 0;
}
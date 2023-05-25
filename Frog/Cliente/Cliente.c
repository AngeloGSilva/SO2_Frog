#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


//Pipe
#define PIPE_NAME TEXT("\\\\.\\pipe\\SERVER_CLIENTE")
#define N 10

//estrutura do named pipe
typedef struct {
	HANDLE hPipe;
	OVERLAPPED overlap;//estrutura overlapped para uso asincrono
	BOOL active;
}PipeData;


typedef struct {
	PipeData hPipes[N];
	HANDLE hEvent[N];
	HANDLE hMutex;
	int terminar;
};


int _tmain(int argc, TCHAR* argv[]) {

	int terminar = 0; // para acabar com tudo

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	HANDLE hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,PIPE_WAIT|PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,
		N,
		(nao sei),
		(nao sei),
		(nao sei),
		NULL
		);


	return 0;
}
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
	HANDLE hPipe, hEventPipe;
	TCHAR buf[256];
	DWORD n;
	int i;

	_tprintf(TEXT("[Escritor] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);
	//bloqueia aqui
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		exit(-1);
	}
	_tprintf(TEXT("[Escritor] Ligação ao pipe do leitor... (CreateFile)\n"));

	hPipe = CreateNamedPipe(PIPE_NAME,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}
	_tprintf(TEXT("[Escritor] Liguei-me...\n"));
	HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
	//aqui , o servidor já recebeu um cliente
	do {
		//vai buscar informação à consola
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(buf, 256, stdin);
		buf[_tcslen(buf) - 1] = '\0';

		//bloqueamos aqui porque é uma regiao critica
		WaitForSingleObject(mutex, INFINITE);

			if (!WriteFile(hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}

			_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao leitor [%d]... (WriteFile)\n"), n, i);
		//libertamos o mutex
		ReleaseMutex(mutex);


	} while (_tcscmp(buf, TEXT("fim")));


	return 0;
}
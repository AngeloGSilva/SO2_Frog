//#define MAX_CARS 10
//#define MAX_FROGS 10
//#define MAX_ROWS 20
//#define MAX_COLS 40
//
//
#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX_CARS 10
#define MAX_FROGS 10
#define MAX_ROWS 20
#define MAX_COLS 40


typedef struct {
	HANDLE sem;
	HANDLE event;
	HANDLE mutex;
	int num_cars;
	int num_frogs;
	int car_pos[MAX_CARS][2]; //2 seria para representar o x e o y
	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
	TCHAR map[MAX_ROWS][MAX_COLS];
}GameData;



int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, TEXT("TP_SEM"));
	if (hSem == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
		return 1;
	}
	// matriz de handles das threads
	//HANDLE hThreads[MAX_THREADS];

	_tprintf(TEXT("Waiting for slot...\n"));

	WaitForSingleObject(hSem, INFINITE);
	_tprintf(TEXT("Got in!\n"));

	GameData dados;

	dados.num_cars = 10;
	dados.num_frogs = 99;

	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), TEXT("TP_GameData"));
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("ERRO CreateFileMapping\n"));
		return 0;
	}




	TCHAR* pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("ERRO MapViewOfFile\n"));
		return 0;
	}

	dados.event = CreateEvent(NULL, TRUE, FALSE, TEXT("TP_Evento"));
	if (dados.event == NULL)
	{
		_tprintf(TEXT("ERRO CreateEvent\n"));
		return 0;
	}

	//criar mutex
	dados.mutex = CreateMutex(NULL, FALSE, TEXT("TP_Mutex"));
	if (dados.mutex == NULL)
	{
		_tprintf(TEXT("ERRO CreateMutex\n"));
		return 0;
	}
	
	_tprintf(TEXT("Enter the number of cars: "));
	_tscanf_s(TEXT("%d"), &dados.num_cars);
	_tprintf(TEXT("Enter the number of frogs: "));
	_tscanf_s(TEXT("%d"), &dados.num_frogs);

	WaitForSingleObject(dados.mutex, INFINITE);

	CopyMemory(pBuf,&dados,sizeof(GameData));

	//libertat o mutex
	ReleaseMutex(dados.mutex);

	//Criamos evento para que as threads ja consiga ler
	SetEvent(dados.event);

	//Sleep(500);
	ResetEvent(dados.event);

	while (1)
	{

	}


	//ReleaseSemaphore(hSem, 1, NULL);
	//UnmapViewOfFile(pBuf);
	return 0;
}
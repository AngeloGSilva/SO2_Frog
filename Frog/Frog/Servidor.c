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
	HANDLE Serv_HSem,Serv_HMutex,Serv_HEvent;
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

	GameData data;

	data.num_cars = 10;
	data.num_frogs = 99;

	for (int i = 0; i < MAX_ROWS; i++)
	{
		for (int j = 0; j < MAX_COLS; i++)
		{
			data.map[i][j] = '-';
		}
	}

	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), TEXT("TP_GameData"));
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("ERRO CreateFileMapping\n"));
		return 0;
	}


	GameData* pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("ERRO MapViewOfFile\n"));
		return 0;
	}

	data.Serv_HEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("TP_Evento"));
	if (data.Serv_HEvent == NULL)
	{
		_tprintf(TEXT("ERRO CreateEvent\n"));
		return 0;
	}

	//criar mutex
	data.Serv_HMutex = CreateMutex(NULL, FALSE, TEXT("TP_Mutex"));
	if (data.Serv_HMutex == NULL)
	{
		_tprintf(TEXT("ERRO CreateMutex\n"));
		return 0;
	}
	while (1)
	{
	
	_tprintf(TEXT("Enter the number of cars: "));
	_tscanf_s(TEXT("%d"), &data.num_cars);
	_tprintf(TEXT("Enter the number of frogs: "));
	_tscanf_s(TEXT("%d"), &data.num_frogs);

	WaitForSingleObject(data.Serv_HMutex, INFINITE);

	ZeroMemory(pBuf, sizeof(GameData));
	CopyMemory(pBuf,&data,sizeof(GameData));

	//libertat o mutex
	ReleaseMutex(data.Serv_HMutex);

	//Criamos evento para que as threads ja consiga ler
	SetEvent(data.Serv_HEvent);

	Sleep(500);
	ResetEvent(data.Serv_HEvent);


	}


	//ReleaseSemaphore(hSem, 1, NULL);
	//UnmapViewOfFile(pBuf);
	return 0;
}
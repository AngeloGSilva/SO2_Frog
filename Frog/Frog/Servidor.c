#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "SharedMemory.h"

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	TCHAR BlockElement = 95;
	TCHAR CarElement = 72;

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

	for (int i = 0; i < MAX_ROWS; i++)
	{
		for (int j = 0; j < MAX_COLS; j++)
		{
			if (i == 3 && j == 5)
				data.map[i][j] = CarElement;
			else
				data.map[i][j] = BlockElement;
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
		for (int i = 0; i < MAX_ROWS; i++)
		{
			int next_col = 0;
			for (int j = 0; j < MAX_COLS; j++)
			{
				if (data.map[i][j] == CarElement) {
					data.map[i][j] = BlockElement;
					next_col = (j + 1) % MAX_COLS;
					data.map[i][next_col] = CarElement;
					break;
				}
			}
		}
		Sleep(1000);
		WaitForSingleObject(data.Serv_HMutex, INFINITE);

		ZeroMemory(pBuf, sizeof(GameData));
		CopyMemory(pBuf, &data, sizeof(GameData));

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
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "Utils.h"
#include "Struct.h"
#include "Registry.h"
#include "SharedMemory.h"
#include <time.h>
#include <stdlib.h>


int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	//rand
	srand((unsigned)time(NULL));

	GameData data = RegistryKeyValue();

	_tprintf(TEXT("car: %d,speed: %d\n"), data.num_cars, data.carSpeed);

	//criar Semaf
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, SEMAPHORE_UNIQUE_SERVER);
	if (hSem == NULL)
	{
		_tprintf("[ERRO]CreateSemaphore: %d\n", GetLastError());
		return 1;
	}
	// matriz de handles das threads
	//HANDLE hThreads[MAX_THREADS];

	_tprintf(TEXT("Waiting for slot...\n"));

	WaitForSingleObject(hSem, INFINITE);
	_tprintf(TEXT("Got in!\n"));

	//desenho do mapa
	for (int i = 0; i < MAX_ROWS + 4; i++)
	{
		for (int j = 0; j < MAX_COLS; j++)
		{
			if (i == 0 || j == 0 || i == MAX_ROWS + 3 || j == MAX_COLS - 1)
				data.map[i][j] = BLOCK_ELEMENT;
			else if (i == 1 || i == MAX_ROWS + 2) {
				data.map[i][j] = BEGIN_END_ELEMENT;
			}else
				data.map[i][j] = ROAD_ELEMENT;
		}
	}

	//Gerar carros
	for (int i = 2; i < MAX_ROWS + 2; i++)
	{
		for (int cars = (rand() % 8) + 1; cars >= 0; cars--) {
			int ra = (rand() % (MAX_COLS - 2)) + 1;
			data.map[i][ra] = CAR_ELEMENT;
			_tprintf(TEXT("cars:%d\n"),ra);
		}
	}
	
	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), FILE_MAPPING_GAME_DATA);
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("[ERRO] CreateFileMapping\n"));
		return 0;
	}

	pGameData pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("[ERRO] MapViewOfFile\n"));
		return 0;
	}

	//criar Event
	data.Serv_HEvent = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORIE_EVENT);
	if (data.Serv_HEvent == NULL)
	{
		_tprintf(TEXT("[ERRO]CreateEvent\n"));
		return 0;
	}

	//criar mutex
	data.Serv_HMutex = CreateMutex(NULL, FALSE, SHARED_MEMORIE_MUTEX);
	if (data.Serv_HMutex == NULL)
	{
		_tprintf(TEXT("[ERRO]CreateMutex\n"));
		return 0;
	}
	while (1)
	{
		for (int i = 0; i < MAX_ROWS + 4; i++)
		{
			int next_col = 0;
			for (int j = 0; j < MAX_COLS; j++)
			{
				if (data.map[i][j] == CAR_ELEMENT) {
					//next_col = (j + 1) % MAX_COLS;
					if (j + 1 != MAX_COLS - 1) {
						data.map[i][j + 1] = CAR_ELEMENT;
						data.map[i][j] = ROAD_ELEMENT;
					}
					else if(j + 1 == MAX_COLS - 1){
						data.map[i][1] = CAR_ELEMENT;
						data.map[i][j] = ROAD_ELEMENT;
					}
				}
			}
			//adicionar ao array de posições de carros e gerar as posições no mapa de uma vez
		}
		Sleep(data.carSpeed);
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
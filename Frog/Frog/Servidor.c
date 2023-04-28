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
#include <string.h>

#define MAX_ROADS_THREADS 50

//typedef struct {
//	int col;
//	int row;
//}CarPos, *pCarPos;

typedef struct {
	int speed;
	int numCars;
	pCarPos car_pos;
	TCHAR* Map;
	BOOLEAN direction;
	pCarPos sharedCarPos;
	TCHAR* sharedMap;
	HANDLE hEventRoads, hMutex;
	int id;
} TRoads, *pTRoads;


DWORD WINAPI ThreadRoads(LPVOID lpParam)
{
	pTRoads data = (pTRoads)lpParam;
	pCarPos temp;
	_tprintf(TEXT("INICIO DA THREAD %d\n"), data->id);

	while (1)
	{
		WaitForSingleObject(data->hMutex, INFINITE);
		temp = data->car_pos;

		_tprintf(TEXT("thread %d comecou\n"),data->id);
		//movimento carros direita esquerda
		for (int i = 0; i < data->numCars; i++)
		{
			int x = temp[i].row;
			if (x == data->id)
			{
				int y = temp[i].col;
				data->Map[x * MAX_COLS + y] = ' ';

				if (y - 1 == 0) {
					temp[i].col = MAX_COLS - 2;
				}
				else
					temp[i].col--;
			}
		}
		for (int i = 0; i < data->numCars; i++)
				{
					int x = data->car_pos[i].col;
					int y = data->car_pos[i].row;
					_tprintf(TEXT("x:%d\n"),x);
					_tprintf(TEXT("y:%d\n"),y);
					data->Map[y * MAX_COLS + x] = CAR_ELEMENT;
				}
		//copiar o conteudo para a memoria partilhada
		_tprintf(TEXT("temppppp1 %c\n"), data->Map[2]);
		CopyMemory(data->sharedMap, data->Map, sizeof(TCHAR) * MAX_ROWS * (MAX_COLS + 4));
		_tprintf(TEXT("temppppp2 %c\n"), data->sharedMap[2]);

		ReleaseMutex(data->hMutex);
		_tprintf(TEXT("thread %d acabou\n"), data->id);
		//Criamos evento para que as threads ja consiga ler
		SetEvent(data->hEventRoads);

		Sleep(500);
		ResetEvent(data->hEventRoads);
		Sleep(data->speed);
	}
	
	////atualizar mapa
	//for (int i = 0; i < data->Game.numCars; i++)
	//{
	//	int x = data->Game.car_pos[i][0];
	//	int y = data->Game.car_pos[i][1];
	//	_tprintf(TEXT("x:%d\n"), x);
	//	_tprintf(TEXT("y:%d\n"), y);
	//	data->Game.map[x][y] = CAR_ELEMENT;
	//}

	return 0;
}


DWORD WINAPI ThreadBufferCircular(LPVOID lpParam)
{
	pTDados dados = (pTDados)lpParam;
	EspacoBuffer space;
	space.id = 0;
	space.val = 0;
	while (1)
	{
		WaitForSingleObject(dados->hSemLeitura, INFINITE);
		WaitForSingleObject(dados->hMutex, INFINITE);
		//copiar o conteudo para a memoria partilhada
		CopyMemory(&space, &dados->BufferCircular->espacosDeBuffer[dados->BufferCircular->posLeitura], sizeof(EspacoBuffer));
		dados->BufferCircular->posLeitura++;
		if (dados->BufferCircular->posLeitura == 10)
		{
			dados->BufferCircular->posLeitura = 0;
		}
		//_tprintf(TEXT("Consumidor: id do Produtor: %d comeu %d\n"), space.id, space.val);
		ReleaseMutex(dados->hMutex);
		ReleaseSemaphore(dados->hSemEscrita, 1, NULL);
	}
	return 0;
}

int parse_args(TCHAR* arg1_str, TCHAR* arg2_str, DWORD* arg1, DWORD* arg2) {
	_tprintf(TEXT("%s %s \n"), arg1_str, arg2_str);

	*arg1 = atoi(arg1_str);
	*arg2 = atoi(arg2_str);

	if (!arg1 || !arg2) {
		_tprintf(TEXT("Invalid argument format\n"));
		return 1;
	}

	_tprintf(TEXT("arg1 = %d\n"), *arg1);
	_tprintf(TEXT("arg2 = %d\n"), *arg2);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	//NAO APAGAR, IMPORTANTE 
	//if (argc != 3) { 
	//	_tprintf(TEXT("Bad usage of parameters \n"));
	//	return 1;
	//}

	//DWORD arg1, arg2, parse_result;
	//parse_result = parse_args(argv[1], argv[2], &arg1, &arg2);
	//if (parse_result != 0) {
	//	_tprintf(TEXT("Failed to parse arguments\n"));
	//	return 1;
	//}

	//_tprintf(TEXT("Good parse!\n arg1 :%d \narg2 :%d \n"),arg1,arg2);

	srand((unsigned)time(NULL));

	GameData data = RegistryKeyValue();

	TDados dataThread;

	dataThread.hMutex = CreateMutex(NULL, FALSE, BUFFER_CIRCULAR_MUTEX_LEITORE);
	dataThread.hSemEscrita = CreateSemaphore(NULL, 10, 10, BUFFER_CIRCULAR_SEMAPHORE_ESCRITOR);
	dataThread.hSemLeitura = CreateSemaphore(NULL, 0, 10, BUFFER_CIRCULAR_SEMAPHORE_LEITORE);

	HANDLE HMapFileBuffer = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FILE_MAPPING_BUFFER_CIRCULAR);
	if (HMapFileBuffer == NULL)
	{
		_tprintf(TEXT("CreateFileMapping\n"));
		HMapFileBuffer = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Buffer), TEXT("SO2_BUFFERCIRCULAR"));
		dataThread.BufferCircular = (pBuffer)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		dataThread.BufferCircular->nConsumidores = 0;
		dataThread.BufferCircular->nProdutores = 0;
		dataThread.BufferCircular->posEscrita = 0;
		dataThread.BufferCircular->posLeitura = 0;

		if (HMapFileBuffer == NULL)
		{
			_tprintf(TEXT("ERRO CreateFileMapping\n"));
			return 0;
		}
	}
	else
	{
		dataThread.BufferCircular = (Buffer*)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (HMapFileBuffer == NULL)
		{
			_tprintf(TEXT("ERRO CreateFileMapping\n"));
			return 0;
		}
	}
	dataThread.id = dataThread.BufferCircular->nConsumidores++;

	HANDLE hThreads = CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		ThreadBufferCircular, // Thread start address
		&dataThread,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);   // Thread id   // returns the thread identifier 


	_tprintf(TEXT("car: %d,speed: %d\n"), data.numCars, data.carSpeed);

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
				data.map[i][j] = ' ';
		}
	}


	//Colocar tudo a zero... numRoads fix size for debug
	//apagar prints after testes
	data.numRoads = 8;
	data.numCars = 0;
	_tprintf(TEXT("NumRoads %d\n"), data.numRoads);

	//Gerar carros
	for (int i = 0; i < data.numRoads; i++) 
	{
		int carsInRoad = (rand() % 8) + 1;
		for (;carsInRoad >= 0; carsInRoad--) {
			data.car_pos[data.numCars].row = i + 2; //X -> linha
			int posInRoad = 0;
			do {
				posInRoad = (rand() % (MAX_COLS - 2)) + 1;
			} while (data.map[i][posInRoad] == CAR_ELEMENT);
			data.car_pos[data.numCars].col = posInRoad; //y -> coluna
			data.map[i][posInRoad] == CAR_ELEMENT;
			data.numCars++;
		}
	}
	_tprintf(TEXT("NUMERO TOTAL DE CARS %d\n"),data.numCars);

	
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

		WaitForSingleObject(data.Serv_HMutex, INFINITE);

		ZeroMemory(pBuf, sizeof(GameData));
		CopyMemory(pBuf, &data, sizeof(GameData));

		//libertat o mutex
		ReleaseMutex(data.Serv_HMutex);

		//Criamos evento para que as threads ja consiga ler
		SetEvent(data.Serv_HEvent);

		//Sleep(500);
		ResetEvent(data.Serv_HEvent);



		//Gerar Threads das Roads
		HANDLE mutex_ROADS;

		// matriz de handles das threads
		HANDLE RoadThreads[MAX_ROADS_THREADS];

		TRoads RoadsData[MAX_ROADS_THREADS];


		_tprintf(TEXT("[DEBUG] NUM ROADS %d criada\n"), data.numRoads);
		for (int i = 0; i < data.numRoads; i++)
		{
			HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * MAX_ROWS * (MAX_COLS + 4), TEXT("SO2_MAP_OLA"));
			if (HMapFile == NULL)
			{
				_tprintf(TEXT("ERRO CreateFileMapping\n"));
				return 0;
			}
			_tprintf(TEXT("Criado CreateFileMapping\n"));
			RoadsData[i].sharedMap = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (RoadsData[i].sharedMap == NULL)
			{
				_tprintf(TEXT("ERRO MapViewOfFile\n"));
				return 0;
			}

			RoadsData[i].Map = &data.map;
			RoadsData[i].car_pos = &data.car_pos;
			RoadsData[i].numCars = data.numCars;
			RoadsData[i].hMutex = CreateMutex(NULL, FALSE, TEXT("MUTEX_ROADS"));
			RoadsData[i].hEventRoads = CreateEvent(NULL, TRUE, FALSE, TEXT("EVENT_ROADS"));
			RoadsData[i].id = i + 2; //o numero do id é a estrada q elas estao encarregues
			RoadsData[i].speed = ((rand() % 8) + 1) * 1000;
			RoadsData[i].direction = 1;
			RoadThreads[i] = CreateThread(
				NULL,    // Thread attributes
				0,       // Stack size (0 = use default)
				ThreadRoads, // Thread start address
				&RoadsData[i],    // Parameter to pass to the thread
				0,       // Creation flags
				NULL);   // Thread id   // returns the thread identifier 
			_tprintf(TEXT("[DEBUG] Thread estrada %d criada\n"), i);

		}

		while (1)
		{

		}

		// FAZER aguarda / controla as threads 
		//       manda as threads parar
		//WaitForMultipleObjects(data.numRoads, RoadThreads, TRUE, INFINITE);

		
	//}
	CloseHandle(hThreads);
	//ReleaseSemaphore(hSem, 1, NULL);
	//UnmapViewOfFile(pBuf);
	return 0;
}
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

typedef struct {
	int speed;
	BOOLEAN direction;
	pGameData Game;
	HANDLE hSemRoads, hMutex;
	int id;
} TRoads, * pTRoads;


DWORD WINAPI ThreadRoads(LPVOID lpParam)
{
	pTRoads data = (pTRoads)lpParam;
	_tprintf(TEXT("n de carros%d\n"), data->Game->numCars);
	while (1)
	{
		WaitForSingleObject(data->hMutex, INFINITE);
		_tprintf(TEXT("thread %d comecou\n"),data->id);
		//movimento carros direita esquerda
		for (int i = 0; i < data->Game->numCars; i++)
		{
			int x = data->Game->car_pos[i][0];
			if (x == data->id)
			{
				_tprintf(TEXT("carro: %d da thread %d\n"), x, data->id);
				int y = data->Game->car_pos[i][1];
				data->Game->map[x][y] = ROAD_ELEMENT;
				if (data->Game->car_pos[i][1] - 1 == 0)
					data->Game->car_pos[i][1] = MAX_COLS - 2;
				else
					data->Game->car_pos[i][1]--;
				data->Game->map[x][y] = CAR_ELEMENT;
				_tprintf(TEXT("Thread da estrada %d: fez o carro andar\n"), data->id);
			}
		}

		ReleaseMutex(data->hMutex);
		_tprintf(TEXT("thread %d acabou\n"), data->id);
		Sleep(((rand() % 8) + 1)*2000);

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
		_tprintf(TEXT("Consumidor: id do Produtor: %d comeu %d\n"), space.id, space.val);
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
				data.map[i][j] = ROAD_ELEMENT;
		}
	}


	//Colocar tudo a zero... numRoads fix size for debug
	//apagar prints after testes
	data.numRoads = 3;
	data.numCars = 0;
	_tprintf(TEXT("NumRoads %d\n"), data.numRoads);



	//Gerar carros
	for (int i = 0; i < data.numRoads; i++) 
	{
		int carsInRoad = (rand() % 8) + 1;
		_tprintf(TEXT("CarInRoad %d\n"), carsInRoad);
		//data.numCars = data.numCars + carsInRoad;
		for (;carsInRoad >= 0; carsInRoad--) {
			_tprintf(TEXT("numCars antes %d\n"), data.numCars);
			
			_tprintf(TEXT("numCars depois %d\n"), data.numCars);
			data.car_pos[data.numCars][0] = i + 2; //X -> linha
			_tprintf(TEXT("car na linha %d\n"), data.car_pos[data.numCars][0]);
			int posInRoad = 0;
			do {
				posInRoad = (rand() % (MAX_COLS - 2)) + 1;
			} while (data.map[i][posInRoad] == CAR_ELEMENT);

			data.car_pos[data.numCars][1] = posInRoad; //y -> coluna
			_tprintf(TEXT("car na coluna %d\n"), data.car_pos[data.numCars][1]);
			data.map[i][posInRoad] == CAR_ELEMENT;
			_tprintf(TEXT("car criado nesta pos:%d , %d\n"), data.car_pos[data.numCars][0], data.car_pos[data.numCars][1]);
			data.numCars++;
		}
	}


	HANDLE mutex_ROADS;

	// matriz de handles das threads
	HANDLE RoadThreads[MAX_ROADS_THREADS];

	TRoads RoadsData[MAX_ROADS_THREADS];



	for (int i = 0; i < data.numRoads; i++) 
	{
		_tprintf(TEXT("[~DEBUG] Thread estrada %d criada\n"),i);
		RoadsData[i].hMutex = CreateMutex(NULL, FALSE, TEXT("MUTEX_ROADS"));
		RoadsData[i].Game = &data;
		RoadsData[i].id = i + 2; //o numero do id é a estrada q elas estao encarregues
		RoadsData[i].speed = 0;
		RoadsData[i].direction = 1;
		RoadThreads[i] = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			ThreadRoads, // Thread start address
			&RoadsData[i],    // Parameter to pass to the thread
			CREATE_SUSPENDED,       // Creation flags
			NULL);   // Thread id   // returns the thread identifier 
	}

	for (int i = 0; i < data.numRoads; i++)
	{
		_tprintf(TEXT("[~DEBUG] Thread estrada %d iniciada\n"), i);
		ResumeThread(RoadThreads[i]);
	}

	// FAZER aguarda / controla as threads 
	//       manda as threads parar
	WaitForMultipleObjects(data.numRoads, RoadThreads, TRUE, INFINITE);

	////Gerar carros
	//for (int i = 2; i < MAX_ROWS + 2; i++)
	//{
	//	for (int cars = (rand() % 8) + 1; cars >= 0; cars--) {
	//		int ra = (rand() % (MAX_COLS - 2)) + 1;
	//		data.map[i][ra] = CAR_ELEMENT;
	//		_tprintf(TEXT("cars:%d\n"),ra);
	//	}
	//}
	
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

	//while (1)
	//{
	//	//movimento carros esquerda direita
	//	/*for (int i = 0; i < data.numCars; i++)
	//	{
	//		int x = data.car_pos[i][0];
	//		int y = data.car_pos[i][1];
	//		data.map[x][y] = ROAD_ELEMENT;
	//		if (data.car_pos[i][1] + 2 == MAX_COLS)
	//			data.car_pos[i][1] = 1;
	//		else  
	//			data.car_pos[i][1]++;
	//	}*/

	//	//movimento carros direita esquerda
	//	for (int i = 0; i < data.numCars; i++)
	//	{
	//		int x = data.car_pos[i][0];
	//		int y = data.car_pos[i][1];
	//		data.map[x][y] = ROAD_ELEMENT;
	//		if (data.car_pos[i][1] - 1 == 0)
	//			data.car_pos[i][1] = MAX_COLS - 2;
	//		else
	//			data.car_pos[i][1]--;
	//	}
	//	//atualizar mapa
	//	for (int i = 0; i < data.numCars; i++)
	//	{
	//		int x = data.car_pos[i][0];
	//		int y = data.car_pos[i][1];
	//		_tprintf(TEXT("x:%d\n"),x);
	//		_tprintf(TEXT("y:%d\n"),y);
	//		data.map[x][y] = CAR_ELEMENT;
	//	}

	//	//for (int i = 0; i < MAX_ROWS + 4; i++)
	//	//{
	//	//	int next_col = 0;
	//	//	for (int j = 0; j < MAX_COLS; j++)
	//	//	{
	//	//		if (data.map[i][j] == CAR_ELEMENT) {
	//	//			//next_col = (j + 1) % MAX_COLS;
	//	//			if (j + 1 != MAX_COLS - 1) {
	//	//				data.map[i][j + 1] = CAR_ELEMENT;
	//	//				data.map[i][j] = ROAD_ELEMENT;
	//	//			}
	//	//			else if(j + 1 == MAX_COLS - 1){
	//	//				data.map[i][1] = CAR_ELEMENT;
	//	//				data.map[i][j] = ROAD_ELEMENT;
	//	//			}
	//	//		}
	//	//	}
	//	//	//adicionar ao array de posições de carros e gerar as posições no mapa de uma vez
	//	//}

	//	Sleep(data.carSpeed);
	//	WaitForSingleObject(data.Serv_HMutex, INFINITE);

	//	ZeroMemory(pBuf, sizeof(GameData));
	//	CopyMemory(pBuf, &data, sizeof(GameData));

	//	//libertat o mutex
	//	ReleaseMutex(data.Serv_HMutex);

	//	//Criamos evento para que as threads ja consiga ler
	//	SetEvent(data.Serv_HEvent);

	//	Sleep(500);
	//	ResetEvent(data.Serv_HEvent);
	//}
	CloseHandle(hThreads);
	//ReleaseSemaphore(hSem, 1, NULL);
	//UnmapViewOfFile(pBuf);
	return 0;
}
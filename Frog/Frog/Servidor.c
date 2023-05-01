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
				if (data->direction[data->id] == ROAD_RIGHT) {
					if (data->Map[x * MAX_COLS + y + 1] != OBSTACLE_ELEMENT ) {
						//&& data->Map[x * MAX_COLS + y + 1] != CAR_ELEMENT
						if (y + 1 == 19) {
							temp[i].col = 1;
						}
						else
							temp[i].col++;
					}
					else {
						//IDK
						_tprintf(TEXT("IALSD"));
					}
				}
				else if (data->direction == ROAD_LEFT) {
					if (data->Map[x * MAX_COLS + y - 1] != OBSTACLE_ELEMENT) {
						// && data->Map[x * MAX_COLS + y + 1] != CAR_ELEMENT
						if (y - 1 == 0) {
							temp[i].col = MAX_COLS - 2;
						}
						else
							temp[i].col--;
					}
					else {
					//IDK
					}
				}
			}
		}

		for (int i = 1; i < MAX_COLS - 1; i++)
		{
			if (data->Map[data->id * MAX_COLS + i] == OBSTACLE_ELEMENT)
			{
				data->Map[data->id * MAX_COLS + i] = OBSTACLE_ELEMENT;
			}
			else
			{
				data->Map[data->id * MAX_COLS + i] = ROAD_ELEMENT;
			}

		}


		// refazer mapa (apenas a linha)
		for (int i = 0; i < data->numCars; i++)
		{
			int x = temp[i].row;
			if (x == data->id)
			{
				data->Map[temp[i].row * MAX_COLS + temp[i].col] = CAR_ELEMENT;
			}
		}

		//copiar o conteudo para a memoria partilhada
		_tprintf(TEXT("temppppp1 %c\n"), data->Map[2]);
		CopyMemory(data->sharedMap, data->Map, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
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

DWORD WINAPI CheckOperators(LPVOID lpParam)
{
	while(1) {
		//Criamos evento para que as threads ja consiga ler

		HANDLE x = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORIE_EVENT);
		if (x == NULL)
		{
			_tprintf(TEXT("[ERRO]CreateEvent\n"));
			return 0;
		}
		SetEvent(x);
		ResetEvent(x);

		Sleep(10000);
	}
}

DWORD WINAPI ThreadBufferCircular(LPVOID lpParam)
{
	pTDados dados = (pTDados)lpParam;
	EspacoBuffer space;
	space.id = 0;
	//space.val = 0;
	while (1)
	{
		WaitForSingleObject(dados->hSemLeitura, INFINITE);
		WaitForSingleObject(dados->hMutex, INFINITE);
		//copiar o conteudo para a memoria partilhada
		_tprintf(TEXT("VOU PARA SEU CORNO OPOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"));
		CopyMemory(&space, &dados->BufferCircular->espacosDeBuffer[dados->BufferCircular->posLeitura], sizeof(EspacoBuffer));
		dados->BufferCircular->posLeitura++;
		if (dados->BufferCircular->posLeitura == 10)
		{
			dados->BufferCircular->posLeitura = 0;
		}

		dados->RoadsDirection[1] = ROAD_LEFT;



		//acrescentar pedra
		//dados->Mapv2[4 * MAX_COLS + 14] = OBSTACLE_ELEMENT;

		//para alterar a direcao, fazer um array com as threads com a estrada e a direcao e alterar quando for necessario, necessario usar mutex no acesso a esse array

		//_tprintf(TEXT("ESTA PARA A %d"),dados->RoadsData[0].direction);

		//Parar o tempo
		//for (int i = 0; i < dados->numRoads; i++)
		//{
		//	SuspendThread(dados->threadsHandles[i]);
		//}

		//Sleep(10000);

		//for (int i = 0; i < dados->numRoads; i++)
		//{
		//	ResumeThread(dados->threadsHandles[i]);
		//}

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
	data.numRoads = 3;
	data.numCars = 0;
	//desenho do mapa
	for (int i = 0; i < data.numRoads + SKIP_BEGINING_END; i++)
	{
		for (int j = 0; j < MAX_COLS; j++)
		{
			if (i == 0 || j == 0 || i == data.numRoads + 3 || j == MAX_COLS - 1)
				data.map[i][j] = BLOCK_ELEMENT;
			else if (i == 1 || i == data.numRoads + SKIP_BEGINING) {
				data.map[i][j] = BEGIN_END_ELEMENT;
			}else
				data.map[i][j] = ROAD_ELEMENT;

			if (i == 2 && j == 6)
				data.map[i][j] = OBSTACLE_ELEMENT;
		}
	}

	data.map;
	//Colocar tudo a zero... numRoads fix size for debug
	//apagar prints after testes
	
	_tprintf(TEXT("NumRoads %d\n"), data.numRoads);

	//Gerar carros
	for (int i = 0; i < data.numRoads; i++) 
	{
		int carsInRoad = 8; //(rand() % 8) + 1
		for (;carsInRoad > 0; carsInRoad--) {
			data.car_pos[data.numCars].row = i + SKIP_BEGINING; //X -> linha
			int posInRoad = 0;
			do {
				posInRoad = (rand() % (MAX_COLS - 2)) + 1;
			}while (data.map[i + SKIP_BEGINING][posInRoad] == 'H');
			_tprintf(TEXT("PUS AQUI %d %d\n"), posInRoad, i + SKIP_BEGINING);
			data.car_pos[data.numCars].col = posInRoad; //y -> coluna
			data.map[i + SKIP_BEGINING][posInRoad] = CAR_ELEMENT;
			data.numCars++;
		}
	}
	_tprintf(TEXT("NUMERO TOTAL DE CARS %d\n"),data.numCars);

	data.map;
	
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
	CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		CheckOperators, // Thread start address
		NULL,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);

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

		//criar a thread

		//Gerar Threads das Roads
		HANDLE mutex_ROADS;

		// matriz de handles das threads
		HANDLE RoadThreads[MAX_ROADS_THREADS];

		TRoads RoadsData[MAX_ROADS_THREADS];

		_tprintf(TEXT("[DEBUG] NUM ROADS %d criada\n"), data.numRoads);
		for (int i = 0; i < data.numRoads; i++)
		{
			HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS, TEXT("SO2_MAP_OLA"));
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
			RoadsData[i].hEventRoads = CreateEvent(NULL, TRUE, FALSE, TEXT("EVENT_ROADS") + i);
			RoadsData[i].id = i + SKIP_BEGINING; //o numero do id é a estrada q elas estao encarregues
			RoadsData[i].speed = ((rand() % 8) + 1) * 1000;
			RoadsData[i].direction[i] = ROAD_RIGHT;//(rand() % 1)
			RoadThreads[i] = CreateThread(
				NULL,    // Thread attributes
				0,       // Stack size (0 = use default)
				ThreadRoads, // Thread start address
				&RoadsData[i],    // Parameter to pass to the thread
				CREATE_SUSPENDED,       // Creation flags
				NULL);   // Thread id   // returns the thread identifier 
			_tprintf(TEXT("[DEBUG] Thread estrada %d criada\n"), i);
		}


		//BufferCircular
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
			dataThread.RoadsDirection = &RoadsData->direction;
			//dataThread.Mapv2 = &data.map;
			dataThread.threadsHandles = &RoadThreads;
			dataThread.numRoads = data.numRoads;

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












		HANDLE InitialEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("INITIAL EVENT"));

		WaitForSingleObject(InitialEvent, INFINITE);
		for (int i = 0; i < data.numRoads; i++)
			ResumeThread(RoadThreads[i]);






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
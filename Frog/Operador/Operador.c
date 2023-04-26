#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"
#include "SharedMemory.h"
#include <time.h>
#include <stdlib.h>

#define MAX_ROADS_THREADS 50
typedef struct {
	int speed;
	BOOLEAN direction;
	pGameData Game;
	HANDLE hEventRoads, hMutex;
	int id;
} TRoads, * pTRoads;



DWORD WINAPI ThreadRoads(LPVOID lpParam)
{
	pTRoads data = (pTRoads)lpParam;
	pGameData temp;
	_tprintf(TEXT("n de carros%d\n"), data->Game->numCars);
	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), TEXT("SO2_MAP") + data->id);
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("ERRO CreateFileMapping\n"));
		return 0;
	}

	temp = (GameData*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("ERRO MapViewOfFile\n"));
		return 0;
	}
	while (1)
	{
		WaitForSingleObject(data->hEventRoads, INFINITE);
		WaitForSingleObject(data->hMutex, INFINITE);
		_tprintf(TEXT("thread %d comecou\n"), data->id);
		//movimento carros direita esquerda
		for (int i = 0; i < MAX_COLS; i++)
		{
			_tprintf(TEXT("%c"), temp->map[data->id][i]);
		}

		ReleaseMutex(data->hMutex);
		_tprintf(TEXT("thread %d acabou\n"), data->id);
		Sleep(((rand() % 8) + 1) * 1000);
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
	pTDados dados = (pTDados) lpParam;
	EspacoBuffer space;
	while (1)
	{
		space.id = dados->id;
		space.val = (rand() % 9) + 1;

		WaitForSingleObject(dados->hSemEscrita, INFINITE);
		WaitForSingleObject(dados->hMutex, INFINITE);
		//copiar o conteudo para a memoria partilhada
		CopyMemory(&dados->BufferCircular->espacosDeBuffer[dados->BufferCircular->posEscrita], &space, sizeof(EspacoBuffer));
		dados->BufferCircular->posEscrita++;
		if (dados->BufferCircular->posEscrita == 10)
		{
			dados->BufferCircular->posEscrita = 0;
		}

		_tprintf(TEXT("Produtor %d fez %d\n"), dados->id, space.val);
		ReleaseMutex(dados->hMutex);
		ReleaseSemaphore(dados->hSemLeitura, 1, NULL);
		Sleep(((rand() % 4) + 1) * 1000);
	}
	return 0;
}

//ideias para tratar do mapa.... 
//fazer uma thread separada do main que trate so do desenho do mapa
//movimentos do sapo sera algo semelhante




int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	//rand
	srand((unsigned)time(NULL));


	GameData data;


	TDados dataThread;

	dataThread.hMutex = CreateMutex(NULL, FALSE, BUFFER_CIRCULAR_MUTEX_ESCRITOR);
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
	dataThread.id = dataThread.BufferCircular->nProdutores++;

	HANDLE hThreads = CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		ThreadBufferCircular, // Thread start address
		&dataThread,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);   // Thread id   // returns the thread identifier 


	//data tem de sair e so ficar pbuf penso eu
	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), FILE_MAPPING_GAME_DATA);
	pGameData pBuf = (GameData*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//data.event = OpenEvent(READ_CONTROL,TRUE, TEXT("TP_Evento"));
	data.Serv_HEvent = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORIE_EVENT);
	//data.mutex = OpenMutex(READ_CONTROL, TRUE, TEXT("TP_Mutex"));
	data.Serv_HMutex = CreateMutex(NULL, FALSE, SHARED_MEMORIE_MUTEX);




	//while (1)
	//{
		WaitForSingleObject(data.Serv_HEvent, INFINITE);

		WaitForSingleObject(data.Serv_HMutex, INFINITE);

		system("cls");//nao sei ate q ponto é bom usar isto

		//_tprintf(TEXT("Mensagem Recebida: %d %d\n"), pBuf->num_cars,pBuf->num_frogs);
		//for (int i = 0; i < MAX_ROWS + 4; i++)
		//{
		//	for (int j = 0; j < MAX_COLS; j++)
		//	{
		//		_tprintf(TEXT("%c"), pBuf->map[i][j]);
		//	}
		//	_tprintf("\n");
		//}



		//libertat o mutex
		ReleaseMutex(data.Serv_HMutex);

		HANDLE mutex_ROADS;

		// matriz de handles das threads
		HANDLE RoadThreads[MAX_ROADS_THREADS];

		TRoads RoadsData[MAX_ROADS_THREADS];

		_tprintf(TEXT("[DEBUG] Thread estrada %d criada\n"), pBuf->numCars);
		for (int i = 0; i < pBuf->numRoads; i++)
		{
			_tprintf(TEXT("[DEBUG] Thread estrada %d criada\n"), i);
			RoadsData[i].hMutex = CreateMutex(NULL, FALSE, TEXT("MUTEX_ROADS") + i);
			RoadsData[i].hEventRoads = CreateEvent(NULL, TRUE, FALSE, TEXT("EVENT_ROADS") + i);
			_tprintf(TEXT("EVENT_ROADS") + i);
			RoadsData[i].Game = pBuf;
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


		for (int i = 0; i < pBuf->numRoads; i++)
		{
			_tprintf(TEXT("[DEBUG] Thread estrada %d iniciada\n"), i);
			ResumeThread(RoadThreads[i]);
		}


		//Sleep(pBuf->carSpeed);
	/*}*/

	CloseHandle(hThreads);
	return 0;
}
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

DWORD WINAPI ThreadRoads(LPVOID lpParam)
{
	pTRoads data = (pTRoads)lpParam;
	TCHAR* temp;
	while (1)
	{
		WaitForSingleObject(data->hEventRoads, INFINITE);
		WaitForSingleObject(data->hMutex, INFINITE);
		CopyMemory(&temp, &data->sharedMap, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD cursorPos;
		for (int i = 0; i < MAX_COLS; i++)
		{
			DWORD numWritten; // Number of characters actually written
			cursorPos.X = i;
			cursorPos.Y = data->id;
			//_tprintf(TEXT("coluna do carro %d :%d\n"), i,temp[i].col);
			SetConsoleCursorPosition(hConsole, cursorPos);
			WriteConsole(hConsole, &temp[data->id * MAX_COLS + i], 1, &numWritten, NULL);
		}
		ReleaseMutex(data->hMutex);
	}
	return 0;
}

DWORD WINAPI ThreadBeginEnd(LPVOID lpParam)
{
	pTStartEnd data = (pTStartEnd)lpParam;
	TCHAR* temp;
	while (1)
	{
		WaitForSingleObject(data->hMutex, INFINITE);
		CopyMemory(&temp, &data->sharedMap, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD cursorPos;
		for (int r = 0; r < 4; r++) {
			int aux = r;
			if (r > 1)
				aux = r + data->numRoads;
			for (int i = 0; i < MAX_COLS; i++)
			{
				DWORD numWritten; // Number of characters actually written
				cursorPos.X = i;
				cursorPos.Y = aux;
				SetConsoleCursorPosition(hConsole, cursorPos);
				WriteConsole(hConsole, &temp[aux * MAX_COLS + i], 1, &numWritten, NULL);
			}
		}
		ReleaseMutex(data->hMutex);
	}
	return 0;
}

DWORD WINAPI ThreadBufferCircular(LPVOID lpParam)
{
	pTDados dados = (pTDados)lpParam;
	EspacoBuffer space;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursorPos;
	while (1)
	{
		Sleep(10000);
		//DWORD numWritten; // Number of characters actually written
		//cursorPos.X = 0;
		//cursorPos.Y = 20;
		////_tprintf(TEXT("coluna do carro %d :%d\n"), i,temp[i].col);
		//SetConsoleCursorPosition(hConsole, cursorPos);
		//WriteConsole(hConsole, TEXT("<-COMANDO->"), 1, &numWritten, NULL);

		space.id = dados->id;
		_tprintf(TEXT("COMANDO:"));
		//_tscanf_s(TEXT("%lu"), space.val);
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



//DWORD WINAPI ThreadGameInfo(LPVOID lpParam)
//{
//	int numRoads = (int)lpParam;
//	_tprintf(TEXT("Thread info %d\n"),numRoads);
//	while (1)
//	{
//		for (int i = 0; i < numRoads; i++)
//		{
//			for (int j = 0; j < MAX_COLS; j++)
//			{
//				if (i == 0 || i==1|| i == numRoads+3 || i == numRoads+4)
//				{
//					_tprintf(TEXT("Thread info\n"));
//					HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//					COORD cursorPos;
//					DWORD numWritten; // Number of characters actually written
//					cursorPos.X = j;
//					cursorPos.Y = i;
//					TCHAR buffer = 'X';
//					SetConsoleCursorPosition(hConsole, cursorPos);
//					WriteConsole(hConsole, &buffer, 1, &numWritten, NULL);
//				}
//			}
//		}
//		
//	}
//	return 0;
//}

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

	WaitForSingleObject(data.Serv_HEvent, INFINITE);

	WaitForSingleObject(data.Serv_HMutex, INFINITE);

	//system("cls");//nao sei ate q ponto é bom usar isto

	//libertat o mutex
	ReleaseMutex(data.Serv_HMutex);

	////Thread para inicio e fim do Mapa + pontuacao/ restante info necessaria 
	//HANDLE hThreadsINFO = CreateThread(
	//	NULL,    // Thread attributes
	//	0,       // Stack size (0 = use default)
	//	ThreadGameInfo, // Thread start address
	//	&pBuf->numRoads,    // Parameter to pass to the thread
	//	0,       // Creation flags
	//	NULL);   // Thread id   // returns the thread identifier 

	//Threads de geração do inicio e fim
	HANDLE StartEndThreads[1];

	TStartEnd StartEndData[1];

	HANDLE HMapFileBeginEnd = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS, TEXT("SO2_MAP_OLA"));
	//_tprintf(TEXT("SO2_MAP_OLA") + (i + 2));
	if (HMapFileBeginEnd == NULL)
	{
		_tprintf(TEXT("ERRO CreateFileMapping\n"));
		return 0;
	}

	StartEndData[0].sharedMap = (TCHAR*)MapViewOfFile(HMapFileBeginEnd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (StartEndData[0].sharedMap == NULL)
	{
		_tprintf(TEXT("ERRO MapViewOfFile\n"));
		return 0;
	}
	StartEndData[0].hMutex = CreateMutex(NULL, FALSE, TEXT("MUTEX_ROADS"));
	StartEndData[0].numRoads = pBuf->numRoads;
	StartEndThreads[0] = CreateThread(
		NULL,
		0,
		ThreadBeginEnd,
		&StartEndData[0],
		0,
		NULL);

	//Gerar Threads Roads
	HANDLE mutex_ROADS;

	// matriz de handles das threads
	HANDLE RoadThreads[MAX_ROADS_THREADS];

	TRoads RoadsData[MAX_ROADS_THREADS];

	_tprintf(TEXT("[DEBUG] NUM ROADS %d criada\n"), pBuf->numRoads);
	//criar Threads para lidar com os carros por estrada
	for (int i = 0; i < pBuf->numRoads; i++)
	{
		HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS, TEXT("SO2_MAP_OLA"));
		//_tprintf(TEXT("SO2_MAP_OLA") + (i + 2));
		if (HMapFile == NULL)
		{
			_tprintf(TEXT("ERRO CreateFileMapping\n"));
			return 0;
		}

		RoadsData[i].sharedMap = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (RoadsData[i].sharedMap == NULL)
		{
			_tprintf(TEXT("ERRO MapViewOfFile\n"));
			return 0;
		}

		RoadsData[i].numCars = pBuf->numCars;
		RoadsData[i].hMutex = CreateMutex(NULL, FALSE, TEXT("MUTEX_ROADS"));
		RoadsData[i].hEventRoads = CreateEvent(NULL, TRUE, FALSE, TEXT("EVENT_ROADS") + i);
		RoadsData[i].id = i + SKIP_BEGINING; //o numero do id é a estrada q elas estao encarregues
		RoadsData[i].speed = 0;
		//RoadsData[i].direction = 1;
		RoadThreads[i] = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			ThreadRoads, // Thread start address
			&RoadsData[i],    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id   // returns the thread identifier 
		//_tprintf(TEXT("[DEBUG] Thread estrada %d criada\n"), i);
	}

	HANDLE InitialEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("INITIAL EVENT"));

	SetEvent(InitialEvent);
	Sleep(1000);
	ResetEvent(InitialEvent);

	while (1)
	{

	}
	CloseHandle(hThreads);
	return 0;
}
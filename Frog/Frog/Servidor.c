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
	_tprintf(TEXT("[INFO] INICIO DA THREAD %d\n"), data->id);
	while (*data->terminar == 0)
	{
		WaitForSingleObject(data->hMutex, INFINITE);
		temp = data->car_pos;
		//movimento carros
		for (int i = 0; i < data->numCars; i++)
		{
			int x = temp[i].row;
			if (x == data->id)
			{
				int y = temp[i].col;

				if (data->direction == ROAD_RIGHT) {
					//carros em fila
					if (data->Map[x * MAX_COLS + y + 1] == BLOCK_ELEMENT && data->Map[x * MAX_COLS + 1] == CAR_ELEMENT)
						y = 1;

					if (data->Map[x * MAX_COLS + y + 1] == CAR_ELEMENT) {
						int testing = y;
						int increment = 1;
						while (data->Map[x * MAX_COLS + testing + increment] == CAR_ELEMENT || data->Map[x * MAX_COLS + testing + increment] == BLOCK_ELEMENT) {
							if (testing + increment == 19) {
								testing = 1;
								increment = -1;
							}
							increment += 1;
						}
						if (data->Map[x * MAX_COLS + testing + increment] != OBSTACLE_ELEMENT) {
							if (temp[i].col + 1 == 19) {
								temp[i].col = 1;
							}
							else
								temp[i].col++;
						}
						else
							continue;
					}
					else if (data->Map[x * MAX_COLS + y + 1] != OBSTACLE_ELEMENT) {
						if (temp[i].col + 1 == 19) {
							temp[i].col = 1;
						}
						else
							temp[i].col++;
					}
				}
				//logica que acho q é para a esquerda
				else if (data->direction == ROAD_LEFT) {

					// carros em fila
					if (data->Map[x * MAX_COLS + y - 1] == BLOCK_ELEMENT)
						y = MAX_COLS - 1;

					if (data->Map[x * MAX_COLS + y - 1] == CAR_ELEMENT) {
						int testing = y;
						int increment = -1;
						while (data->Map[x * MAX_COLS + testing + increment] == CAR_ELEMENT || data->Map[x * MAX_COLS + testing + increment] == BLOCK_ELEMENT) {
							if (testing + increment == 0) {
								testing = MAX_COLS - 2;
								increment = 1;
							}
							increment -= 1;
						}
						if (data->Map[x * MAX_COLS + testing + increment] != OBSTACLE_ELEMENT) {
							if (temp[i].col - 1 == 0) {
								temp[i].col = MAX_COLS - 2;
							}
							else
								temp[i].col--;
						}
						else
							continue;
					}
					else if (data->Map[x * MAX_COLS + y - 1] != OBSTACLE_ELEMENT) {
						if (temp[i].col - 1 == 0) {
							temp[i].col = MAX_COLS - 2;
						}
						else
							temp[i].col--;
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

		// refazer linha
		for (int i = 0; i < data->numCars; i++)
		{
			int x = temp[i].row;
			if (x == data->id)
			{
				data->Map[temp[i].row * MAX_COLS + temp[i].col] = CAR_ELEMENT;
			}
		}
		CopyMemory(data->sharedMap, data->Map, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
		ReleaseMutex(data->hMutex);

		//Criamos evento para que as threads ja consiga ler
		SetEvent(data->hEventRoads);
		Sleep(500);
		ResetEvent(data->hEventRoads);
		Sleep(data->speed);
	}
	return 0;
}

DWORD WINAPI CheckOperators(LPVOID lpParam)
{
	HANDLE InitialEvent = CreateEvent(NULL, TRUE, FALSE, INITIAL_EVENT);
	while (1) {
		//Criamos evento para que as threads ja consiga ler
		_tprintf(TEXT("[INFO] Espera do evento check\n"));
		WaitForSingleObject(InitialEvent, INFINITE);

		HANDLE threadPontuacaoEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("PONTUACAO"));

		HANDLE x = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORY_EVENT);

		if (x == NULL)
		{
			_tprintf(TEXT("[ERRO] CreateEvent na Thread CheckOperators\n"));
			return 0;
		}
		SetEvent(x);
		Sleep(500);
		ResetEvent(x);
		SetEvent(threadPontuacaoEvent);
		Sleep(500);
		ResetEvent(threadPontuacaoEvent);
	}
}

void ToggleRoadDirection(pTRoads roads, int numRoads, int roadId) {
	for (int i = 0; i < numRoads; i++) {
		if (roads[i].id == roadId) {
			roads[i].direction = (roads[i].direction == ROAD_RIGHT) ? ROAD_LEFT : ROAD_RIGHT;
			break;
		}
	}
}

void HandleStopCommand(HANDLE *dados, int numRoads) {
	for (int i = 0; i < numRoads; i++) {
		SuspendThread(dados[i]);
	}
}

void HandleStartCommand(HANDLE* dados, int numRoads) {
	for (int i = 0; i < numRoads; i++) {
		ResumeThread(dados[i]);
	}
}

DWORD WINAPI threadStopGame(LPVOID lpParam)
{
	pTStopGame dados = (pTStopGame)lpParam;
	int time = 0;
	time = _wtoi(dados->time);
	time = time * 1000;
	HandleStopCommand(dados->roadThreadsHandles, dados->numRoads);
	Sleep(time); // for the specified time in seconds
	HandleStartCommand(dados->roadThreadsHandles, dados->numRoads);
}

void HandleChangeCommand(pTDados dados, const TCHAR* firstNumber) {
	int roadId = _wtoi(firstNumber);
	ToggleRoadDirection(dados->RoadsDirection, dados->numRoads, roadId);
}

void HandleInsertCommand(pTDados dados, const TCHAR* firstNumber, const TCHAR* secondNumber) {
	int roadId = _wtoi(firstNumber);
	int coluna = _wtoi(secondNumber);

	_tprintf(TEXT("Testing[%s]!"), firstNumber);
	_tprintf(TEXT("Testing[%s]!"), secondNumber);

	if (roadId >= 2 && roadId <= dados->numRoads + 2)
		_tprintf(TEXT("Am digit[%d]!"), roadId);
	else
		_tprintf(TEXT("Am not digit!"));

	if (coluna >= 1 && coluna <= 18)
		_tprintf(TEXT("Am digit[%d]!"), coluna);
	else
		_tprintf(TEXT("Am not digit!"));

	WaitForSingleObject(dados->hMutexInsertRoad, INFINITE);
	if (dados->Map[roadId * MAX_COLS + coluna] != CAR_ELEMENT) {
		dados->Map[roadId * MAX_COLS + coluna] = OBSTACLE_ELEMENT;
	}
	ReleaseMutex(dados->hMutexInsertRoad);
}

void HandleDeleteCommand(pTDados dados, const TCHAR* firstNumber, const TCHAR* secondNumber) {
	int roadId = _wtoi(firstNumber);
	int coluna = _wtoi(secondNumber);

	if (roadId >= 2 && roadId <= dados->numRoads + 2)
		_tprintf(TEXT("Numero [%d]!"), roadId);
	else
		_tprintf(TEXT("Nao numero!"));

	if (coluna >= 1 && coluna <= 18)
		_tprintf(TEXT("Numero [%d]!"), coluna);
	else
		_tprintf(TEXT("Nao numero!"));

	WaitForSingleObject(dados->hMutexInsertRoad, INFINITE);
	if (dados->Map[roadId * MAX_COLS + coluna] != CAR_ELEMENT && dados->Map[roadId * MAX_COLS + coluna] == OBSTACLE_ELEMENT) {
		dados->Map[roadId * MAX_COLS + coluna] = ROAD_ELEMENT;
	}
	ReleaseMutex(dados->hMutexInsertRoad);
}

DWORD WINAPI ThreadBufferCircular(LPVOID lpParam)
{
	pTDados dados = (pTDados)lpParam;
	EspacoBuffer space;
	space.id = 0;
	HANDLE hThreadStopGame = NULL;
	while (*dados->terminar == 0)
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

		TCHAR* token;
		TCHAR* context;
		TCHAR command[20];
		TCHAR firstNumber[20];
		TCHAR secondNumber[20];

		token = _tcstok_s(space.val, _T(" "), &context);
		if (token != NULL)
			_tcscpy_s(command, 20, token);

		if (lstrcmp(command, TEXT("Stop")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			tStopGame stopGame;
			stopGame.roadThreadsHandles = dados->threadsHandles;
			stopGame.time = firstNumber;//para ser em segundos
			stopGame.numRoads = dados->numRoads;
			hThreadStopGame = CreateThread(
				NULL,    // Thread attributes
				0,       // Stack size (0 = use default)
				threadStopGame, // Thread start address
				&stopGame,    // Parameter to pass to the thread
				0,       // Creation flags
				NULL);   // Thread id   // returns the thread identifier
			if (hThreadStopGame == NULL)
			{
				_tprintf(TEXT("[ERRO] Thread BufferCircular Erro ao criar thread de pausa do jogo\n"));
				return 1;
			}
		}
		else if (lstrcmp(command, TEXT("Terminar")) == 0) {
			//acabar com tudo
			*dados->terminar = 1;
		}
		else if (lstrcmp(command, TEXT("Start")) == 0) {
			//HandleStartCommand(dados);
		}
		else if (lstrcmp(command, TEXT("Change")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			HandleChangeCommand(dados, firstNumber);
		}
		else if (lstrcmp(command, TEXT("Insert")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(secondNumber, 20, token);
			HandleInsertCommand(dados,firstNumber,secondNumber);
		}
		else if (lstrcmp(command, TEXT("Remove")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(secondNumber, 20, token);
			HandleDeleteCommand(dados, firstNumber, secondNumber);
		}
		ReleaseMutex(dados->hMutex);
		ReleaseSemaphore(dados->hSemEscrita, 1, NULL);
	}
	CloseHandle(hThreadStopGame);
	return 0;
}

int parse_args(TCHAR* arg1_str, TCHAR* arg2_str, DWORD* arg1, DWORD* arg2) {
	_tprintf(TEXT("%s %s \n"), arg1_str, arg2_str);

	*arg1 = _wtoi(arg1_str);
	*arg2 = _wtoi(arg2_str);

	if (!arg1 || !arg2) {
		_tprintf(TEXT("[ERRO] Formato invalido para argumentos\n"));
		return 1;
	}

	_tprintf(TEXT("[INFO] arg1 = %lu\n"), *arg1);
	_tprintf(TEXT("[INFO] arg2 = %lu\n"), *arg2);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

	int terminar = 0; // para acabar com tudo

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	srand((unsigned)time(NULL));

	DWORD arg1 = 0, arg2 = 0, parse_result = 0;

	GameData data;
	if (argc != 3) {
		_tprintf(TEXT("[ERRO] Parametros errados \n"));
		//função que vai buscar ao registry se existir, senão return 1;
		data = RegistryGetValues();
	}
	else {
		parse_result = parse_args(argv[1], argv[2], &arg1, &arg2);
		if (parse_result != 0) {
			_tprintf(TEXT("[ERRO] Conversao de argumentos falhada\n"));
			return 1;
		}
		_tprintf(TEXT("[INFO] Conversao completa!\n arg1 :%lu \narg2 :%lu \n"), arg1, arg2);
		data = RegistryKeyValue(arg1, arg2);

	}
	//criar Semaf
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, SEMAPHORE_UNIQUE_SERVER);
	if (hSem == NULL)
	{
		_tprintf("[ERRO] CreateSemaphore: %d\n", GetLastError());
		return 1;
	}

	_tprintf(TEXT("[INFO] Waiting for slot...\n"));

	WaitForSingleObject(hSem, INFINITE);
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
			}
			else
				data.map[i][j] = ROAD_ELEMENT;

		}
	}

	//Gerar carros
	for (int i = 0; i < data.numRoads; i++)
	{
		int carsInRoad = (rand() % 8) + 1;
		for (; carsInRoad > 0; carsInRoad--) {
			data.car_pos[data.numCars].row = i + SKIP_BEGINING; //X -> linha
			int posInRoad = 0;
			do {
				posInRoad = (rand() % (MAX_COLS - 2)) + 1;
			} while (data.map[i + SKIP_BEGINING][posInRoad] == 'H' || data.map[i + SKIP_BEGINING][posInRoad] == '#');
			//_tprintf(TEXT("PUS AQUI %d %d\n"), posInRoad, i + SKIP_BEGINING);
			data.car_pos[data.numCars].col = posInRoad; //y -> coluna
			data.map[i + SKIP_BEGINING][posInRoad] = CAR_ELEMENT;
			data.numCars++;
		}
	}
	_tprintf(TEXT("[INFO] Numero total de carros %d\n"), data.numCars);

	//Gerar sapos (Primeira meta....)
	int sapRowRandom = data.numRoads + 2; //+3 para ficar na penultima estrada.
	for (int i = 0; i < 2; i++)
	{
		int sapColRandom = (rand() % (MAX_COLS - 2)) + 1;
		do {
			sapColRandom = (rand() % (MAX_COLS - 2)) + 1;
		} while (data.map[sapRowRandom][sapColRandom] == 'S');
		data.map[sapRowRandom][sapColRandom] = FROGGE_ELEMENT;
	}

	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), FILE_MAPPING_GAME_DATA);
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("[ERRO] CreateFileMapping GameInfo\n"));
		return 0;
	}

	pGameData pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("[ERRO] MapViewOfFile GameInfo\n"));
		return 0;
	}

	//criar mutex
	data.Serv_HMutex = CreateMutex(NULL, FALSE, SHARED_MEMORY_MUTEX);
	if (data.Serv_HMutex == NULL)
	{
		_tprintf(TEXT("[ERRO] CreateMutex GameInfo\n"));
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

	for (int i = 0; i < data.numRoads; i++)
	{
		HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS, FILE_MAPPING_THREAD_ROADS);
		if (HMapFile == NULL)
		{
			_tprintf(TEXT("[ERRO] CreateFileMapping Mapa\n"));
			return 0;
		}
		RoadsData[i].sharedMap = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (RoadsData[i].sharedMap == NULL)
		{
			_tprintf(TEXT("[ERRO] MapViewOfFile Mapa\n"));
			return 0;
		}

		RoadsData[i].Map = &data.map;
		RoadsData[i].car_pos = &data.car_pos;
		RoadsData[i].numCars = data.numCars;
		RoadsData[i].hMutex = CreateMutex(NULL, FALSE, THREAD_ROADS_MUTEX);
		RoadsData[i].hEventRoads = CreateEvent(NULL, TRUE, FALSE, THREAD_ROADS_EVENT + i);
		RoadsData[i].id = i + SKIP_BEGINING; //o numero do id é a estrada q elas estao encarregues
		RoadsData[i].speed = data.carSpeed;//((rand() % 8) + 1) * 1000
		RoadsData[i].terminar = &terminar;
		RoadsData[i].direction = (rand() % 2);
		RoadThreads[i] = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			ThreadRoads, // Thread start address
			&RoadsData[i],    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id   // returns the thread identifier 
		if (RoadThreads[i] == NULL) {
			_tprintf(TEXT("[DEBUG] Thread estrada %d erro\n"), i);
			return 1;
		}
	}

	//BufferCircular
	TDados dataThread;

	dataThread.hMutex = CreateMutex(NULL, FALSE, BUFFER_CIRCULAR_MUTEX_LEITOR);
	dataThread.hSemEscrita = CreateSemaphore(NULL, 10, 10, BUFFER_CIRCULAR_SEMAPHORE_ESCRITOR);
	dataThread.hSemLeitura = CreateSemaphore(NULL, 0, 10, BUFFER_CIRCULAR_SEMAPHORE_LEITORE);

	HANDLE HMapFileBuffer = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FILE_MAPPING_BUFFER_CIRCULAR);
	if (HMapFileBuffer == NULL)
	{
		HMapFileBuffer = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Buffer), FILE_MAPPING_BUFFER_CIRCULAR);
		dataThread.BufferCircular = (pBuffer)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		dataThread.BufferCircular->nConsumidores = 0;
		dataThread.BufferCircular->nProdutores = 0;
		dataThread.BufferCircular->posEscrita = 0;
		dataThread.BufferCircular->posLeitura = 0;

		if (HMapFileBuffer == NULL)
		{
			_tprintf(TEXT("[ERRO] CreateFileMapping BufferCircular\n"));
			return 0;
		}
	}
	else
	{
		dataThread.BufferCircular = (Buffer*)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (HMapFileBuffer == NULL)
		{
			_tprintf(TEXT("[ERRO] CreateFileMapping BufferCircular\n"));
			return 0;
		}
	}
	dataThread.id = dataThread.BufferCircular->nConsumidores++;
	dataThread.RoadsDirection = &RoadsData;
	dataThread.Map = &data.map;
	dataThread.threadsHandles = &RoadThreads;
	dataThread.numRoads = data.numRoads;
	dataThread.hMutexInsertRoad = CreateMutex(NULL, FALSE, THREAD_ROADS_MUTEX);
	dataThread.terminar = &terminar;

	HANDLE hThreads = CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		ThreadBufferCircular, // Thread start address
		&dataThread,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);   // Thread id   // returns the thread identifier 
	if (hThreads == NULL)
	{
		_tprintf(TEXT("[ERRO] Thread BufferCircular\n"));
		return 1;
	}

	HANDLE InitialEvent = CreateEvent(NULL, TRUE, FALSE, INITIAL_EVENT);

	_tprintf(TEXT("\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("[INFO] SERVIDOR A ESPERA DE PELO MENOS UM OPERADOR\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("[INFO] OPERADOR INICIADO\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("\n"));

	HANDLE tHCheckOpearators = CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		CheckOperators, // Thread start address
		NULL,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);
	if (tHCheckOpearators == NULL)
	{
		_tprintf(TEXT("[DEBUG] Thread CheckOperadores\n"));
		return 1;
	}

	while (terminar == 0)
	{

	}
		
	_tprintf(TEXT("[INFO] SERVIDOR VAI TERMINAR\n"));
	
	UnmapViewOfFile(HMapFile);
	UnmapViewOfFile(HMapFileBuffer);
	// Close thread handles
	for (int i = 0; i < data.numRoads; i++) {
		CloseHandle(RoadThreads[i]);
	}
	CloseHandle(hThreads);
	CloseHandle(tHCheckOpearators);
	//ReleaseSemaphore(hSem, 1, NULL);
	//UnmapViewOfFile(pBuf);
	return 0;
}
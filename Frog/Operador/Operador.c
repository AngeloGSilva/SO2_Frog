#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"
#include "../SharedMemory/SharedMemory.h"
#include <time.h>
#include <stdlib.h>

HHOOK g_keyboardHook = NULL;

//Estrutura para KeyHook Thread
typedef struct {
	HANDLE* threadsHandlesOperator;
	HANDLE Hhook;
	int numRoads;
} TKeyBoardHook, * pTKeyBoardHook;

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LSHIFT)
		{
			MessageBox(NULL, TEXT("Escondes te o jogo, podes digitar um comando"), TEXT("Modo Comando"), MB_OK);
		}
	}
	return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

DWORD WINAPI ThreadKeyHook(LPVOID lpParam)
{

	//getAsyncState(VK_DOWN) & 0x8000
		//setEvent();
	pTKeyBoardHook data = (pTKeyBoardHook)lpParam;
	HANDLE eventKeyBoard = CreateEvent(NULL, TRUE, FALSE, KEYBOARD_EVENT);
	g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		SetEvent(eventKeyBoard);
		//Sleep(500);
		ResetEvent(eventKeyBoard);
		UnhookWindowsHookEx(g_keyboardHook);
	}// Enter the message loop to keep the program running
	return 0;
}

DWORD WINAPI CheckEnding(LPVOID lpParam)
{
	int* terminate = (int*)lpParam;
	HANDLE ending_event = CreateEvent(NULL, TRUE, FALSE, ENDING_EVENT);
	WaitForSingleObject(ending_event, INFINITE);

	*terminate = 1;
}

DWORD WINAPI ThreadRoads(LPVOID lpParam)
{
	pTRoads data = (pTRoads)lpParam;
	TCHAR* temp;
	COORD cursorPos;
	DWORD numWritten; // Number of characters actually written
	while (*data->terminar == 0)
	{
		WaitForSingleObject(data->evtHandles.hEventRoads[data->id - SKIP_BEGINING], INFINITE);
		WaitForSingleObject(data->mtxHandles.mutexMapaChange, INFINITE);
		//copyMemoryOperation(&temp, &data->sharedMap, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
		
		SharedMemoryMapThreadRoadsOperador(data, &temp);
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		for (int i = 0; i < MAX_COLS; i++)
		{
			cursorPos.X = i;
			cursorPos.Y = data->id;
			//_tprintf(TEXT("coluna do carro %d :%d\n"), i,temp[i].col);
			SetConsoleCursorPosition(hConsole, cursorPos);
			WriteConsole(hConsole, &temp[data->id * MAX_COLS + i], 1, &numWritten, NULL);
		}
		ReleaseMutex(data->mtxHandles.mutexMapaChange);
	}
	return 0;
}

//Margem superior e inferior
DWORD WINAPI ThreadBeginEnd(LPVOID lpParam)
{
	pTStartEnd data = (pTStartEnd)lpParam;
	TCHAR* temp;
	WaitForSingleObject(data->hMutex, INFINITE);
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
			WriteConsole(hConsole, &data->Map[aux * MAX_COLS + i], 1, &numWritten, NULL);
		}
	}
	ReleaseMutex(data->hMutex);
	ExitThread(7);
}

DWORD WINAPI ThreadBufferCircular(LPVOID lpParam)
{
	HANDLE eventKeyBoard = CreateEvent(NULL, TRUE, FALSE, KEYBOARD_EVENT);
	pTDados dados = (pTDados)lpParam;
	EspacoBuffer space;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursorPos;
	TKeyBoardHook TDataKeyHook;
	TDataKeyHook.threadsHandlesOperator = dados->threadsHandles;
	TDataKeyHook.numRoads = dados->numRoads;
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	// Get the current console information
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);

	while (*dados->terminar == 0)
	{
		HANDLE HTKeyHook = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			ThreadKeyHook, // Thread start address
			&TDataKeyHook,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id   // returns the thread identifier 
		if (HTKeyHook == NULL)
		{
			_tprintf(TEXT("[ERRO] Thread KeyHook\n"));
			return 0;
		}
		//space.id = dados->id;

		WaitForSingleObject(eventKeyBoard, INFINITE);

		for (int i = 0; i < dados->numRoads; i++) {
			SuspendThread(TDataKeyHook.threadsHandlesOperator[i]);
		}
		DWORD numWritten; // Number of characters actually written
		cursorPos.X = 0;
		cursorPos.Y = 20;
		SetConsoleCursorPosition(hConsole, cursorPos);
		FillConsoleOutputCharacter(hConsole, ' ', consoleInfo.dwSize.X, cursorPos, &numWritten);
		_tprintf(TEXT("COMANDO:"));
		_getts_s(space.val,20);

		if (lstrcmp(space.val, TEXT("terminar")) == 0)
		{
			*dados->terminar = 1;
		}
		//WaitForSingleObject(dados->hSemEscrita, INFINITE);
		//WaitForSingleObject(dados->hMutex, INFINITE);

		////copiar o conteudo para a memoria partilhada
		//copyMemoryOperation(&dados->BufferCircular->espacosDeBuffer[dados->BufferCircular->posEscrita], &space, sizeof(EspacoBuffer));
		//dados->BufferCircular->posEscrita++;
		//if (dados->BufferCircular->posEscrita == 10)
		//{
		//	dados->BufferCircular->posEscrita = 0;
		//}

		//ReleaseMutex(dados->hMutex);
		//ReleaseSemaphore(dados->hSemLeitura, 1, NULL);

		ReadSharedMemoryOperador(dados->BufferCircular, space);


		Sleep(((rand() % 4) + 1) * 1000);

		for (int i = 0; i < dados->numRoads; i++) {
			ResumeThread(TDataKeyHook.threadsHandlesOperator[i]);
		}
	}
	return 0;
}

DWORD WINAPI ThreadGameInfo(LPVOID lpParam)
{
	int *terminar = (int)lpParam;
	HANDLE mutex = CreateMutex(NULL, FALSE, THREAD_ROADS_MUTEX);
	HANDLE threadPontuacaoEvent = CreateEvent(NULL, TRUE, FALSE, GAMEDATA_EVENT);
	while (1)
	{
		WaitForSingleObject(threadPontuacaoEvent, INFINITE);
		WaitForSingleObject(mutex,INFINITE);
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD cursorPos;
		DWORD numWritten; // Number of characters actually written
		cursorPos.X = 50;
		cursorPos.Y = 7;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("JOGO FROGGER"), 15, &numWritten, NULL);
		cursorPos.Y = 8;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("Pontuacao : 0 Tempo : 0"), 23, &numWritten, NULL);
		cursorPos.Y = 9;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("Comandos:"), 10, &numWritten, NULL);
		cursorPos.Y = 10;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("stop [Tempo em Segundo]"), 24, &numWritten, NULL);
		/*cursorPos.Y = 11;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("start"), 6, &numWritten, NULL);*/
		cursorPos.Y = 12;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("change [Estrada]"), 17, &numWritten, NULL);
		cursorPos.Y = 13;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("insert [Estrada] [Coluna]"), 26, &numWritten, NULL);
		cursorPos.Y = 14;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("remove [Estrada] [Coluna]"), 26, &numWritten, NULL);
		cursorPos.Y = 15;
		SetConsoleCursorPosition(hConsole, cursorPos);
		WriteConsole(hConsole, TEXT("Para Utilizar os Comandos � necessario clicar na tecla SHIFT"), 61, &numWritten, NULL);
		ReleaseMutex(mutex);
	}
	
	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	int terminar = 0;

	//rand
	srand((unsigned)time(NULL));

	GameData data;

	EventHandles evtHandles;
	SemaphoreHandles smphHandles;
	MutexHandles mtxHandles;

	pGameData pBuf = InitSharedMemoryMap();

	evtHandles.SharedMemoryEvent = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORY_EVENT);
	mtxHandles.mutexMapaChange = CreateMutex(NULL, FALSE, THREAD_ROADS_MUTEX);

	//data.event = OpenEvent(READ_CONTROL,TRUE, TEXT("TP_Evento"));
	//data.Serv_HEvent = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORY_EVENT);
	//data.mutex = OpenMutex(READ_CONTROL, TRUE, TEXT("TP_Mutex"));

	evtHandles.InitialEvent = CreateEvent(NULL, TRUE, FALSE, INITIAL_EVENT);

	SetEvent(evtHandles.InitialEvent);
	ResetEvent(evtHandles.InitialEvent);


	DWORD abandon = WaitForSingleObject(evtHandles.SharedMemoryEvent, 5000);

	if (abandon == WAIT_TIMEOUT) {
		_tprintf(TEXT("[INFO] Nao ha servidor disponivel\n"));
		return 1;
	}

	//Thread para inicio e fim do Mapa + pontuacao/ restante info necessaria 
	HANDLE hThreadsINFO = CreateThread(
		NULL,
		0,
		ThreadGameInfo,
		&terminar,
		0,
		NULL);

	//Threads de gera��o do inicio e fim
	HANDLE StartEndThreads[1];

	TStartEnd StartEndData[1];

	StartEndData[0].sharedMap = InitSharedMemoryMapThreadRoads();
	StartEndData[0].hMutex = CreateMutex(NULL, FALSE, THREAD_ROADS_MUTEX);
	StartEndData[0].numRoads = pBuf->numRoads;
	StartEndData[0].Map = pBuf->map;
	StartEndData[0].terminar = &terminar;
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

	//criar Threads para lidar com os carros por estrada
	for (int i = 0; i < pBuf->numRoads; i++)
	{
		////TODO Nao me lembro o porque de isto ser preciso
		//HANDLE HMapFile = createMemoryMapping(sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS, FILE_MAPPING_THREAD_ROADS);
		////_tprintf(TEXT("SO2_MAP_OLA") + (i + 2));
		//if (HMapFile == NULL)
		//{
		//	_tprintf(TEXT("[ERRO] CreateFileMapping Thread da estrada %d\n"),i);
		//	return 0;
		//}

		//RoadsData[i].sharedMap = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		//if (RoadsData[i].sharedMap == NULL)
		//{
		//	_tprintf(TEXT("[ERRO] CreateFileMapping Thread da estrada %d\n"), i);
		//	return 0;
		//}
		RoadsData[i].sharedMap = InitSharedMemoryMapThreadRoads();
		RoadsData[i].numCars = pBuf->numCars;
		evtHandles.hEventRoads[i] = CreateEvent(NULL, TRUE, FALSE, THREAD_ROADS_EVENT + i);
		RoadsData[i].id = i + SKIP_BEGINING; //o numero do id � a estrada q elas estao encarregues
		RoadsData[i].speed = 0;
		RoadsData[i].terminar = &terminar;
		RoadsData[i].mtxHandles = mtxHandles;
		RoadsData[i].evtHandles = evtHandles;
		//RoadsData[i].direction = 1;

		RoadThreads[i] = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			ThreadRoads, // Thread start address
			&RoadsData[i],    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id   // returns the thread identifier 
		if (RoadThreads[i] == NULL)
		{
			_tprintf(TEXT("[ERRO] Thread da estrada %d\n"),i);
			return 0;
		}
	}

	TDados dataThread;

	/*dataThread.hMutex = CreateMutex(NULL, FALSE, BUFFER_CIRCULAR_MUTEX_ESCRITOR);
	dataThread.hSemEscrita = CreateSemaphore(NULL, 10, 10, BUFFER_CIRCULAR_SEMAPHORE_ESCRITOR);
	dataThread.hSemLeitura = CreateSemaphore(NULL, 0, 10, BUFFER_CIRCULAR_SEMAPHORE_LEITORE);*/

	dataThread.threadsHandles = &RoadThreads;
	dataThread.numRoads = pBuf->numRoads;
	dataThread.BufferCircular = InitSharedMemoryBufferCircular();
	dataThread.terminar = &terminar;


	HANDLE hThreads = CreateThread(
		NULL,
		0,
		ThreadBufferCircular,
		&dataThread,
		0,
		NULL);
	if (hThreads == NULL)
	{
		_tprintf(TEXT("[ERRO] Thread BufferCircular\n"));
		return 0;
	}

	HANDLE fCheckEnding = CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		CheckEnding, // Thread start address
		&terminar,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);
	if (fCheckEnding == NULL)
	{
		_tprintf(TEXT("[ERRO] Thread CheckEnding\n"));
		return 1;
	}

	while (terminar == 0)
	{

	}
	_tprintf(TEXT("[INFO] OPERADOR VAI TERMINAR\n"));

	//UnmapViewOfFile(HMapFile);
	//UnmapViewOfFile(HMapFileBuffer);
	for (int i = 0; i < pBuf->numRoads; i++) {
		CloseHandle(RoadThreads[i]);
	}
	CloseHandle(hThreadsINFO);
	CloseHandle(StartEndThreads);
	CloseHandle(hThreads);
	return 0;
}
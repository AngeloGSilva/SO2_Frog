#include <windows.h>
#include "SharedMemory.h"
//#include "../Frog/Utils.h"
//#include "../Frog/Struct.h"

HANDLE createMemoryMapping(DWORD size,LPCSTR name){
	return CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
}

HANDLE openMemoryMapping(DWORD dwDesiredAccess, LPCSTR lpName) {
	return OpenFileMapping(dwDesiredAccess, FALSE ,lpName);
}

void copyMemoryOperation(PVOID destiny, VOID* source, SIZE_T Length) {
	CopyMemory(destiny,source,Length);
}

void clearMemoryOperation(PVOID destination, SIZE_T Length) {
	ZeroMemory(destination, Length);
}

TCHAR* InitSharedMemoryMapThreadRoads() {
	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS, FILE_MAPPING_THREAD_ROADS);
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("[ERRO] CreateFileMapping Mapa\n"));
		return NULL;
	}

	TCHAR* sharedMap = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (sharedMap == NULL)
	{
		_tprintf(TEXT("[ERRO] MapViewOfFile Mapa\n"));
		return NULL;
	}

	return sharedMap;
}

void SharedMemoryMapThreadRoads(pTRoads data) {
	CopyMemory(data->sharedMap, data->Map, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
}

void SharedMemoryMapThreadRoadsOperador(pTRoads data, TCHAR* temp) {
	CopyMemory(temp, &data->sharedMap, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
}


//primeiro mapa se nao me engano
void SharedMemoryMap(pGameData pBuf, pGameData data) {
	HANDLE hMutex = CreateMutex(NULL, FALSE, SHARED_MEMORY_MUTEX);
	if (hMutex == NULL)
	{
		_tprintf(TEXT("[ERRO] CreateMutex GameInfo\n"));
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);

	ZeroMemory(pBuf, sizeof(GameData));
	CopyMemory(pBuf, data, sizeof(GameData));

	//libertat o mutex
	ReleaseMutex(hMutex);
}

//primeiro mapa se nao me engano
pGameData InitSharedMemoryMap() {
	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), FILE_MAPPING_GAME_DATA);
	if (HMapFile == NULL)
	{
		_tprintf(TEXT("[ERRO] CreateFileMapping GameInfo\n"));
		return NULL;
	}

	//criar mesmo a memoria
	pGameData pBuf = (pGameData)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("[ERRO] MapViewOfFile GameInfo\n"));
		return NULL;
	}

	return pBuf;
}


//bufferCircular
pBuffer InitSharedMemoryBufferCircular() {
	pBuffer BufferCircular = NULL;
	HANDLE HMapFileBuffer = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FILE_MAPPING_BUFFER_CIRCULAR);
	if (HMapFileBuffer == NULL)
	{
		HMapFileBuffer = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Buffer), FILE_MAPPING_BUFFER_CIRCULAR);
		BufferCircular = (pBuffer)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		BufferCircular->nConsumidores = 0;
		BufferCircular->nProdutores = 0;
		BufferCircular->posEscrita = 0;
		BufferCircular->posLeitura = 0;

		if (HMapFileBuffer == NULL)
		{
			_tprintf(TEXT("[ERRO] CreateFileMapping BufferCircular\n"));
			return NULL;
		}
		else
		{
			BufferCircular = (pBuffer)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (HMapFileBuffer == NULL)
			{
				_tprintf(TEXT("[ERRO] CreateFileMapping BufferCircular\n"));
				return NULL;
			}
		}
	}
	else
	{
		BufferCircular = (Buffer*)MapViewOfFile(HMapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (HMapFileBuffer == NULL)
		{
			_tprintf(TEXT("[ERRO] CreateFileMapping BufferCircular\n"));
			return 0;
		}
	}
	return BufferCircular;
}

//bufferCircular
EspacoBuffer ReadSharedMemoryServer(pBuffer BufferCircular) {
	EspacoBuffer space;
	HANDLE hMutex = CreateMutex(NULL, FALSE, BUFFER_CIRCULAR_MUTEX_LEITOR);
	HANDLE hSemEscrita = CreateSemaphore(NULL, 10, 10, BUFFER_CIRCULAR_SEMAPHORE_ESCRITOR);
	HANDLE hSemLeitura = CreateSemaphore(NULL, 0, 10, BUFFER_CIRCULAR_SEMAPHORE_LEITORE);

	WaitForSingleObject(hSemLeitura, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);
	CopyMemory(&space, &BufferCircular->espacosDeBuffer[BufferCircular->posLeitura], sizeof(EspacoBuffer));
	BufferCircular->posLeitura++;
	if (BufferCircular->posLeitura == 10)
	{
		BufferCircular->posLeitura = 0;
	}

	//e o codigo???

	ReleaseMutex(hMutex);
	ReleaseSemaphore(hSemEscrita, 1, NULL);
	return space;
}

//bufferCircular
BOOL ReadSharedMemoryOperador(pBuffer BufferCircular, EspacoBuffer space) {
	//EspacoBuffer space;
	HANDLE hMutex = CreateMutex(NULL, FALSE, BUFFER_CIRCULAR_MUTEX_ESCRITOR);
	HANDLE hSemEscrita = CreateSemaphore(NULL, 10, 10, BUFFER_CIRCULAR_SEMAPHORE_ESCRITOR);
	HANDLE hSemLeitura = CreateSemaphore(NULL, 0, 10, BUFFER_CIRCULAR_SEMAPHORE_LEITORE);

	WaitForSingleObject(hSemEscrita, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

	CopyMemory(&BufferCircular->espacosDeBuffer[BufferCircular->posEscrita], &space, sizeof(EspacoBuffer));
	BufferCircular->posEscrita++;
	if (BufferCircular->posEscrita == 10)
	{
		BufferCircular->posEscrita = 0;
	}

	ReleaseMutex(hMutex);
	ReleaseSemaphore(hSemLeitura, 1, NULL);
	return TRUE;
}
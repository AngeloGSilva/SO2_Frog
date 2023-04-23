#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "Utils.h"
#include "Struct.h"
#include "SharedMemory.h"


//ideias para tratar do mapa.... 
//fazer uma thread separada do main que trate so do desenho do mapa
//movimentos do sapo sera algo semelhante



int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	GameData data;

	//data tem de sair e so ficar pbuf penso eu

	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), FILE_MAPPING_GAME_DATA);
	//TCHAR* pbuf = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("TP_GameData"));
	pGameData pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//data.event = OpenEvent(READ_CONTROL,TRUE, TEXT("TP_Evento"));
	data.Serv_HEvent = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORIE_EVENT);
	//data.mutex = OpenMutex(READ_CONTROL, TRUE, TEXT("TP_Mutex"));
	data.Serv_HMutex = CreateMutex(NULL, FALSE, SHARED_MEMORIE_MUTEX);

	while (1)
	{
		WaitForSingleObject(data.Serv_HEvent, INFINITE);

		WaitForSingleObject(data.Serv_HMutex, INFINITE);

		system("cls");//nao sei ate q ponto é bom usar isto

		//_tprintf(TEXT("Mensagem Recebida: %d %d\n"), pBuf->num_cars,pBuf->num_frogs);
		for (int i = 0; i < MAX_ROWS + 4; i++)
		{
			for (int j = 0; j < MAX_COLS; j++)
			{
				_tprintf(TEXT("%c"), pBuf->map[i][j]);
			}
			_tprintf("\n");
		}

		//libertat o mutex
		ReleaseMutex(data.Serv_HMutex);

		Sleep(pBuf->carSpeed);
	}


	return 0;
}
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX_CARS 10
#define MAX_FROGS 10
#define MAX_ROWS 10
#define MAX_COLS 20


typedef struct {
	HANDLE Serv_HSem, Serv_HMutex, Serv_HEvent;
	int num_cars;
	int num_frogs;
	int car_pos[MAX_CARS][2]; //2 seria para representar o x e o y
	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
	TCHAR map[MAX_ROWS][MAX_COLS];
}GameData;


int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	GameData data;

	HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), TEXT("TP_GameData"));
	//TCHAR* pbuf = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("TP_GameData"));
	GameData* pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//data.event = OpenEvent(READ_CONTROL,TRUE, TEXT("TP_Evento"));
	data.Serv_HEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("TP_Evento"));
	//data.mutex = OpenMutex(READ_CONTROL, TRUE, TEXT("TP_Mutex"));
	data.Serv_HMutex = CreateMutex(NULL, FALSE, TEXT("TP_Mutex"));

	while (1)
	{
		WaitForSingleObject(data.Serv_HEvent, INFINITE);

		WaitForSingleObject(data.Serv_HMutex, INFINITE);
		_tprintf("\n");
		_tprintf("\n");
		_tprintf("\n");

		//_tprintf(TEXT("Mensagem Recebida: %d %d\n"), pBuf->num_cars,pBuf->num_frogs);
		for (int i = 0; i < MAX_ROWS; i++)
		{
			for (int j = 0; j < MAX_COLS; j++)
			{
				_tprintf(TEXT("%c"), pBuf->map[i][j]);
			}
			_tprintf("\n");
		}

		//libertat o mutex
		ReleaseMutex(data.Serv_HMutex);

		Sleep(1000);
	}


	return 0;
}
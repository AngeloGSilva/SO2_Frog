//#define MAX_CARS 10
//#define MAX_FROGS 10
//#define MAX_ROWS 20
//#define MAX_COLS 40
//
//
//#define GAME_NAME "frogger_shared_memory"
//#define S_SIZE sizeof(GameData)
//
//struct GameData {
//	int num_cars;
//	int num_frogs;
//	int car_pos[MAX_CARS][2]; //2 seria para representar o x e o y
//	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
//	TCHAR map[MAX_ROWS][MAX_CLOS]
//}
//
//int main() {
//	HANDLE handle;
//	GameData* game_data;
//
//	handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, S_SIZE, GAME_NAME);
//
//	return 0;
//
//}


#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX_CARS 10
#define MAX_FROGS 10
#define MAX_ROWS 20
#define MAX_COLS 40


typedef struct {
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
		data.num_cars = 0;
		data.num_frogs = 0;
		HANDLE HMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), TEXT("TP_GameData"));
		//TCHAR* pbuf = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("TP_GameData"));
		GameData* pBuf = (TCHAR*)MapViewOfFile(HMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		//data.event = OpenEvent(READ_CONTROL,TRUE, TEXT("TP_Evento"));
		HANDLE event = CreateEvent(NULL, TRUE, FALSE, TEXT("TP_Evento"));
		//data.mutex = OpenMutex(READ_CONTROL, TRUE, TEXT("TP_Mutex"));
		HANDLE mutex = CreateMutex(NULL, FALSE, TEXT("TP_Mutex"));

		while (1)
		{
			WaitForSingleObject(event, INFINITE);

			WaitForSingleObject(mutex, INFINITE);
			_tprintf(TEXT("Mensagem Recebida: %d %d\n"), pBuf->num_cars,pBuf->num_frogs);

			//libertat o mutex
			ReleaseMutex(mutex);

			Sleep(1000);
		}


	return 0;
}
#include <windows.h>
#include "SharedMemory.h"

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

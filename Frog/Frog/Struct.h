#pragma once
#include <windows.h>
#include <tchar.h>
#include "Utils.h"

typedef struct {
	HANDLE Serv_HMutex, Serv_HEvent;
	DWORD carSpeed;
	DWORD num_cars;
	int num_frogs;
	int car_pos[MAX_CARS][2]; //2 seria para representar o x e o y
	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
}GameData, *pGameData;
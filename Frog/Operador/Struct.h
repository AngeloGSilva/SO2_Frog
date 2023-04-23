#pragma once
#include <windows.h>
#include <tchar.h>
#include "Utils.h"

typedef struct {
	HANDLE Serv_HMutex, Serv_HEvent;
	int carSpeed;
	int num_cars;
	int num_frogs;
	int car_pos[MAX_CARS][2]; //2 seria para representar o x e o y
	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
}GameData, *pGameData;

//buffer circular
typedef struct {
	int id;
	int val;
}CelulaBuffer;

//Memoria Partilhada para o Bffer circular
typedef struct {
	int nProdutores;
	int nConsumidores;
	int posEscrita;
	int posLeitura;
	CelulaBuffer buffer[TAM_BUFFER_CIRCULAR]; //Buffer circular em si
}BufferCircular;

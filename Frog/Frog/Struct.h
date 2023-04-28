#pragma once
#include <windows.h>
#include <tchar.h>
#include "Utils.h"

typedef struct {
	int col;
	int row;
}CarPos, * pCarPos;

typedef struct {
	HANDLE Serv_HMutex, Serv_HEvent;
	DWORD carSpeed;
	DWORD numCars;
	int numRoads;
	int num_frogs;
	CarPos car_pos[MAX_CARS]; 
	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
}GameData, *pGameData;

typedef struct {
	int val;
	int id;
}EspacoBuffer;

typedef struct {
	EspacoBuffer espacosDeBuffer[10];
	int posLeitura;
	int posEscrita;
	int nConsumidores;
	int nProdutores;
}Buffer, * pBuffer;

typedef struct {
	pBuffer BufferCircular;
	HANDLE hSemEscrita, hSemLeitura, hMutex;
	int id;
} TDados, * pTDados;
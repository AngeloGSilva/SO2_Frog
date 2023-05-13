#pragma once
#include <windows.h>
#include <tchar.h>
#include "Utils.h"

typedef struct {
	int col;
	int row;
}CarPos, *pCarPos;

typedef struct {
	int col;
	int row;
}FrogPos, *pFrogPos;

typedef struct {
	int *terminar;
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
	TCHAR val[20];
	int id;
}EspacoBuffer;

typedef struct {
	EspacoBuffer espacosDeBuffer[10];
	int posLeitura;
	int posEscrita;
	int nConsumidores;
	int nProdutores;
}Buffer, *pBuffer;

//Estrutura para movimento das Estradas Thread
typedef struct {
	int *terminar;
	int speed;
	int numCars;
	pCarPos car_pos;
	TCHAR* Map;
	int direction;
	pCarPos sharedCarPos;
	TCHAR* sharedMap;
	HANDLE hEventRoads, hMutex;
	int id;
} TRoads, *pTRoads;

//Estrutura para BufferCircular Thread
typedef struct {
	int *terminar;
	TCHAR* Map;
	TRoads* RoadsDirection;
	int numRoads;
	HANDLE* threadsHandles;
	pBuffer BufferCircular;
	HANDLE hSemEscrita, hSemLeitura, hMutex, hMutexInsertRoad;
	int id;
} TDados, *pTDados;

//Estrutura para Desenhar Inicio e fim Thread
typedef struct {
	int *terminar;
	int numRoads;
	pFrogPos frog_pos;
	TCHAR* Map;
	pFrogPos sharedFrogPos;
	TCHAR* sharedMap;
	HANDLE hEventRoads, hMutex;
	int id;
} TStartEnd, *pTStartEnd;

//Estrutura para Thread De para o jogo x tempo
typedef struct {
	int *terminar;
	HANDLE roadThreadsHandles;
	TCHAR* time;
	int numRoads;
} tStopGame, * pTStopGame;
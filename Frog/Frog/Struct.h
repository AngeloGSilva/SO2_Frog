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
	FrogPos frog_pos[MAX_FROGS];
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
	pFrogPos frog_pos;
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

//Estrutura para Thread que atualiza a posicao do sapo no mapa
typedef struct {
	HANDLE mutexRoads;
	TCHAR* Map;
	pFrogPos frog_pos;
}TdadosUpdateSapoMapa, * pTdadosUpdateSapoMapa;

//estrutra para passar os dados para as threads relacionadas com o named pipe
typedef struct {
	HANDLE hPipe[3];
	HANDLE hMutex; //para controlar o numClientes
	GameData* gamedatatemp;
	int numClientes;
	int terminar;
}TdadosPipeSendReceive, * pTdadosPipeSendReceive;

//estrutura que é enviada do cliente para o server com o input
typedef struct {
	int x;
	int y;
	int pressInput;
}PipeFroggeInput, *pPipeFroggeInput;
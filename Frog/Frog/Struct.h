#pragma once
#include <windows.h>
#include <tchar.h>
#include "Utils.h"

typedef struct {
	int col;
	int row;
}CarPos, *pCarPos;

//talvez trocar nome
typedef struct {
	char name[100];
	int time;
	int score;
	int level;
	int col;
	int row;
}FrogPos, *pFrogPos;

//Estrutura usada na thread de conectar Operadores
typedef struct {
	HANDLE InitialEvent;
	HANDLE GameDataEvent;
	HANDLE SharedMemoryEvent;
	HANDLE hEventRoads[MAX_ROWS];
	HANDLE hEventPipeWrite;
	HANDLE hEventPipeRead;
	HANDLE hEventFroggeMovement;
	HANDLE hEventFrogMovement;
	HANDLE hCountDownEvent;
}EventHandles, * pEventHandles;

typedef struct {
	HANDLE semSingleServer;
}SemaphoreHandles, * pSemaphoreHandles;

typedef struct {
	HANDLE mutexServerPipe;
	HANDLE mutexMapaChange;
	HANDLE mutexPipe;
	HANDLE mutexEventoEnviarMapaCliente;
	HANDLE mutexFrogMovement;
}MutexHandles, * pMutexHandles;

typedef struct {
	int nivel;
	int *terminar;
	HANDLE Serv_HMutex, Serv_HEvent;
	DWORD carSpeed;
	DWORD numCars;
	int numRoads;
	int num_frogs;
	CarPos car_pos[MAX_CARS]; 
	FrogPos frog_pos[MAX_FROGS];
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
	int directions[MAX_ROWS];
	int gamemode;
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
	int numRoads; //so para reset do sapo.. temporario
	int numCars;
	pCarPos car_pos;
	TCHAR* Map;
	int* direction;
	pCarPos sharedCarPos;
	TCHAR* sharedMap;
	MutexHandles mtxHandles;
	EventHandles evtHandles;
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
	EventHandles evtHandles;
	MutexHandles mtxHandles;
}TdadosUpdateSapoMapa, * pTdadosUpdateSapoMapa;

//estrutura para mandar para o cliente de cada vez
typedef struct {
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
	int numRoads;
	int directions[MAX_ROWS];
	FrogPos frog_pos[MAX_FROGS];
}PipeSendToClient, *pPipeSendToClient;

//estrutra para passar os dados para as threads relacionadas com o named pipe
typedef struct {
	HANDLE hPipe[3];
	EventHandles evtHandles;
	MutexHandles mtxHandles;
	PipeSendToClient structToSend;
	pFrogPos frogPos;
	int* directions;
	TCHAR *mapToShare;
	int numClientes;
	int terminar;
	pTRoads structToGetDirection;
	int* pGamemode;
}TdadosPipeSendReceive, * pTdadosPipeSendReceive;

//estrutura que é enviada do cliente para o server com o input
typedef struct {
	int x;
	int y;
	int pressInput;
}PipeFroggeInput, *pPipeFroggeInput;

//estrutura que é enviada do cliente para o server com o input
typedef struct {
	int Gamemode;
	TCHAR username[100];
}FrogInitialdata, * pFrogInitialdata;


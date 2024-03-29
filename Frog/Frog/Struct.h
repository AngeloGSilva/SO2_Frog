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
	char name[20];
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
	HANDLE hEventFroggeMovement[2];
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
	int numCars;
	int numRoads;
	int num_frogs;
	CarPos car_pos[MAX_CARS]; 
	FrogPos frog_pos[MAX_FROGS];
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
	int directions[MAX_ROWS];
	int gamemode;
	int time;
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
	int* numClientes;
	int *terminar;
	int speed;
	int numRoads; //so para reset do sapo.. temporario
	int* numCars;
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
	HANDLE* threadsHandles, StartEndThreads, hThreadsINFO;
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
	int* numFrogs;
	EventHandles evtHandles;
	MutexHandles mtxHandles;
}TdadosUpdateSapoMapa, * pTdadosUpdateSapoMapa;

//estrutura para mandar para o cliente de cada vez
typedef struct {
	TCHAR map[MAX_ROWS + 4][MAX_COLS];
	int numRoads;
	int directions[MAX_ROWS];
	FrogPos frog_pos[MAX_FROGS];
	int identifier;
	int time;
}PipeSendToClient, *pPipeSendToClient;

typedef struct {
	HANDLE hPipe;
	OVERLAPPED oOverlap;
	BOOL* ready;
}PipeInstance, * pPipeInstance;

//estrutra para passar os dados para as threads relacionadas com o named pipe
typedef struct {
	//HANDLE hPipe[3];
	PipeInstance hPipe;
	EventHandles evtHandles;
	MutexHandles mtxHandles;
	PipeSendToClient structToSend;
	pFrogPos frogPos;
	int* directions;
	TCHAR *mapToShare;
	int *numClientes;
	int terminar;
	pTRoads structToGetDirection;
	int* pGamemode;
	int clienteIdentificador;
	int* time;
	pGameData gameToShare;
	pGameData realGame;
	int timeInactive;
}TdadosPipeSendReceive, * pTdadosPipeSendReceive;


typedef struct {
	PipeInstance hPipe[2];
	EventHandles evtHandles;
	MutexHandles mtxHandles;
	PipeSendToClient structToSend;
	pFrogPos frogPos;
	TCHAR* mapToShare;
	int numClientes;
	int terminar;
	pTRoads structToGetDirection;
	int* pGamemode;
	int* time;
}TdadosPipeSend, * pTdadosPipeSend;

//estrutura que � enviada do cliente para o server com o input
//TODO X PARA IDENTIFICADOR
typedef struct {
	int x;
	int y;
	int pressInput;
}PipeFroggeInput, *pPipeFroggeInput;

//estrutura que � enviada do cliente para o server com o input
typedef struct {
	int Gamemode;
	TCHAR username[100];
	int identifier;
}FrogInitialdata, * pFrogInitialdata;


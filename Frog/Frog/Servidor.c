#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "Utils.h"
#include "Struct.h"
#include "Registry.h"
#include "../SharedMemory/SharedMemory.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

void resetMapCars(TCHAR* map, int numRoads, pCarPos car_pos, int* numCars,HANDLE hmutex) {

		//desenho do mapa... limites do mapa e numero de estradas
		WaitForSingleObject(hmutex, INFINITE);
		*numCars = 0;

	for (int i = 0; i < numRoads + SKIP_BEGINING_END; i++)
	{
		for (int j = 0; j < MAX_COLS; j++)
		{
			if (i == 0 || j == 0 || i == numRoads + 3 || j == MAX_COLS - 1)
				map[i * MAX_COLS + j] = BLOCK_ELEMENT;
			else if (i == 1 || i == numRoads + SKIP_BEGINING) {
				map[i * MAX_COLS + j] = BEGIN_END_ELEMENT;
			}
			else if (i == 4 && j == 10) {
				map[i * MAX_COLS + j] = OBSTACLE_ELEMENT;
			}
			else
				map[i * MAX_COLS + j] = ROAD_ELEMENT;

		}
	}
	_tprintf(TEXT("[DEBUG]Novo mapa gerado\n"));

	//Reset ao array car_pos
	for (int i = 0; i < MAX_CARS; i++)
	{
		car_pos[i].col = -1;
		car_pos[i].row = -1;
	}


	//Gerar carros
	for (int i = 0; i < numRoads; i++)
	{
		int carsInRoad = (rand() % 8) + 1;
		_tprintf(TEXT("[DEBUG]Numero novo de carros gerado para a estrada %d é %d\n"),i, carsInRoad);
		for (; carsInRoad > 0; carsInRoad--) {
			car_pos[*numCars].row = i + SKIP_BEGINING; //X -> linha
			int posInRoad = 0;
			do {
				posInRoad = (rand() % (MAX_COLS - 2)) + 1;
			} while (map[(i + SKIP_BEGINING) * MAX_COLS + posInRoad] == 'H' || map[(i + SKIP_BEGINING) * MAX_COLS + posInRoad] == '#');
			//_tprintf(TEXT("PUS AQUI %d %d\n"), posInRoad, i + SKIP_BEGINING);
			car_pos[*numCars].col = posInRoad; //y -> coluna
			map[(i + SKIP_BEGINING) * MAX_COLS + posInRoad] = CAR_ELEMENT;
			*numCars = *numCars + 1;
		}
	}
	_tprintf(TEXT("[DEBUG]Novo numero total de carros %d\n"), *numCars);
	ReleaseMutex(hmutex);
}

void copyMapArray(int numRoads, TCHAR *mapOriginal, TCHAR *mapSend) {
	for (int i = 0; i < numRoads + 4; i++)
	{
		for (int j = 0; j < MAX_COLS; j++) {
			mapSend[i * MAX_COLS + j] = mapOriginal[i * MAX_COLS + j];
		}
	}
}

DWORD WINAPI send(LPVOID lpParam)
{
	pTdadosPipeSend dados = (pTdadosPipeSend)lpParam;
	TCHAR buf[256];
	DWORD n;
		
	for (int i = 0; i < dados->numClientes; i++) {
		dados->structToSend.frog_pos[i].level = dados->frogPos[i].level;
		dados->structToSend.frog_pos[i].score = dados->frogPos[i].score;
	}

	while(1){
		//WaitForSingleObject(dados->hMutex, INFINITE);
		WaitForSingleObject(dados->evtHandles.hEventPipeWrite,INFINITE);
		Sleep(100);
		WaitForSingleObject(dados->mtxHandles.mutexServerPipe, INFINITE);
		copyMapArray(dados->structToSend.numRoads,dados->mapToShare, dados->structToSend.map);

		for (int i = 0; i < dados->structToSend.numRoads; i++)
		{
			dados->structToSend.directions[i] = dados->structToGetDirection[i].direction;
			//_tprintf(TEXT("[DEBUG] direcao [%d]... (WriteFile)\n"), dados->structToGetDirection[i].direction);
		}
		
		//dados->structToSend.frog_pos->time = dados->frogPos->time;
		//CopyMemory(dados->structToSend.map, dados->mapToShare, sizeof(dados->mapToShare));
		for (int i = 0; i < dados->numClientes; i++) {
			if (dados->hPipe[i].ready == TRUE) {
				dados->structToSend.identifier = i;
				dados->structToSend.frog_pos[i].score = dados->frogPos[i].score;
				dados->structToSend.frog_pos[i].level = dados->frogPos[i].level;
				dados->structToSend.frog_pos[i].col = dados->frogPos[i].col;
				dados->structToSend.frog_pos[i].row = dados->frogPos[i].row;
				dados->structToSend.time = *dados->time;
				if (!WriteFile(dados->hPipe[i].hPipe, &dados->structToSend, sizeof(PipeSendToClient), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
				_tprintf(TEXT("[send] Enviei %d bytes ao leitor [%d]... (WriteFile)\n"), n, i);

			}
		}
		ReleaseMutex(dados->mtxHandles.mutexServerPipe);
		SetEvent(dados->evtHandles.hEventPipeRead);
		ResetEvent(dados->evtHandles.hEventPipeRead);
		/*ReleaseMutex(dados->mtxHandles.hMutex);*/
	} 
	dados->terminar = 1;
	return 1;

}

void HandleFroggeMovement(int frogge, PipeFroggeInput input, pFrogPos pos, TCHAR* map, int numRoads, pTRoads structToRoads,int Gamemode) {
	//TODO nao sei se +e o sito certo para fazer o evento de atualizar o mapa com o sapo
	WaitForSingleObject(structToRoads->mtxHandles.mutexMapaChange, INFINITE);
	WaitForSingleObject(structToRoads->mtxHandles.mutexFrogMovement, INFINITE);
	
	BOOL levelUp = FALSE;
	BOOL colisaoReset = FALSE;
	
	switch (input.pressInput)
	{
	case KEY_UP:
		if(pos[frogge].row - 1 != 0)
		{
			if(pos[frogge].row == 1 || pos[frogge].row == numRoads + 2)
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = BEGIN_END_ELEMENT;
			else if (map[(pos[frogge].row - 1) * MAX_COLS + pos[frogge].col] == OBSTACLE_ELEMENT || map[(pos[frogge].row - 1) * MAX_COLS + pos[frogge].col] == FROGGE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch iobstacle\n"));
				break;
			}
			else
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = ROAD_ELEMENT;

			//movimento
			pos[frogge].row = pos[frogge].row - 1;

			if (map[pos[frogge].row * MAX_COLS + pos[frogge].col] == CAR_ELEMENT) {
				colisaoReset = TRUE;
			} else if(pos[frogge].row == 1)
			{
				levelUp = TRUE;
				colisaoReset = TRUE;
			}
		}
		break;
	case KEY_DOWN:
		//passar as estradas para aqui
		if (pos[frogge].row + 1 != numRoads + 3)
		{
			if (map[(pos[frogge].row + 1) * MAX_COLS + pos[frogge].col] == FROGGE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch Frog!\n"));
				break;
			}
		else if (pos[frogge].row == 1 || pos[frogge].row == numRoads + 2)
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = BEGIN_END_ELEMENT;
			else if (map[(pos[frogge].row + 1) * MAX_COLS + pos[frogge].col] == OBSTACLE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch iobstacle\n"));
				break;
			}
			else
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = ROAD_ELEMENT;

			//movimento
			pos[frogge].row = pos[frogge].row + 1;

			if (map[pos[frogge].row * MAX_COLS + pos[frogge].col] == CAR_ELEMENT) {
				colisaoReset = TRUE;
			}
			
		}
		break;
	case KEY_LEFT:
		if (pos[frogge].col - 1 != 0)
		{
			if (map[pos[frogge].row * MAX_COLS + (pos[frogge].col - 1)] == FROGGE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch Frog!\n"));
				break;
			}
			else if (pos[frogge].row == 1 || pos[frogge].row == numRoads + 2)
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = BEGIN_END_ELEMENT;
			else if (map[pos[frogge].row * MAX_COLS + (pos[frogge].col - 1)] == OBSTACLE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch iobstacle\n"));
				break;
			}
			else
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = ROAD_ELEMENT;
			
			//movimento
			pos[frogge].col = pos[frogge].col - 1;

			if (map[pos[frogge].row * MAX_COLS + pos[frogge].col] == CAR_ELEMENT) {
				colisaoReset = TRUE;
			}
			
		}
		break;
	case KEY_RIGHT:
		//esta merda ta foda e nao sei pq
		if (pos[frogge].col + 1 != MAX_COLS - 1)
		{
			if (map[pos[frogge].row * MAX_COLS + (pos[frogge].col + 1)] == FROGGE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch Frog!\n"));
				break;
			}
			if (pos[frogge].row == 1 || pos[frogge].row == numRoads + 2)
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = BEGIN_END_ELEMENT;
			else if (map[pos[frogge].row * MAX_COLS + (pos[frogge].col + 1)] == OBSTACLE_ELEMENT) {
				_tprintf(TEXT("[DEBUG] Ouch iobstacle\n"));
				break;
			}
			else
				map[pos[frogge].row * MAX_COLS + pos[frogge].col] = ROAD_ELEMENT;

			//movimento
			pos[frogge].col = pos[frogge].col + 1;

			if (map[pos[frogge].row * MAX_COLS + pos[frogge].col] == CAR_ELEMENT) {
				colisaoReset = TRUE;
			}
		}
		break;
	case REPOSITION:
		if (pos[input.x].row == numRoads + 2)
			map[pos[frogge].row * MAX_COLS + pos[frogge].col] = BEGIN_END_ELEMENT;
		else
			map[pos[frogge].row * MAX_COLS + pos[frogge].col] = ROAD_ELEMENT;

		pos[input.x].row = numRoads + 2;
		pos[input.x].col = (rand() % (MAX_COLS - 2)) + 1;
		break;
	default:
		break;
	}
	if (levelUp) {
		_tprintf(TEXT("[DEBUG] PASSOU DE NIVEL\n"));
		pos[frogge].score += 1;
		if (Gamemode == MULTIPLAYER) {
			for (int i = 0; i < 2; i++) {
				pos[i].level += 1;
			}
		}else
			pos[frogge].level += 1;

		_tprintf(TEXT("[DEBUG] Pontuacao passou a %d\n"), pos[frogge].score);

		//int newSpeed = ((rand() % 8) + 1) * 1000;
		//_tprintf(TEXT("[DEBUG] Nova Speed é %d, antiga speed é %d\n"), newSpeed,structToRoads[0].speed);

		for (int i = 0; i < numRoads; i++)
		{
			//structToRoads[i].speed = newSpeed;
			structToRoads[i].speed = ((rand() % 8) + 1) * 1000 / pos[frogge].level;
			if (structToRoads[i].speed < 700)
				structToRoads[i].speed = 700;
			structToRoads[i].direction = (rand() % 2);
			//Adicionar uns segundos ao tempo?
			_tprintf(TEXT("[DEBUG] Nova direcao da estrada %d\n"), structToRoads[i].direction);
		}

		//alterar numero de carros, ainda tenho de ver como se pode fazer (nao faço ideia)
		resetMapCars(map,numRoads,structToRoads->car_pos,structToRoads->numCars, structToRoads->mtxHandles.mutexMapaChange);
	}
	//else if coma  lógica
	if (colisaoReset) {
		_tprintf(TEXT("PERDEU PQ ESTA NUM CARRO\n"));
		if (Gamemode == MULTIPLAYER && levelUp) {
			for (int i = 0; i < 2; i++) {
				pos[i].row = numRoads + 2;
				pos[i].col = (rand() % (MAX_COLS - 2)) + 1;
			}
		}
		else {
			pos[frogge].row = numRoads + 2;
			pos[frogge].col = (rand() % (MAX_COLS - 2)) + 1;
		}
	}

	SetEvent(structToRoads->evtHandles.hEventFrogMovement);
	ResetEvent(structToRoads->evtHandles.hEventFrogMovement);
	ReleaseMutex(structToRoads->mtxHandles.mutexFrogMovement);
	ReleaseMutex(structToRoads->mtxHandles.mutexMapaChange);
}

DWORD WINAPI ThreadSapos(LPVOID lpParam)
{
	pTdadosUpdateSapoMapa data = (pTdadosUpdateSapoMapa)lpParam;
	
	while (1) {
		WaitForSingleObject(data->evtHandles.hEventFrogMovement,INFINITE);
		WaitForSingleObject(data->mtxHandles.mutexMapaChange, INFINITE);
		for(int i=0;i<*data->numFrogs;i++)
			data->Map[data->frog_pos[i].row * MAX_COLS + data->frog_pos[i].col] = FROGGE_ELEMENT;
		ReleaseMutex(data->mtxHandles.mutexMapaChange);
		WaitForSingleObject(data->mtxHandles.mutexEventoEnviarMapaCliente, INFINITE);
		SetEvent(data->evtHandles.hEventPipeWrite, INFINITE);
		ResetEvent(data->evtHandles.hEventPipeWrite);
		ReleaseMutex(data->mtxHandles.mutexEventoEnviarMapaCliente);
	}
	return 1;
}

DWORD WINAPI ThreadGameTimer(LPVOID lpParam) {
	pTdadosPipeSendReceive data = (pTdadosPipeSendReceive)lpParam;
	HANDLE timerevent = CreateEvent(NULL, TRUE, FALSE, TEXT("countdownevent"));
	HANDLE gamedataevent= CreateEvent(NULL, TRUE, FALSE, GAMEDATA_EVENT);
	while (*data->time > 0)
	{
		SharedMemoryMap(data->gameToShare, data->realGame);
		Sleep(1000);
		*data->time = *data->time - 1;
		WaitForSingleObject(data->mtxHandles.mutexEventoEnviarMapaCliente, INFINITE);
		SetEvent(data->evtHandles.hEventPipeWrite);
		ResetEvent(data->evtHandles.hEventPipeWrite);
		SetEvent(gamedataevent);
		ResetEvent(gamedataevent);

		ReleaseMutex(data->mtxHandles.mutexEventoEnviarMapaCliente);
	}
	_tprintf(TEXT("[INFO]TIME OVER!\n"));
	SetEvent(timerevent);
	ResetEvent(timerevent);
	return 1;
}

DWORD WINAPI ThreadInactivePlayer(LPVOID lpParam) {
	pTdadosPipeSendReceive data = (pTdadosPipeSendReceive)lpParam;
	HANDLE Active = CreateEvent(NULL, TRUE, FALSE, TEXT("Active"));
	HANDLE hmutexInactive = CreateMutex(NULL, FALSE, TEXT("inactive"));

	BOOL inactive = FALSE;
	while (1)
	{
		for (int i = 0; i < 10; i++) {
			Sleep(1000);
			WaitForSingleObject(hmutexInactive,INFINITE);
			data->timeInactive = data->timeInactive + 1;
			ReleaseMutex(hmutexInactive);
		}
		if (data->timeInactive > 10) {
			WaitForSingleObject(data->mtxHandles.mutexMapaChange, INFINITE);
			WaitForSingleObject(data->mtxHandles.mutexFrogMovement, INFINITE);
			if (data->frogPos[data->clienteIdentificador].row == data->structToGetDirection->numRoads + 2)
				data->mapToShare[data->frogPos[data->clienteIdentificador].row * MAX_COLS + data->frogPos[data->clienteIdentificador].col] = BEGIN_END_ELEMENT;
			else
				data->mapToShare[data->frogPos[data->clienteIdentificador].row * MAX_COLS + data->frogPos[data->clienteIdentificador].col] = ROAD_ELEMENT;

			data->frogPos[data->clienteIdentificador].row = data->structToGetDirection->numRoads + 2;
			data->frogPos[data->clienteIdentificador].col = (rand() % (MAX_COLS - 2)) + 1;
			ReleaseMutex(data->mtxHandles.mutexMapaChange);
			ReleaseMutex(data->mtxHandles.mutexFrogMovement);
			SetEvent(data->evtHandles.hEventFrogMovement);
			ResetEvent(data->evtHandles.hEventFrogMovement);
		}
	}
	return 1;
}

DWORD WINAPI receive(LPVOID lpParam)
{
	TdadosPipeSendReceive* dados = (TdadosPipeSendReceive*)lpParam;
	TCHAR buf[256];
	BOOL ret;
	DWORD n;
	PipeFroggeInput receiveInfo;
	FrogInitialdata froginitialdata;
	HANDLE clientGamemode = CreateEvent(NULL, TRUE, FALSE, TEXT("clientgamemode"));
	HANDLE Active = CreateEvent(NULL, TRUE, FALSE, TEXT("Active"));
	HANDLE hmutexInactive = CreateMutex(NULL, FALSE, TEXT("inactive"));
	//Passar para a outra thread
	WaitForSingleObject(dados->evtHandles.hEventFroggeMovement[dados->clienteIdentificador], INFINITE);
	ReadFile(dados->hPipe.hPipe, &froginitialdata, sizeof(FrogInitialdata), &n, NULL);
	_tprintf(TEXT("[receive] Recebi %d bytes... (ReadFile)\n"), n);
	
	*dados->pGamemode = froginitialdata.Gamemode;
	SetEvent(clientGamemode);
	ResetEvent(clientGamemode);

	_tprintf(TEXT("CONEXÂO POR %s com o modo de jogo %d\n"), froginitialdata.username,froginitialdata.Gamemode);

	//altera o nivel e pontuação por alguma razão
	//_tcscpy_s(dados->frogPos[dados->clienteIdentificador].name, sizeof(froginitialdata.username), froginitialdata.username);

	if (*dados->pGamemode == MULTIPLAYER)
	{	
		while (*dados->numClientes < 2)
		{
			_tprintf(TEXT("Espera de outro jogador\n"));
		}
	}
	//Thread do game timer
	if (dados->clienteIdentificador == 0) {
		HANDLE FroggeThread = CreateThread(
			NULL,
			0,
			ThreadGameTimer,
			dados,
			0,
			NULL);
	}
	//Thread de inatividade
	
		HANDLE FroggeThread = CreateThread(
			NULL,
			0,
			ThreadInactivePlayer,
			dados,
			0,
			NULL);
	
	while (1) {
		WaitForSingleObject(dados->evtHandles.hEventFroggeMovement[dados->clienteIdentificador], INFINITE);

		WaitForSingleObject(dados->mtxHandles.mutexServerPipe, INFINITE);
		ret = ReadFile(dados->hPipe.hPipe, &receiveInfo, sizeof(PipeFroggeInput), &n, NULL);
		_tprintf(TEXT("[receive] Recebi %d bytes: '%d' do %d... (ReadFile)\n"), n, receiveInfo.pressInput, dados->clienteIdentificador);
		WaitForSingleObject(hmutexInactive, INFINITE);
		dados->timeInactive = 0;
		ReleaseMutex(hmutexInactive);
		ReleaseMutex(dados->mtxHandles.mutexServerPipe);
		WaitForSingleObject(dados->mtxHandles.mutexMapaChange, INFINITE);
		HandleFroggeMovement(dados->clienteIdentificador, receiveInfo, dados->frogPos,dados->mapToShare,dados->structToSend.numRoads,dados->structToGetDirection,*dados->pGamemode);
		ReleaseMutex(dados->mtxHandles.mutexMapaChange);
	}

	dados->terminar = 1;
	return 1;

}

DWORD WINAPI ThreadRoads(LPVOID lpParam)
{
	pTRoads data = (pTRoads)lpParam;
	pCarPos temp;
	_tprintf(TEXT("[INFO] INICIO DA THREAD ROAD %d\n"), data->id);
	while (*data->terminar == 0)
	{
		WaitForSingleObject(data->mtxHandles.mutexMapaChange, INFINITE);
		temp = data->car_pos;
		//movimento carros
		for (int i = 0; i < *data->numCars; i++)
		{
			int x = temp[i].row;
			if (x == data->id)
			{
				int y = temp[i].col;

				if (data->direction == ROAD_RIGHT) {
					//carros em fila
					if (data->Map[x * MAX_COLS + y + 1] == BLOCK_ELEMENT && data->Map[x * MAX_COLS + 1] == CAR_ELEMENT)
						y = 1;

					if (data->Map[x * MAX_COLS + y + 1] == CAR_ELEMENT) {
						int testing = y;
						int increment = 1;
						while (data->Map[x * MAX_COLS + testing + increment] == CAR_ELEMENT || data->Map[x * MAX_COLS + testing + increment] == BLOCK_ELEMENT) {
							if (testing + increment == 19) {
								testing = 1;
								increment = -1;
							}
							increment += 1;
						}
						if (data->Map[x * MAX_COLS + testing + increment] != OBSTACLE_ELEMENT) {
							if (temp[i].col + 1 == 19) {
								temp[i].col = 1;
							}
							else
								temp[i].col++;
						}
						else
							continue;
					}
					else if (data->Map[x * MAX_COLS + y + 1] != OBSTACLE_ELEMENT) {
						if (temp[i].col + 1 == 19) {
							temp[i].col = 1;
						}
						else
							temp[i].col++;
					}
				}
				//logica que acho q é para a esquerda
				else if (data->direction == ROAD_LEFT) {

					// carros em fila
					if (data->Map[x * MAX_COLS + y - 1] == BLOCK_ELEMENT)
						y = MAX_COLS - 1;

					if (data->Map[x * MAX_COLS + y - 1] == CAR_ELEMENT) {
						int testing = y;
						int increment = -1;
						while (data->Map[x * MAX_COLS + testing + increment] == CAR_ELEMENT || data->Map[x * MAX_COLS + testing + increment] == BLOCK_ELEMENT) {
							if (testing + increment == 0) {
								testing = MAX_COLS - 2;
								increment = 1;
							}
							increment -= 1;
						}
						if (data->Map[x * MAX_COLS + testing + increment] != OBSTACLE_ELEMENT) {
							if (temp[i].col - 1 == 0) {
								temp[i].col = MAX_COLS - 2;
							}
							else
								temp[i].col--;
						}
						else
							continue;
					}
					else if (data->Map[x * MAX_COLS + y - 1] != OBSTACLE_ELEMENT) {
						if (temp[i].col - 1 == 0) {
							temp[i].col = MAX_COLS - 2;
						}
						else
							temp[i].col--;
					}
				}
			}
		}

		//TODO provavelmente tem de sair daqui.. (Reset do mapa antes de atualizar as posicoes dos carros)
		//è preciso refazer esta lógica toda
		BOOL existe = FALSE;
		for (int i = 1; i < MAX_COLS - 1; i++)
		{
			existe = FALSE;
			if (data->Map[data->id * MAX_COLS + i] == OBSTACLE_ELEMENT)
			{
				data->Map[data->id * MAX_COLS + i] = OBSTACLE_ELEMENT;
			}
			else if (data->Map[data->id * MAX_COLS + i] == FROGGE_ELEMENT)
			{
				
				for (int j = 0; j < *data->numClientes; j++)
				{
					if (data->frog_pos[j].row == data->id && data->frog_pos[j].col == i) 
					{
						existe = TRUE;
					}

				}

				if(existe) {
					data->Map[data->id * MAX_COLS + i] = FROGGE_ELEMENT;
				}
				else
					data->Map[data->id * MAX_COLS + i] = ROAD_ELEMENT;

			}
			else
			{
				data->Map[data->id * MAX_COLS + i] = ROAD_ELEMENT;
			}
		}




		// refazer linha
		for (int i = 0; i < *data->numCars; i++)
		{
			int x = temp[i].row;
			if (x == data->id)
			{
				//isto meio q faz o programa ir com o caracas.. mas nao percebi bem pq
				if (data->Map[temp[i].row * MAX_COLS + temp[i].col] == FROGGE_ELEMENT) {
					//isto vai depois ser um flag q é atualizada para informar que o sapo perdeu ou no multiplayer tem de voltar para o inicio
					_tprintf(TEXT("[DEBUG] Carro bateu em sapo\n"));

					for (int j = 0; j < *data->numClientes; j++)
					{
						if (data->frog_pos[j].row == temp[i].row && data->frog_pos[j].col == temp[i].col) 
						{
							data->frog_pos[j].row = data->numRoads + 2;
							data->frog_pos[j].col = (rand() % (MAX_COLS - 2)) + 1;
							//isto nao pode ser assim pq deixa o 'corpo' do sapo para tras e nao o reseta... ou nao pq depois meto o carro.. ja nao sei bem
							data->Map[data->frog_pos[j].row * MAX_COLS + data->frog_pos[j].col] = FROGGE_ELEMENT;
						}
					}
					
				}
				data->Map[temp[i].row * MAX_COLS + temp[i].col] = CAR_ELEMENT;
			}
		}
		//copyMemoryOperation(data->sharedMap, data->Map, sizeof(TCHAR) * (MAX_ROWS + SKIP_BEGINING_END) * MAX_COLS);
		SharedMemoryMapThreadRoads(data);
		ReleaseMutex(data->mtxHandles.mutexMapaChange);

		//Criamos evento para que as threads ja consiga ler
		SetEvent(data->evtHandles.hEventRoads[data->id - SKIP_BEGINING]);
		ResetEvent(data->evtHandles.hEventRoads[data->id - SKIP_BEGINING]);
		Sleep(data->speed);
		//HANDLE hidk = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoPipeWrite"));
		//HANDLE hMutexEventoEnviarMapaCliente = CreateMutex(NULL, FALSE, TEXT("MutexServerPipeEsperarEnviarEventoParaEnviarMapaCliente"));
		WaitForSingleObject(data->mtxHandles.mutexEventoEnviarMapaCliente,INFINITE);
		SetEvent(data->evtHandles.hEventPipeWrite, INFINITE);
		ResetEvent(data->evtHandles.hEventPipeWrite);
		ReleaseMutex(data->mtxHandles.mutexEventoEnviarMapaCliente);
	}
	return 0;
}

DWORD WINAPI CheckOperators(LPVOID lpParam)
{
	pEventHandles evt = (pEventHandles)lpParam;

	while (1) {
		//Criamos evento para que as threads ja consiga ler
		_tprintf(TEXT("[INFO] Espera do evento check\n"));
		WaitForSingleObject(evt->InitialEvent, INFINITE);

		SetEvent(evt->SharedMemoryEvent);
		Sleep(500);
		ResetEvent(evt->SharedMemoryEvent);
		SetEvent(evt->GameDataEvent);
		Sleep(500);
		ResetEvent(evt->GameDataEvent);
	}
}

void ToggleRoadDirection(pTRoads roads, int numRoads, int roadId) {
	for (int i = 0; i < numRoads; i++) {
		if (roads[i].id == roadId) {
			roads[i].direction = (roads[i].direction == ROAD_RIGHT) ? ROAD_LEFT : ROAD_RIGHT;
			break;
		}
	}
}

void HandleStopCommand(HANDLE *dados, int numRoads) {
	for (int i = 0; i < numRoads; i++) {
		SuspendThread(dados[i]);
	}
}

void HandleStartCommand(HANDLE* dados, int numRoads) {
	for (int i = 0; i < numRoads; i++) {
		ResumeThread(dados[i]);
	}
}

DWORD WINAPI threadStopGame(LPVOID lpParam)
{
	pTStopGame dados = (pTStopGame)lpParam;
	int time = 0;
	time = _wtoi(dados->time);
	time = time * 1000;
	HandleStopCommand(dados->roadThreadsHandles, dados->numRoads);
	Sleep(time); // for the specified time in seconds
	HandleStartCommand(dados->roadThreadsHandles, dados->numRoads);
}

void HandleChangeCommand(pTDados dados, const TCHAR* firstNumber) {
	int roadId = _wtoi(firstNumber);
	ToggleRoadDirection(dados->RoadsDirection, dados->numRoads, roadId + 1);
}

void HandleInsertCommand(pTDados dados, const TCHAR* firstNumber, const TCHAR* secondNumber) {
	int roadId = _wtoi(firstNumber);
	int coluna = _wtoi(secondNumber);

	_tprintf(TEXT("Testing[%s]!"), firstNumber);
	_tprintf(TEXT("Testing[%s]!"), secondNumber);

	if (roadId >= 1 && roadId <= dados->numRoads + 2)
		_tprintf(TEXT("Valido[%d]!\n"), roadId + 1);
	else
		_tprintf(TEXT("Invalido!\n"));

	if (coluna >= 1 && coluna <= 18)
		_tprintf(TEXT("Valido[%d]!"), coluna);
	else
		_tprintf(TEXT("Invalido!"));

	WaitForSingleObject(dados->hMutexInsertRoad, INFINITE);
	if (dados->Map[(roadId + 1) * MAX_COLS + coluna] == ROAD_ELEMENT) {
		dados->Map[(roadId + 1) * MAX_COLS + coluna] = OBSTACLE_ELEMENT;
	}
	ReleaseMutex(dados->hMutexInsertRoad);
}

void HandleDeleteCommand(pTDados dados, const TCHAR* firstNumber, const TCHAR* secondNumber) {
	int roadId = _wtoi(firstNumber);
	int coluna = _wtoi(secondNumber);

	if (roadId >= 1 && roadId <= dados->numRoads + 2)
		_tprintf(TEXT("Valido[%d]!"), roadId + 1);
	else
		_tprintf(TEXT("Invalido!"));

	if (coluna >= 1 && coluna <= 18)
		_tprintf(TEXT("Valido[%d]!"), coluna);
	else
		_tprintf(TEXT("Invalido!"));

	WaitForSingleObject(dados->hMutexInsertRoad, INFINITE);
	if (dados->Map[(roadId + 1) * MAX_COLS + coluna] == OBSTACLE_ELEMENT) {
		dados->Map[(roadId + 1) * MAX_COLS + coluna] = ROAD_ELEMENT;
	}
	ReleaseMutex(dados->hMutexInsertRoad);
}

DWORD WINAPI ThreadBufferCircular(LPVOID lpParam)
{
	pTDados dados = (pTDados)lpParam;
	EspacoBuffer space;
	space.id = 0;
	HANDLE hThreadStopGame = NULL;
	while (*dados->terminar == 0)
	{
		//WaitForSingleObject(dados->hSemLeitura, INFINITE);
		//WaitForSingleObject(dados->hMutex, INFINITE);
		////copiar o conteudo para a memoria partilhada
		//CopyMemory(&space, &dados->BufferCircular->espacosDeBuffer[dados->BufferCircular->posLeitura], sizeof(EspacoBuffer));
		//dados->BufferCircular->posLeitura++;
		//if (dados->BufferCircular->posLeitura == 10)
		//{
		//	dados->BufferCircular->posLeitura = 0;
		//}

		space = ReadSharedMemoryServer(dados->BufferCircular);

		TCHAR* token;
		TCHAR* token2;
		TCHAR* context;
		TCHAR command[20] = TEXT("empty");
		TCHAR firstNumber[20] = TEXT("empty");
		TCHAR secondNumber[20] = TEXT("empty");

		_tprintf(TEXT("Received from operator:[%s]\n"), space.val);


		token = _tcstok_s(space.val, _T(" "), &context);
		if (token != NULL)
			_tcscpy_s(command, 20, token);

		if (lstrcmp(command, TEXT("stop")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			tStopGame stopGame;
			stopGame.roadThreadsHandles = dados->threadsHandles;
			stopGame.time = firstNumber;//para ser em segundos
			stopGame.numRoads = dados->numRoads;
			hThreadStopGame = CreateThread(
				NULL,    // Thread attributes
				0,       // Stack size (0 = use default)
				threadStopGame, // Thread start address
				&stopGame,    // Parameter to pass to the thread
				0,       // Creation flags
				NULL);   // Thread id   // returns the thread identifier
			if (hThreadStopGame == NULL)
			{
				_tprintf(TEXT("[ERRO] Thread BufferCircular Erro ao criar thread de pausa do jogo\n"));
				return 1;
			}
		}
		else if (lstrcmp(command, TEXT("terminar")) == 0) {
			//acabar com tudo
			*dados->terminar = 1;
		}
		else if (lstrcmp(command, TEXT("start")) == 0) {
			//HandleStartCommand(dados);
		}
		else if (lstrcmp(command, TEXT("change")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
			{
				_tcscpy_s(firstNumber, 20, token);
				HandleChangeCommand(dados, firstNumber);
			}
		}
		else if (lstrcmp(command, TEXT("insert")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			token2 = token;
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(secondNumber, 20, token);
			if(token2 != NULL && token != NULL && lstrcmp(firstNumber, TEXT("empty"))!=0 && lstrcmp(secondNumber, TEXT("empty")) != 0)
				HandleInsertCommand(dados,firstNumber,secondNumber);
		}
		else if (lstrcmp(command, TEXT("remove")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(firstNumber, 20, token);
			token2 = token;
			token  = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL)
				_tcscpy_s(secondNumber, 20, token);
			if (token2 != NULL && token != NULL && lstrcmp(firstNumber, TEXT("empty")) != 0 && lstrcmp(secondNumber, TEXT("empty")) != 0)
			HandleDeleteCommand(dados, firstNumber, secondNumber);
		}
		//ReleaseMutex(dados->hMutex);
		//ReleaseSemaphore(dados->hSemEscrita, 1, NULL);
	}
	CloseHandle(hThreadStopGame);
	return 0;
}

int parse_args(TCHAR* arg1_str, TCHAR* arg2_str, DWORD* arg1, DWORD* arg2) {
	_tprintf(TEXT("%s %s \n"), arg1_str, arg2_str);

	*arg1 = _wtoi(arg1_str);
	*arg2 = _wtoi(arg2_str);

	if (!arg1 || !arg2) {
		_tprintf(TEXT("[ERRO] Formato invalido para argumentos\n"));
		return 1;
	}

	_tprintf(TEXT("[INFO] arg1 = %lu\n"), *arg1);
	_tprintf(TEXT("[INFO] arg2 = %lu\n"), *arg2);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

	int terminar = 0; // para acabar com tudo

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	srand((unsigned)time(NULL));

	DWORD arg1 = 0, arg2 = 0, parse_result = 0;

	//estrutra com toda a info do jogo
	GameData data;
	EventHandles evtHandles;
	SemaphoreHandles smphHandles;
	MutexHandles mtxHandles;
	
	if (argc != 3) {
		_tprintf(TEXT("[ERRO] Parametros errados \n"));
		//função que vai buscar ao registry se existir, senão return 1;
		data = RegistryGetValues();
	}
	else {
		parse_result = parse_args(argv[1], argv[2], &arg1, &arg2);
		if (parse_result != 0) {
			_tprintf(TEXT("[ERRO] Conversao de argumentos falhada\n"));
			return 1;
		}
		_tprintf(TEXT("[INFO] Conversao completa!\n arg1 :%lu \narg2 :%lu \n"), arg1, arg2);
		data = RegistryKeyValue(arg1, arg2);
	}

	//criar Semaf para apenas 1 server
	smphHandles.semSingleServer = CreateSemaphore(NULL, 1, 1, SEMAPHORE_UNIQUE_SERVER);
	if (smphHandles.semSingleServer == NULL)
	{
		_tprintf("[ERRO] CreateSemaphore: %d\n", GetLastError());
		return 1;
	}

	//server limit 1 (2 segundos max wait)
	DWORD instanceServer = WaitForSingleObject(smphHandles.semSingleServer, 2000);
	if (instanceServer == WAIT_TIMEOUT) {
		_tprintf(TEXT("[INFO] Ja existe um servidor a correr!\n"));
		return 1;
	}

	evtHandles.InitialEvent = CreateEvent(NULL, TRUE, FALSE, INITIAL_EVENT);
	evtHandles.GameDataEvent = CreateEvent(NULL, TRUE, FALSE, GAMEDATA_EVENT);
	evtHandles.SharedMemoryEvent = CreateEvent(NULL, TRUE, FALSE, SHARED_MEMORY_EVENT);
	evtHandles.hEventPipeWrite = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoPipeWrite"));

	mtxHandles.mutexEventoEnviarMapaCliente = CreateMutex(NULL, FALSE, TEXT("MutexServerPipeEsperarEnviarEventoParaEnviarMapaCliente"));
	mtxHandles.mutexServerPipe = CreateMutex(NULL, FALSE, TEXT("MutexServerPipe"));

	evtHandles.hEventPipeRead = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoPipeRead"));
	int trash = 0, trash1 = 1;
	evtHandles.hEventFroggeMovement[0] = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoSapo") + trash);
	evtHandles.hEventFroggeMovement[1] = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoSapo") + trash1);

	mtxHandles.mutexMapaChange = CreateMutex(NULL, FALSE, THREAD_ROADS_MUTEX);

	evtHandles.hEventFrogMovement = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoSapoMOVEMENT"));
	mtxHandles.mutexFrogMovement = CreateMutex(NULL, FALSE, TEXT("mutexsaposmovimento"));

	evtHandles.hCountDownEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoPipeRead"));


	//Verificar handles criados

	HANDLE tHCheckOpearators = CreateThread(
		NULL,
		0,
		CheckOperators,
		&evtHandles,
		0,
		NULL);
	if (tHCheckOpearators == NULL)
	{
		_tprintf(TEXT("[DEBUG] Thread CheckOperadores\n"));
		return 1;
	}

	//Gerar mapa e dados de jogo
	data.nivel = 1;
	data.numCars = 0;
	data.num_frogs = 0;
	data.time = 80;

	//TODO

	resetMapCars(data.map,data.numRoads,&data.car_pos,&data.numCars, mtxHandles.mutexMapaChange);

	//Preencher direções das estradas
	for(int i=0;i<data.numRoads;i++)
		data.directions[i] = (rand() % 2);

	

	pGameData pMemoriaPartilhada = InitSharedMemoryMap();

	SharedMemoryMap(pMemoriaPartilhada, &data);

	// matriz de handles das threads
	HANDLE RoadThreads[MAX_ROADS_THREADS];

	TRoads RoadsData[MAX_ROADS_THREADS];

	//gerar thread por estrada para tratar do movimento do carro na estrada especifica
	for (int i = 0; i < data.numRoads; i++)
	{
		RoadsData[i].sharedMap = InitSharedMemoryMapThreadRoads();

		//temporario acho
		RoadsData[i].numClientes = &data.num_frogs;
		RoadsData[i].frog_pos = &data.frog_pos;
		RoadsData[i].numRoads = data.numRoads;
		RoadsData[i].Map = &data.map;
		RoadsData[i].car_pos = &data.car_pos;
		RoadsData[i].numCars = &data.numCars;
		evtHandles.hEventRoads[i] = CreateEvent(NULL, TRUE, FALSE, THREAD_ROADS_EVENT + i);
		RoadsData[i].id = i + SKIP_BEGINING; 
		RoadsData[i].speed = data.carSpeed;
		RoadsData[i].terminar = &terminar;
		RoadsData[i].direction = data.directions[i];
		RoadsData[i].mtxHandles = mtxHandles;
		RoadsData[i].evtHandles = evtHandles;
		RoadThreads[i] = CreateThread(
			NULL,
			0,       
			ThreadRoads, 
			&RoadsData[i],    
			0, 
			NULL);
		if (RoadThreads[i] == NULL) {
			_tprintf(TEXT("[DEBUG] Thread estrada %d erro\n"), i);
			return 1;
		}
	}

	//thread para tratar da atualizacao do sapo no mapa
	TdadosUpdateSapoMapa TfroggeData;

	TfroggeData.mutexRoads = mtxHandles.mutexMapaChange;
	TfroggeData.frog_pos = &data.frog_pos;
	TfroggeData.Map =&data.map;
	TfroggeData.numFrogs = &data.num_frogs;
	TfroggeData.evtHandles = evtHandles;
	TfroggeData.mtxHandles = mtxHandles;

	HANDLE FroggeThread = CreateThread(
		NULL,
		0,
		ThreadSapos,
		&TfroggeData,
		0,
		NULL);
		if (FroggeThread == NULL) {
			_tprintf(TEXT("[DEBUG] Thread sapo erro\n"));
			return 1;
		}

	//BufferCircular e a thread

	TDados dataThread;

	dataThread.BufferCircular = InitSharedMemoryBufferCircular();

	//dataThread.id = dataThread.BufferCircular->nConsumidores++;
	dataThread.RoadsDirection = &RoadsData;
	dataThread.Map = &data.map;
	dataThread.threadsHandles = &RoadThreads;
	dataThread.numRoads = data.numRoads;
	dataThread.hMutexInsertRoad = mtxHandles.mutexMapaChange;
	dataThread.terminar = &terminar;

	HANDLE hThreads = CreateThread(
		NULL,    
		0,       
		ThreadBufferCircular, 
		&dataThread,    
		0,
		NULL);
	if (hThreads == NULL)
	{
		_tprintf(TEXT("[ERRO] Thread BufferCircular\n"));
		return 1;
	}


	//criar thread de ler e escrever em cada programa

	//Create pipe no main do servidor e connect named pipe, com as duas threads

	//Main cliente tem create file, com criação das threads
	
	//Named pip com cliente
	DWORD n;
	HANDLE hPipe;
	HANDLE threadWPipe, threadRPipe0, threadRPipe1;
	TCHAR bufPipe[256];
	TdadosPipeSend dadosPipeSend;
	TdadosPipeSendReceive dadosPipeReceive1;
	TdadosPipeSendReceive dadosPipeReceive2;

	dadosPipeSend.evtHandles = evtHandles;
	dadosPipeSend.mtxHandles = mtxHandles;
	dadosPipeSend.numClientes = 0;
	dadosPipeSend.terminar = 0;
	dadosPipeSend.mapToShare = &data.map;
	dadosPipeSend.structToSend.numRoads = data.numRoads;
	dadosPipeSend.pGamemode = &data.gamemode;
	dadosPipeSend.frogPos = &data.frog_pos;
	dadosPipeSend.structToGetDirection = &RoadsData;
	dadosPipeSend.time = &data.time;

	dadosPipeReceive1.evtHandles = evtHandles;
	dadosPipeReceive1.mtxHandles = mtxHandles;
	dadosPipeReceive1.numClientes = 0;
	dadosPipeReceive1.terminar = 0;
	dadosPipeReceive1.mapToShare = &data.map;
	dadosPipeReceive1.structToSend.numRoads = data.numRoads;
	dadosPipeReceive1.pGamemode = &data.gamemode;
	dadosPipeReceive1.frogPos = &data.frog_pos;
	dadosPipeReceive1.structToGetDirection = &RoadsData;
	dadosPipeReceive1.time = &data.time;
	dadosPipeReceive1.gameToShare = pMemoriaPartilhada;
	dadosPipeReceive1.realGame = &data;

	dadosPipeReceive2.evtHandles = evtHandles;
	dadosPipeReceive2.mtxHandles = mtxHandles;
	dadosPipeReceive2.numClientes = 0;
	dadosPipeReceive2.terminar = 0;
	dadosPipeReceive2.mapToShare = &data.map;
	dadosPipeReceive2.structToSend.numRoads = data.numRoads;
	dadosPipeReceive2.pGamemode = &data.gamemode;
	dadosPipeReceive2.frogPos = &data.frog_pos;
	dadosPipeReceive2.time = &data.time;
	dadosPipeReceive2.structToGetDirection = &RoadsData;
	dadosPipeReceive2.gameToShare = pMemoriaPartilhada;
	dadosPipeReceive2.realGame = &data;
	mtxHandles.mutexPipe = CreateMutex(NULL, FALSE, NULL);

	if (dadosPipeSend.mtxHandles.mutexPipe == NULL) {
		_tprintf(TEXT("[Erro] ao criar mutex!\n"));
		return -1;
	}

	PipeInstance pipes[2];
	HANDLE hEvents[2];
	HANDLE htemp, hEventTemp;

	for (int index = 0; index < 2; index++) {
		htemp =  CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 2, sizeof(GameData), sizeof(GameData), 1000, NULL);
		if (htemp == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}

		hEventTemp = CreateEvent(
			NULL,
			TRUE,
			FALSE,
			NULL);

		if (hEventTemp == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
			return 0;
		}

		pipes[index].hPipe = htemp;
		pipes[index].ready = FALSE;

		ZeroMemory(&pipes[index].oOverlap, sizeof(pipes[index].oOverlap));

		pipes[index].oOverlap.hEvent = hEventTemp;

		ConnectNamedPipe(htemp, &pipes[index].oOverlap);
			
		dadosPipeSend.hPipe[index].hPipe = pipes[index].hPipe;
		dadosPipeSend.hPipe[index].oOverlap = pipes[index].oOverlap;
		dadosPipeSend.hPipe[index].ready = &pipes[index].ready;
		hEvents[index] = hEventTemp;
	}

	threadWPipe = CreateThread(NULL, 0, send, &dadosPipeSend, 0, NULL);
	if (threadWPipe == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread send!\n"));
		return -1;
	}

	int waiting=1,i;
	data.gamemode = 1;
	DWORD offset, nBytes;
	
		_tprintf(TEXT("[SERVIDOR] Esperar ligação de um leitor... (ConnectNamedPipe)\n"));

		while (1) {
			offset = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
			i = offset - WAIT_OBJECT_0;
			_tprintf(TEXT("[SERVIDOR] Connection\n"), i);
			if (i >= 0 && i < 2) {

				_tprintf(TEXT("[ESCRITOR] Leitor %d chegou\n"), i);
				if (GetOverlappedResult(pipes[i].hPipe, &pipes[i].oOverlap, &nBytes, FALSE)) {
					// se entrarmos aqui significa que a funcao correu tudo bem
					// fazemos reset do evento porque queremos que o WaitForMultipleObject desbloqueio com base noutro evento e nao neste
					ResetEvent(pipes[i].oOverlap.hEvent);
					//vamos esperar que o mutex esteja livre
					dadosPipeSend.hPipe[i].ready = TRUE; // dizemos que esta instancia do named pipe está ativa para receber info do servidor
					
					//Gerar um sapo
					WaitForSingleObject(dadosPipeSend.mtxHandles.mutexPipe, INFINITE);
					int sapRowRandom = data.numRoads + 2; //+3 para ficar na penultima estrada.
					//_tprintf(TEXT("[DEBUG] Cliente Identificador %d\n"), dadosPipe.clienteIdentificador);
					//Depende de modo de jogo, a ser feito depois da conexão TODO

					int sapColRandom = (rand() % (MAX_COLS - 2)) + 1;
					do {
						sapColRandom = (rand() % (MAX_COLS - 2)) + 1;
					} while (data.map[sapRowRandom][sapColRandom] == 'S');

					data.frog_pos[dadosPipeSend.numClientes].col = sapColRandom;
					data.frog_pos[dadosPipeSend.numClientes].row = sapRowRandom;
					data.frog_pos[dadosPipeSend.numClientes].level = 1;
					data.frog_pos[dadosPipeSend.numClientes].score = 0;
					//data.frog_pos[dadosPipeSend.numClientes].time = 30;
					if(i==0)
					{
						dadosPipeReceive1.clienteIdentificador = dadosPipeSend.numClientes;
						dadosPipeReceive1.hPipe.hPipe = pipes[i].hPipe;
						dadosPipeReceive1.hPipe.oOverlap = pipes[i].oOverlap;
						dadosPipeReceive1.numClientes = &dadosPipeSend.numClientes;
						threadRPipe0 = CreateThread(NULL, 0, receive, &dadosPipeReceive1, 0, NULL);
						if (threadWPipe == NULL) {
							_tprintf(TEXT("[Erro] ao criar thread send!\n"));
							return -1;
						}
					}
					else if(i==1)
					{
						dadosPipeReceive2.clienteIdentificador = dadosPipeSend.numClientes;
						dadosPipeReceive2.hPipe.hPipe = pipes[i].hPipe;
						dadosPipeReceive2.hPipe.oOverlap = pipes[i].oOverlap;
						dadosPipeReceive2.numClientes = &dadosPipeSend.numClientes;
						threadRPipe1 = CreateThread(NULL, 0, receive, &dadosPipeReceive2, 0, NULL);
						if (threadWPipe == NULL) {
							_tprintf(TEXT("[Erro] ao criar thread send!\n"));
							return -1;
						}
					}
					dadosPipeSend.numClientes = dadosPipeSend.numClientes + 1;
					data.num_frogs = dadosPipeSend.numClientes;
					data.map[sapRowRandom][sapColRandom] = FROGGE_ELEMENT;
					ReleaseMutex(dadosPipeSend.mtxHandles.mutexPipe);
				}
			}
		}

		//mandar o handle de apenas um dos clientes para cada thread... basicamente cada receive apenas tem o handle do seu cliente...
		//e o send vai ter q ter os dois handles
		//threadRPipe = CreateThread(NULL, 0, receive, &dadosPipe, 0, NULL);
		//if (threadRPipe == NULL) {
		//	_tprintf(TEXT("[Erro] ao criar thread receive!\n"));
		//	return -1;
		//}
		//WAIT RESPOSTA (SINGLE OU MULTIPLE)
		//if resposta é single, waiting fica a 0

		HANDLE clientGamemode = CreateEvent(NULL, TRUE, FALSE, TEXT("clientgamemode"));
		
		WaitForSingleObject(clientGamemode, INFINITE);
		//Gerar sapos (Primeira meta....) // alterar para ser com opcao do menu

	//} while(data.gamemode == 1);

	while (terminar == 0)
	{

	}
		
	//fechar tudo
	_tprintf(TEXT("[INFO] SERVIDOR VAI TERMINAR\n"));
	//adicionar à cena
	HANDLE ending_event = CreateEvent(NULL, TRUE, FALSE, ENDING_EVENT);
	SetEvent(ending_event);

	/*UnmapViewOfFile(HMapFile);
	UnmapViewOfFile(HMapFileBuffer);*/
	// Close thread handles
	for (int i = 0; i < data.numRoads; i++) {
		CloseHandle(RoadThreads[i]);
	}
	CloseHandle(hThreads);
	CloseHandle(tHCheckOpearators);
	
	return 0;
}
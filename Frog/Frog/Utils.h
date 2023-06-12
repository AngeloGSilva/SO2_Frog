﻿#pragma once
//geral
#define MAX_ROADS_THREADS 50
#define TAM 100
#define MAX_CARS 64
#define MAX_FROGS 10
#define MAX_ROWS 8
#define MAX_COLS 20

//nomes para mutexs/eventos/file mapping/semaphores
#define SEMAPHORE_UNIQUE_SERVER TEXT("SEM_UNIQUE_SERVE")

#define FILE_MAPPING_GAME_DATA TEXT("FILE_MAPPING_GAME_DATA")

#define SHARED_MEMORY_EVENT TEXT("SHARED_MEMORY_EVENT")

#define SHARED_MEMORY_MUTEX TEXT("SHARED_MEMORY_MUTEX")

#define KEYBOARD_EVENT TEXT("KEYBOARD_EVENT_OPERATOR")

#define BUFFER_CIRCULAR_MUTEX_ESCRITOR TEXT("BUFFER_CIRCULAR_MUTEX_ESCRITOR")

#define BUFFER_CIRCULAR_MUTEX_LEITOR TEXT("BUFFER_CIRCULAR_MUTEX_LEITOR")

#define BUFFER_CIRCULAR_SEMAPHORE_ESCRITOR TEXT("BUFFER_CIRCULAR_SEM_ESCRITA")

#define BUFFER_CIRCULAR_SEMAPHORE_LEITORE TEXT("BUFFER_CIRCULAR_SEM_LEITURA")

#define FILE_MAPPING_BUFFER_CIRCULAR TEXT("BUFFER_CIRCULAR")

#define FILE_MAPPING_THREAD_ROADS TEXT("SO2_MAP")

#define THREAD_ROADS_MUTEX TEXT("MUTEX_ROADS")

#define THREAD_ROADS_EVENT TEXT("EVENT_ROADS")

#define INITIAL_EVENT TEXT("INITIAL_EVENT")

#define ENDING_EVENT TEXT("SERVER_ENDING")

#define GAMEDATA_EVENT TEXT("PONTUACAO")

//Tchar elements para o mapa
#define FROGGE_ELEMENT 83
#define BLOCK_ELEMENT 124
#define ROAD_ELEMENT 61
#define CAR_ELEMENT 72
#define OBSTACLE_ELEMENT 35
#define BEGIN_END_ELEMENT 42
#define SKIP_BEGINING_END 4
#define SKIP_BEGINING 2

//para direcao das roads
#define ROAD_RIGHT 0
#define ROAD_LEFT 1

//cliente input
#define KEY_UP 38
#define KEY_LEFT 37
#define KEY_RIGHT 39
#define KEY_DOWN 40

//named pipe
#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")
#define N 10

//Orientação sapos Grafico
#define POSUP 1001
#define	POSDOWN 1001
#define POSLEFT 1001
#define POSRIGHT 1001

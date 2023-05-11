#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <io.h>
#include "Registry.h"
#include "Utils.h"
#include "Struct.h"

GameData RegistryKeyValue(DWORD valueRoad,DWORD valueSpeed) {
	GameData temp;
	HKEY registryKey;
	DWORD keyResult; //o que aconteceu com a chave
	//DWORD valueRoad;
	//DWORD valueSpeed;
	_tprintf(TEXT("Valores recebidos por parametro -> Roads %lu e Speed %lu:\n"), valueRoad, valueSpeed);

	//se valor for a baixo do min imo por ao minimo, maximo ao maximo
	if (valueRoad > 8)
	{
		_tprintf(TEXT("Valor Roads [%lu] acima do máximo,a definir para 8.\n"), valueRoad);
		valueRoad = MAX_ROWS;
	}

	if (valueSpeed > 5000 || valueSpeed < 1000)
	{
		_tprintf(TEXT("Valor Speed [%lu] acima/abaixo do máximo/minimo,a definir para 1000.\n"), valueSpeed);
		valueSpeed = 1000; //random
	}

	/*Criar ou abrir a chave dir no Registry*/
	if (RegCreateKeyEx(
		HKEY_CURRENT_USER,
		KEY_PATH,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&registryKey,
		&keyResult
	) != ERROR_SUCCESS) {
		_tprintf(TEXT("[ERRO] Chave nao foi nem criada nem aberta!\n"));
		return;
	}


	if (keyResult == REG_CREATED_NEW_KEY)
		_tprintf(TEXT("A chave foi criada: %s\n"), KEY_PATH);
	else
		_tprintf(TEXT("A chave foi aberta: %s\n"), KEY_PATH);

	DWORD sizeRoad = sizeof(valueRoad);
	DWORD sizeSpeed = sizeof(sizeSpeed);
	
		_tprintf(TEXT("A editar o parametro Roads...\n"));
		if (RegSetValueEx(
			registryKey,						//HKEY   hKey
			KEY_ROADS_VALUE,					//LPCSTR     lpValueName,
			0,							//DWORD      Reserved,
			REG_DWORD,						//DWORD      dwType -> tipo do atributo,
			(LPBYTE)&valueRoad,					//const BYTE * lpData -> que valor queremos lá por
			sizeRoad			//DWORD      cbData -> tamanho do valor + 1 para terminar a string
		) != ERROR_SUCCESS)
			_tprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo Roads %s!\n"), valueRoad);
		else {
			_tprintf(stderr, TEXT("Foi adicionado o atributo %s!\n"), valueRoad);
		}

	
		_tprintf(TEXT("A editar o parametro Speed...\n"));
		if (RegSetValueEx(
			registryKey,						//HKEY   hKey
			KEY_SPEED_VALUE,			//LPCSTR     lpValueName,
			0,							//DWORD      Reserved,
			REG_DWORD,						//DWORD      dwType -> tipo do atributo,
			(LPBYTE)&valueSpeed,					//const BYTE * lpData -> que valor queremos lá por
			sizeSpeed			//DWORD      cbData -> tamanho do valor + 1 para terminar a string
		) != ERROR_SUCCESS) {
			_tprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo %s!\n"), valueSpeed);
		}
		else {
			_tprintf(stderr, TEXT("Foi adicionado o atributo Speed %s!\n"), valueSpeed);
		}


	temp.carSpeed = valueSpeed;
	_tprintf(TEXT("Speed: %lu!\n"), temp.carSpeed);
	temp.numRoads = valueRoad;
	_tprintf(TEXT("roads: %lu!\n"), temp.numRoads);
	RegCloseKey(registryKey);
	return temp;
}

GameData RegistryGetValues() {
	GameData temp;
	HKEY registryKey;
	DWORD keyResult; //o que aconteceu com a chave
	DWORD valueRoad;
	DWORD valueSpeed;


	/*Criar ou abrir a chave dir no Registry*/
	if (RegCreateKeyEx(
		HKEY_CURRENT_USER,
		KEY_PATH,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&registryKey,
		&keyResult
	) != ERROR_SUCCESS) {
		_tprintf(TEXT("[ERRO] Chave nao foi nem criada nem aberta!\n"));
		return;
	}


	if (keyResult == REG_CREATED_NEW_KEY)
		_tprintf(TEXT("A chave foi criada: %s\n"), KEY_PATH);
	else
		_tprintf(TEXT("A chave foi aberta: %s\n"), KEY_PATH);

	DWORD sizeRoad = sizeof(valueRoad);
	if (RegQueryValueEx(
		registryKey,
		KEY_ROADS_VALUE,
		0,							//DWORD  Reserved,
		NULL,
		(LPBYTE)&valueRoad,
		&sizeRoad
	) != ERROR_SUCCESS) {
		do {
			_tprintf(TEXT("Atributo Roads não definido, insere valor[1-8]:\n"));
			_tscanf_s(TEXT("%lu"), &valueRoad);
		} while (valueRoad < 0 || valueRoad>9);

		if (RegSetValueEx(
			registryKey,						//HKEY   hKey
			KEY_ROADS_VALUE,					//LPCSTR     lpValueName,
			0,							//DWORD      Reserved,
			REG_DWORD,						//DWORD      dwType -> tipo do atributo,
			(LPBYTE)&valueRoad,					//const BYTE * lpData -> que valor queremos lá por
			sizeof(valueRoad)			//DWORD      cbData -> tamanho do valor + 1 para terminar a string
		) != ERROR_SUCCESS)
			_tprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo %s!\n"), valueRoad);
		else {
			_tprintf(stderr, TEXT("Foi adicionado o atributo %s!\n"), valueRoad);
		}
	}
	else
		_tprintf(TEXT("Roads esta definido no registry!\n"));

	DWORD sizeSpeed = sizeof(sizeSpeed);
	if (RegQueryValueEx(
		registryKey,
		KEY_SPEED_VALUE,
		0,							//DWORD  Reserved,
		NULL,
		(LPBYTE)&valueSpeed,
		&sizeSpeed
	) != ERROR_SUCCESS) {
		do {
			_tprintf(TEXT("Atributo Speed não definido, insere valor[1000-5000]:\n"));
			_tscanf_s(TEXT("%lu"), &valueSpeed);
		} while (valueSpeed < 1000 || valueSpeed>5000);

		if (RegSetValueEx(
			registryKey,						//HKEY   hKey
			KEY_SPEED_VALUE,			//LPCSTR     lpValueName,
			0,							//DWORD      Reserved,
			REG_DWORD,						//DWORD      dwType -> tipo do atributo,
			(LPBYTE)&valueSpeed,					//const BYTE * lpData -> que valor queremos lá por
			sizeof(valueSpeed)			//DWORD      cbData -> tamanho do valor + 1 para terminar a string
		) != ERROR_SUCCESS) {
			_tprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo %s!\n"), valueSpeed);
		}
		else {
			_tprintf(stderr, TEXT("Foi adicionado o atributo %s!\n"), valueSpeed);
		}
	}
	else
		_tprintf(TEXT("Speed esta definido no registry!\n"));


	temp.numRoads = valueRoad;
	temp.carSpeed = valueSpeed;
	RegCloseKey(registryKey);
	return temp;
}

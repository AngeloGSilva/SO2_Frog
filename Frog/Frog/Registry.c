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

GameData RegistryKeyValue() {
	GameData temp;
	HKEY registryKey;
	DWORD keyResult; //o que aconteceu com a chave
	DWORD valueRoad;
	DWORD valueSpeed;

	_tprintf(TEXT("Numero de Estradas:"));
	_tscanf_s(TEXT("%lu"), &valueRoad);

	_tprintf(TEXT("Velocidade dos veiculos:"));
	_tscanf_s(TEXT("%lu"), &valueSpeed);

	_tprintf(TEXT("Valores angariados Estradas %lu e speed %lu:\n"), valueRoad, valueSpeed);


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
		_tprintf(TEXT("Roads nao definido no registry vai ser defenido a defenir com valores fornecidos!\n"));
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
	}else
		_tprintf(TEXT("Roads esta definido no registry!\n"));

	temp.num_cars = valueRoad;

	DWORD sizeSpeed = sizeof(sizeSpeed);
	if (RegQueryValueEx(
		registryKey,
		KEY_SPEED_VALUE,
		0,							//DWORD  Reserved,
		NULL,
		(LPBYTE)&valueSpeed,
		&sizeSpeed
	) != ERROR_SUCCESS) {
		_tprintf(TEXT("Speed nao definido no registry vai ser defenido a defenir com valores fornecidos!\n"));
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
	}else
		_tprintf(TEXT("Speed esta definido no registry!\n"));

	temp.carSpeed = valueSpeed;
	_tprintf(TEXT("Speed: %d!\n"), temp.carSpeed);
	temp.num_cars = valueRoad;
	_tprintf(TEXT("roads: %d!\n"), temp.num_cars);
	RegCloseKey(registryKey);
	return temp;
}

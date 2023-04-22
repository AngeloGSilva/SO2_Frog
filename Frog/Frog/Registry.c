#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

int RegistryRoads() {
	HKEY key;
	DWORD result; //o que aconteceu com a chave
	TCHAR par_valor[100] = TEXT("10");
	/*Criar ou abrir a chave dir no Registry*/
	if (RegCreateKeyEx(
		HKEY_CURRENT_USER,
		_T("SO2_Project"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		&result
	) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave nao foi nem criada nem aberta! ERRO!\n"));
		return -1;
	}

	if (result == REG_CREATED_NEW_KEY)
		_tprintf(TEXT("A chave foi criada: %s\n"), _T("SO2_Project"));
	else
		_tprintf(TEXT("A chave foi aberta: %s\n"), _T("SO2_Project"));

	DWORD tamanho = sizeof(par_valor);
	if (RegQueryValueEx(
		key,
		_T("Roads"),
		0,							//DWORD      Reserved,
		NULL,
		(LPBYTE)&par_valor,
		&tamanho
	) != ERROR_SUCCESS) {
		_tprintf(TEXT("Roads nao definido no registry vai ser defenido a defenir com valores fornecidos!\n"));
		if (RegSetValueEx(
			key,						//HKEY   hKey
			_T("Roads"),					//LPCSTR     lpValueName,
			0,							//DWORD      Reserved,
			REG_SZ,						//DWORD      dwType -> tipo do atributo,
			(LPBYTE)&par_valor,					//const BYTE * lpData -> que valor queremos lá por
			sizeof(TCHAR) * (_tcslen(par_valor) + 1)			//DWORD      cbData -> tamanho do valor + 1 para terminar a string
		) != ERROR_SUCCESS)
			_tprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo %s!\n"), par_valor);
		else
			_tprintf(stderr, TEXT("Foi adicionado o atributo %s!\n"), par_valor);
	}
	else
		_tprintf(TEXT("Roads esta definido no registry!\n"));
}

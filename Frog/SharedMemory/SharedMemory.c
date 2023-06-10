#include <windows.h>
#include "SharedMemory.h"
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"

HANDLE createMemoryMapping(DWORD size,LPCSTR name){
	return CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
}

HANDLE openMemoryMapping(DWORD dwDesiredAccess, LPCSTR lpName) {
	return OpenFileMapping(dwDesiredAccess, FALSE ,lpName);
}

void copyMemoryOperation(PVOID destiny, VOID* source, SIZE_T Length) {
	CopyMemory(destiny,source,Length);
}

void clearMemoryOperation(PVOID destination, SIZE_T Length) {
	ZeroMemory(destination, Length);
}
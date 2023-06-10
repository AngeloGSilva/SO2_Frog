#pragma once

#include <windows.h>

#ifdef SO2F3DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

DLL_IMP_API HANDLE createMemoryMapping(DWORD size, LPCSTR name);

DLL_IMP_API HANDLE openMemoryMapping(DWORD dwDesiredAccess, LPCSTR lpName);

DLL_IMP_API void copyMemoryOperation(PVOID destiny, VOID* source, SIZE_T Length);

DLL_IMP_API void clearMemoryOperation(PVOID destination, SIZE_T Length);
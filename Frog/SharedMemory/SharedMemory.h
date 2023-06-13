#pragma once

#include <windows.h>
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"

#ifdef SO2F3DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

DLL_IMP_API HANDLE createMemoryMapping(DWORD size, LPCSTR name);

DLL_IMP_API HANDLE openMemoryMapping(DWORD dwDesiredAccess, LPCSTR lpName);

DLL_IMP_API void copyMemoryOperation(PVOID destiny, VOID* source, SIZE_T Length);

DLL_IMP_API void clearMemoryOperation(PVOID destination, SIZE_T Length);

DLL_IMP_API pGameData InitSharedMemoryMap();
DLL_IMP_API void SharedMemoryMap(pGameData pBuf, pGameData data);

DLL_IMP_API pBuffer InitSharedMemoryBufferCircular();
DLL_IMP_API EspacoBuffer ReadSharedMemoryServer(pBuffer BufferCircular);
DLL_IMP_API BOOL ReadSharedMemoryOperador(pBuffer BufferCircular);
#pragma once

#include <windows.h>
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"

#ifdef SO2F3DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

DLL_IMP_API TCHAR* InitSharedMemoryMapThreadRoads();
DLL_IMP_API void SharedMemoryMapThreadRoads(pTRoads data);

DLL_IMP_API void SharedMemoryMapThreadRoadsOperador(pTRoads data, TCHAR* temp);

DLL_IMP_API pGameData InitSharedMemoryMap();
DLL_IMP_API void SharedMemoryMap(pGameData pBuf, pGameData data);

DLL_IMP_API pBuffer InitSharedMemoryBufferCircular();
DLL_IMP_API EspacoBuffer ReadSharedMemoryServer(pBuffer BufferCircular);
DLL_IMP_API BOOL ReadSharedMemoryOperador(pBuffer BufferCircular);
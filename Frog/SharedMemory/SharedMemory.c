#include <windows.h>
#include "SharedMemory.h"
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"

HANDLE createMemoryMapping(){
	return CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), FILE_MAPPING_GAME_DATA);
}

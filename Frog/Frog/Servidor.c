#define MAX_CARS 10
#define MAX_FROGS 10
#define MAX_ROWS 20
#define MAX_COLS 40


#define GAME_NAME "frogger_shared_memory"
#define S_SIZE sizeof(GameData)

struct GameData {
	int num_cars;
	int num_frogs;
	int car_pos[MAX_CARS][2]; //2 seria para representar o x e o y
	int frog_pos[MAX_FROGS][2]; //2 seria para representar o x e o y
	TCHAR map[MAX_ROWS][MAX_CLOS]
}

int main() {
	HANDLE handle;
	GameData* game_data;

	handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, S_SIZE, GAME_NAME);

	return 0;

}
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "../Frog/Utils.h"
#include "../Frog/Struct.h"
#include "resource.h"

/* ===================================================== */
/* Programa base (esqueleto) para aplica��es Windows     */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo de branco
// Modelo para programas Windows:
//  Composto por 2 fun��es: 
//	WinMain()     = Ponto de entrada dos programas windows
//			1) Define, cria e mostra a janela
//			2) Loop de recep��o de mensagens provenientes do Windows
//     TrataEventos()= Processamentos da janela (pode ter outro nome)
//			1) � chamada pelo Windows (callback) 
//			2) Executa c�digo em fun��o da mensagem recebida

// Fun��o de callback que ser� chamada pelo Windows sempre que acontece alguma coisa
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK TrataEventosInicial(HWND, UINT, WPARAM, LPARAM);

// Nome da classe da janela (para programas de uma s� janela, normalmente este nome � 
// igual ao do pr�prio programa) "szprogName" � usado mais abaixo na defini��o das
// propriedades do objecto janela
TCHAR szProgName[] = TEXT("Base");

HBITMAP hBmp; // handle para o bitmap
HDC bmpDC; // hdc do bitmap
BITMAP bmp; // informa��o sobre o bitmap

HWND hWndGlobal; // handle para a janela
HANDLE hMutex, hMutexDraw;

HDC memDC = NULL; // copia do device context que esta em memoria, tem de ser inicializado a null
HBITMAP hBitmapDB; // copia as filleracteristicas da janela original para a janela que vai estar em memoria
pPipeSendToClient AllGameData ;
HANDLE hPipe;

TCHAR username[100];
BOOL GameOption;
int currentFrogpos = POSUP; // 1 up 2 left 3 right 5 down
BOOL GameEnd = FALSE;
FrogInitialdata froginitialdata;
int FrogX,FrogY;

HANDLE hcommand;

typedef struct {
	HBITMAP hBmp; // handle para o bitmap
	HDC bmpDC; // hdc do bitmap
	BITMAP bmp; // informa��o sobre o bitmap
	HWND hWndGlobal; // handle para a janela
	HANDLE hMutex, hMutexDraw;
	HDC memDC; // copia do device context que esta em memoria, tem de ser inicializado a null
	HBITMAP hBitmapDB; // copia as filleracteristicas da janela original para a janela que vai estar em memoria
	pPipeSendToClient AllGameData;
	HANDLE hPipe;
	TCHAR username[100];
	BOOL GameOption;
	int currentFrogpos; // 1 up 2 left 3 right 5 down
	BOOL GameEnd;
	FrogInitialdata froginitialdata;
	int FrogX, FrogY;
	HANDLE hcommand;
}Win, *pWin;

//pWin data = (pWin)GetWindowLongPtr(hWnd, GWLP_USERDATA);
//SetWindowLongPtr(hWnd, GWLP_USERDATA, (data));



DWORD WINAPI mapPipe(LPVOID lpParam)
{
	//esta parte tem de ser numa thread
	int terminar = 0; // para acabar com tudo
	TCHAR buf[256];
	int i = 0;
	BOOL ret;
	//DWORD n;
	HANDLE heventmapread;

	heventmapread = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoPipeRead"));

	while (1) {
		WaitForSingleObject(heventmapread, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		ret = ReadFile(hPipe, AllGameData, sizeof(PipeSendToClient), 0, NULL);
		InvalidateRect(hWndGlobal, NULL, FALSE);
		ReleaseMutex(hMutex);
		hcommand = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoSapo") + AllGameData->identifier);
	}
	//wait do evento de atualizacao do mapa
	//e mutex para esperar pela vez de desenhar

	//receber o mapa e atualizar a variavel que possui o mapa

	//invalidar a janela para o paint correr outra vez

	return 0;
}

DWORD WINAPI CountDownTimer(LPVOID lpParam) {
	HANDLE timerevent = CreateEvent(NULL, TRUE, FALSE, TEXT("countdownevent"));
	while (1) {
		WaitForSingleObject(timerevent,INFINITE);
		GameEnd = TRUE;
		InvalidateRect(hWndGlobal, NULL, FALSE);
	}
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;		// hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg;		// MSG � uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX � uma estrutura cujos membros servem para

	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Inst�ncia da janela actualmente exibida
	
	wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endere�o da fun��o de processamento da janela
	
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);   // "hIcon" = handler do �con normal

	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION); // "hIconSm" = handler do �con pequeno

	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato)

	wcApp.lpszMenuName = MAKEINTRESOURCE(ID_MENU_PRINCIPAL);			// Classe do menu que a janela pode ter
	// (NULL = n�o tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = 0;				// Livre, para uso particular

	wcApp.hbrBackground = CreateSolidBrush(RGB(125, 125, 125));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("FROG"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPED | WS_MINIMIZEBOX,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posi��o x pixels (default=� direita da �ltima)
		CW_USEDEFAULT,		// Posi��o y pixels (default=abaixo da �ltima)
		1000,		// Largura da janela (em pixels)
		500,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira,
		// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da inst�ncia do programa actual ("hInst" �
		// passado num dos par�metros de WinMain()
		0);				// N�o h� par�metros adicionais para a janela


	//Janela de Inicio :D
	EnableWindow(hWnd,FALSE);
	HWND hDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_INICIAL), hWnd, TrataEventosInicial,0);
	
	EnableWindow(hDialog, TRUE);
	ShowWindow(hDialog, SW_SHOW);

	//// definir as posicoes inicias da imagem
	//GetClientRect(hWnd, &rect);

	//// limite direito � a largura da janela - largura da imagem
	//limDir = rect.right - bmp.bmWidth;
	//hWndGlobal = hWnd;

	// Cria mutex
	//hMutex = CreateMutex(NULL, FALSE, NULL);

	// Cria a thread de movimenta��o
	//CreateThread(NULL, 0, RefreshMap, NULL, 0, NULL);
	
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por
	// "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e.
	// normal/modal); � passado como par�metro de WinMain()
	UpdateWindow(hWnd);		// Refresfiller a janela (Windows envia � janela uma
	// mensagem para pintar, mostrar dados, (refresfiller)

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);	// Pr�-processamento da mensagem (p.e. obter c�digo
		// ASCII da tecla premida)
		DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
		// aguarda at� que a possa reenviar � fun��o de
		// tratamento da janela, CALLBACK TrataEventos (abaixo)
	}

	
	return((int)lpMsg.wParam);	// Retorna sempre o par�metro wParam da estrutura lpMsg
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	//handle para o device context
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	MINMAXINFO* mmi;

	//keyup
	
	TCHAR* message;
	DWORD n;

	static HDC bmpCar = NULL;
	static HDC bmpFrogUp = NULL;
	static HDC bmpRoad = NULL;
	static HDC bmpLimit = NULL;
	static HDC bmpBeginEnd = NULL;
	static HDC bmpObstacle = NULL;
	static HDC bmpFrogLeft = NULL;
	static HDC bmpFrogRight = NULL;
	static HDC bmpFrogDown = NULL;
	static HDC bmpCarRight = NULL;

	HBITMAP hcar = NULL;
	HBITMAP hcarRight = NULL; //NEED
	HBITMAP hroad = NULL;
	HBITMAP hfrog = NULL;
	HBITMAP hfrogLeft = NULL; //NEED
	HBITMAP hfrogRight = NULL; //NEED
	HBITMAP hfrogDown = NULL; //NEED
	HBITMAP hlimit = NULL;
	HBITMAP hbeginend = NULL;
	HBITMAP hObstacle = NULL; //NEED

	static BITMAP bmp;
	static BITMAP car;
	static BITMAP carRight;//NEED
	static BITMAP frog;
	static BITMAP frogLeft; //NEED
	static BITMAP frogRight; //NEED
	static BITMAP frogDown; //NEED
	static BITMAP road;
	static BITMAP limit;
	static BITMAP beginend;
	static BITMAP obstacle; //NEED

	int x;
	int y;




	//buffer
	static HDC memDC = NULL;

	srand((unsigned)time(NULL));

	//send info struct
	PipeFroggeInput sendInfo;

	switch (messg)
	{

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BITMAP_SET1:
			WaitForSingleObject(hMutexDraw, INFINITE);
			hcar = (HBITMAP)LoadImage(NULL, TEXT("car.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			GetObject(hcar, sizeof(car), &car);

			hcarRight = (HBITMAP)LoadImage(NULL, TEXT("carRight.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			GetObject(hcarRight, sizeof(carRight), &carRight);

			//FROG BITMAPS

			hfrog = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_SAPO));

			GetObject(hfrog, sizeof(frog), &frog);

			hfrogLeft = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_SAPO_LEFT));

			GetObject(hfrogLeft, sizeof(frogLeft), &frogLeft);

			hfrogRight = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_SAPO_RIGHT));

			GetObject(hfrogRight, sizeof(frogRight), &frogRight);

			hfrogDown = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_SAPO_DOWN));

			GetObject(hfrogDown, sizeof(frogDown), &frogDown);

			//ROAD BITMAPS
			hroad = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_ROAD));

			GetObject(hroad, sizeof(road), &road);

			//LIMIT BITMAPS
			hlimit = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_LIMIT));

			GetObject(hlimit, sizeof(limit), &limit);

			//BEGIN AND END AREA BITMAPS

			hbeginend = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_BEGINEND));

			GetObject(hbeginend, sizeof(beginend), &beginend);

			//OBSTACLE ELEMENT

			hObstacle = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_OBSTACLE));

			GetObject(hObstacle, sizeof(obstacle), &obstacle);

			hdc = GetDC(hWnd);

			bmpCar = CreateCompatibleDC(hdc);
			SelectObject(bmpCar, hcar);

			bmpFrogUp = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogUp, hfrog);

			bmpRoad = CreateCompatibleDC(hdc);
			SelectObject(bmpRoad, hroad);

			bmpLimit = CreateCompatibleDC(hdc);
			SelectObject(bmpLimit, hlimit);

			bmpBeginEnd = CreateCompatibleDC(hdc);
			SelectObject(bmpBeginEnd, hbeginend);

			bmpObstacle = CreateCompatibleDC(hdc);
			SelectObject(bmpObstacle, hObstacle);

			bmpFrogLeft = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogLeft, hfrogLeft);

			bmpFrogRight = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogRight, hfrogRight);

			bmpFrogDown = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogDown, hfrogDown);

			bmpCarRight = CreateCompatibleDC(hdc);
			SelectObject(bmpCarRight, hcarRight);

			ReleaseDC(hWnd, hdc);

			ReleaseMutex(hMutexDraw);
			MessageBox(hWnd, TEXT("Choosen bitmap set 1!"), TEXT("Bitmap 1"), MB_OK | MB_ICONINFORMATION);
			break;

		case ID_BITMAP_SET2:
			WaitForSingleObject(hMutexDraw,INFINITE);

			hcar = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/meteorite.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			GetObject(hcar, sizeof(car), &car);

			hcarRight = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/ufo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			GetObject(hcarRight, sizeof(carRight), &carRight);

			//FROG BITMAPS

			hfrog = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketUp.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			GetObject(hfrog, sizeof(frog), &frog);

			hfrogLeft = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketLeft.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			GetObject(hfrogLeft, sizeof(frogLeft), &frogLeft);

			hfrogRight = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketRight.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			GetObject(hfrogRight, sizeof(frogRight), &frogRight);

			hfrogDown = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketDown.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			GetObject(hfrogDown, sizeof(frogDown), &frogDown);

			//ROAD BITMAPS
			hroad = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_STARS));

			GetObject(hroad, sizeof(road), &road);

			//LIMIT BITMAPS
			hlimit = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_STARLIMIT));

			GetObject(hlimit, sizeof(limit), &limit);

			//BEGIN AND END AREA BITMAPS

			hbeginend = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_LANDING));

			GetObject(hbeginend, sizeof(beginend), &beginend);

			//OBSTACLE ELEMENT

			hObstacle = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/satellite.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			GetObject(hObstacle, sizeof(obstacle), &obstacle);
			ReleaseMutex(hMutexDraw);
			hdc = GetDC(hWnd);

			bmpCar = CreateCompatibleDC(hdc);
			SelectObject(bmpCar, hcar);

			bmpFrogUp = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogUp, hfrog);

			bmpRoad = CreateCompatibleDC(hdc);
			SelectObject(bmpRoad, hroad);

			bmpLimit = CreateCompatibleDC(hdc);
			SelectObject(bmpLimit, hlimit);

			bmpBeginEnd = CreateCompatibleDC(hdc);
			SelectObject(bmpBeginEnd, hbeginend);

			bmpObstacle = CreateCompatibleDC(hdc);
			SelectObject(bmpObstacle, hObstacle);

			bmpFrogLeft = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogLeft, hfrogLeft);

			bmpFrogRight = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogRight, hfrogRight);

			bmpFrogDown = CreateCompatibleDC(hdc);
			SelectObject(bmpFrogDown, hfrogDown);

			bmpCarRight = CreateCompatibleDC(hdc);
			SelectObject(bmpCarRight, hcarRight);

			ReleaseDC(hWnd, hdc);
			MessageBox(hWnd, TEXT("Choosen bitmap set 2!"), TEXT("Bitmap 2"), MB_ICONINFORMATION);
			break;
		case ID_CLOSE:
			PostQuitMessage(0);
			break;
		}
		
		break;
	case WM_CREATE:
		
		AllGameData = (PipeSendToClient*)malloc(sizeof(PipeSendToClient));
		AllGameData->identifier = 0;
		//_tcscpy_s(AllGameData->frog_pos[0].name, 20, TEXT('adsadsad'));
		if (!WaitNamedPipe(PIPE_NAME, 10000)) {
			PostQuitMessage(0); // Fecha se nao existir named pipe
		}
		hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hPipe == NULL) {
			//tratar
		}
		
		hMutex = CreateMutex(NULL, FALSE, TEXT("MutexClientPipe"));
		hMutexDraw = CreateMutex(NULL, FALSE, TEXT("Bitmap change draw"));

		HANDLE hThreadMapPipe = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			mapPipe, // Thread start address
			NULL,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id   // returns the thread identifier 
		if (hThreadMapPipe == NULL)
		{
			_tprintf(TEXT("[ERRO] Thread pipe Map\n"));
			return 1;
		}
		HANDLE hThreadCountDown = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			CountDownTimer, // Thread start address
			NULL,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);

		//CAR BITMAPS

		hcar = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/meteorite.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		GetObject(hcar, sizeof(car), &car);

		hcarRight = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/ufo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		GetObject(hcarRight, sizeof(carRight), &carRight);

		//FROG BITMAPS

		hfrog = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketUp.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		GetObject(hfrog, sizeof(frog), &frog);

		hfrogLeft = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketLeft.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		GetObject(hfrogLeft, sizeof(frogLeft), &frogLeft);

		hfrogRight = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketRight.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		GetObject(hfrogRight, sizeof(frogRight), &frogRight);

		hfrogDown = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/rocketDown.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		GetObject(hfrogDown, sizeof(frogDown), &frogDown);

		//ROAD BITMAPS
		hroad = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_STARS));

		GetObject(hroad, sizeof(road), &road);

		//LIMIT BITMAPS
		hlimit = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_STARLIMIT));

		GetObject(hlimit, sizeof(limit), &limit);

		//BEGIN AND END AREA BITMAPS

		hbeginend = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_LANDING));

		GetObject(hbeginend, sizeof(beginend), &beginend);

		//OBSTACLE ELEMENT

		hObstacle = (HBITMAP)LoadImage(NULL, TEXT("bitmapsV2/satellite.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		GetObject(hObstacle, sizeof(obstacle), &obstacle);
		ReleaseMutex(hMutexDraw);
		hdc = GetDC(hWnd);

		bmpCar = CreateCompatibleDC(hdc);
		SelectObject(bmpCar, hcar);

		bmpFrogUp = CreateCompatibleDC(hdc);
		SelectObject(bmpFrogUp, hfrog);

		bmpRoad = CreateCompatibleDC(hdc);
		SelectObject(bmpRoad, hroad);

		bmpLimit = CreateCompatibleDC(hdc);
		SelectObject(bmpLimit, hlimit);

		bmpBeginEnd = CreateCompatibleDC(hdc);
		SelectObject(bmpBeginEnd, hbeginend);

		bmpObstacle = CreateCompatibleDC(hdc);
		SelectObject(bmpObstacle, hObstacle);

		bmpFrogLeft = CreateCompatibleDC(hdc);
		SelectObject(bmpFrogLeft, hfrogLeft);

		bmpFrogRight = CreateCompatibleDC(hdc);
		SelectObject(bmpFrogRight, hfrogRight);

		bmpFrogDown = CreateCompatibleDC(hdc);
		SelectObject(bmpFrogDown, hfrogDown);

		bmpCarRight = CreateCompatibleDC(hdc);
		SelectObject(bmpCarRight, hcarRight);

		ReleaseDC(hWnd, hdc);

		break;

		// evento que � disparado sempre que o sistema pede um refrescamento da janela
	case WM_PAINT:
		// Inicio da pintura da janela, que substitui o GetDC
		WaitForSingleObject(hMutexDraw, INFINITE);
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);

		// Calculate the center position
		int centerX = (rect.right - rect.left) / 2;
		int centerY = (rect.bottom - rect.top) / 2;

		// se a copia estiver a NULL, significa que � a 1� vez que estamos a passar no WM_PAINT e estamos a trabalhar com a copia em memoria
		if (memDC == NULL) {
			memDC = CreateCompatibleDC(hdc);
			hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			SelectObject(memDC, hBmp);
			DeleteObject(hBmp);
			
			// Cleanup
		}
		if (GameEnd) {
			TCHAR gameoverText[100];
			TCHAR gameOverText2[100];
			FillRect(memDC, &rect, CreateSolidBrush(RGB(0, 0, 0)));
			RECT rcGameInfo;
			rcGameInfo.left = 20;
			rcGameInfo.top = 200;
			rcGameInfo.right = rect.right - 20;
			rcGameInfo.bottom = rcGameInfo.top + 40;
			DrawText(memDC, TEXT("Time is over!"), -1, &rcGameInfo, DT_CENTER);

			rcGameInfo.left = 20;
			rcGameInfo.top = 220;
			rcGameInfo.right = rect.right - 20;
			rcGameInfo.bottom = rcGameInfo.top + 40;
			wsprintf(gameoverText, TEXT("Your Score:%d"), AllGameData->frog_pos[AllGameData->identifier].score);
			DrawText(memDC, gameoverText, -1, &rcGameInfo, DT_CENTER);
			int buf = AllGameData->identifier;
			if (buf == 0)
				buf = 1;
			else
				buf = 0;
			if (AllGameData->frog_pos[buf].score >= 0) {
				wsprintf(gameOverText2, TEXT("Your Oponents Score:%d"), AllGameData->frog_pos[buf].score);
				rcGameInfo.left = 20;
				rcGameInfo.top = 240;
				rcGameInfo.right = rect.right - 20;
				rcGameInfo.bottom = rcGameInfo.top + 40;
				DrawText(memDC, gameOverText2, -1, &rcGameInfo, DT_CENTER);

			}
		}else{
		int contentWidth = MAX_COLS * 20; // Width of the content
		int contentHeight = (AllGameData->numRoads + 4) * 20; // Height of the content

		// Calculate the top-left position of the content to be centered
		int contentX = centerX - (contentWidth / 2);
		int contentY = centerY - (contentHeight / 2);

		FillRect(memDC, &rect, CreateSolidBrush(RGB(0, 0, 0)));
		// Set the brush color and style
		//Ciclo do tabuleiro de Jogo

		for (int i = 0; i < AllGameData->numRoads + 4; i++) {
			for (int j = 0; j < MAX_COLS; j++) {
				TCHAR buffer = AllGameData->map[i][j];
				int posX = contentX + (j * 20);
				int posY = contentY + (i * 20);
				if (buffer == CAR_ELEMENT)
				{
					if(AllGameData->directions[i - 2] == ROAD_LEFT) //Resolver ele apagar obstaculo quando passa por cima
						BitBlt(memDC, posX, posY, car.bmWidth, car.bmHeight, bmpCar, 0, 0, SRCCOPY);
					else
						BitBlt(memDC, posX, posY, carRight.bmWidth, carRight.bmHeight, bmpCarRight, 0, 0, SRCCOPY);
				}
				else if (buffer == ROAD_ELEMENT) {
					BitBlt(memDC, posX, posY, road.bmWidth, road.bmHeight, bmpRoad, 0, 0, SRCCOPY);
				}
				else if (buffer == BEGIN_END_ELEMENT) {
					BitBlt(memDC, posX, posY, beginend.bmWidth, beginend.bmHeight, bmpBeginEnd, 0, 0, SRCCOPY);
				}
				else if (buffer == BLOCK_ELEMENT) {
					BitBlt(memDC, posX, posY, limit.bmWidth, limit.bmHeight, bmpLimit, 0, 0, SRCCOPY);
				}
				else if (buffer == FROGGE_ELEMENT) {
					if(currentFrogpos == POSUP)
					{
						//� a posi��o do ultimo frog, nao do atual, temos de guardar o Frox e FrogY do que tiver id igual
						if(i == AllGameData->frog_pos[AllGameData->identifier].row && j == AllGameData->frog_pos[AllGameData->identifier].col)
						{
							FrogX = posX;
							FrogY = posY;
						}
						BitBlt(memDC, posX, posY, frog.bmWidth, frog.bmHeight, bmpFrogUp, 0, 0, SRCCOPY);
					}
					else if (currentFrogpos == POSLEFT)
					{
						if (i == AllGameData->frog_pos[AllGameData->identifier].row && j == AllGameData->frog_pos[AllGameData->identifier].col) {
							FrogX = posX;
							FrogY = posY;
						}
						BitBlt(memDC, posX, posY, frogLeft.bmWidth, frogLeft.bmHeight, bmpFrogLeft, 0, 0, SRCCOPY);
					}
					else if (currentFrogpos == POSRIGHT)
					{
						if (i == AllGameData->frog_pos[AllGameData->identifier].row && j == AllGameData->frog_pos[AllGameData->identifier].col) {
							FrogX = posX;
							FrogY = posY;
						}
						BitBlt(memDC, posX, posY, frogRight.bmWidth, frogRight.bmHeight, bmpFrogRight, 0, 0, SRCCOPY);
					}
					else if (currentFrogpos == POSDOWN)
					{
						if (i == AllGameData->frog_pos[AllGameData->identifier].row && j == AllGameData->frog_pos[AllGameData->identifier].col) {
							FrogX = posX;
							FrogY = posY;
						}
						BitBlt(memDC, posX, posY, frogDown.bmWidth, frogDown.bmHeight, bmpFrogDown, 0, 0, SRCCOPY);
					}
				}
				else if (buffer == OBSTACLE_ELEMENT) {
					BitBlt(memDC, posX, posY, obstacle.bmWidth, obstacle.bmHeight, bmpObstacle, 0, 0, SRCCOPY);
				}
			}
		}

		//Ciclo do nome, Pontua��o tempo e nivel
		// Falta fazera s contas certinhas
		RECT rcGameInfo;
		rcGameInfo.left = 20;
		rcGameInfo.top = 20;
		rcGameInfo.right = rect.right - 20;
		rcGameInfo.bottom = rcGameInfo.top + 40;

		char gameInfoText[200];
		//Tem de ir buscar esta info ao AllGameInfo.
		//_tcscpy_s(gameData.name, sizeof(TEXT("WORKING")), TEXT("WORKING"));
		wsprintf(gameInfoText, TEXT("Name: %s  Time: %d   Score: %d  Level: %d"), username,  AllGameData->time, AllGameData->frog_pos[AllGameData->identifier].score, AllGameData->frog_pos[AllGameData->identifier].level);
		SetTextColor(memDC, RGB(255, 255, 255));
		SetBkMode(memDC, TRANSPARENT);
		DrawText(memDC, gameInfoText, -1, &rcGameInfo, DT_CENTER);
		
		//SetPixel(memDC, FrogX + 10, FrogY + 10, RGB(255, 0, 0));
		}

		// Copy the content from the memory DC to the actual DC
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

		//BitBlt(hdc, 0, 0, rect.right, rect.bottom,memDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		ReleaseMutex(hMutexDraw);
		break;

	case WM_ERASEBKGND:
		return TRUE;

	// redimensiona e calcula novamente o centro
	//case WM_SIZE:
	//	memDC = NULL; // metemos novamente a NULL para que caso haja um resize na janela no WM_PAINT a janela em memoria � sempre atualizada com o tamanho novo
	//	break;
	case WM_RBUTTONDOWN:
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		
		FrogX = FrogX + 10;
		FrogY = FrogY + 10;

		if (x < (FrogX + 10) && x > (FrogX - 10) && y < (FrogY + 10) && y > (FrogY - 10)) {
			//Reposicionar V�lido!
			currentFrogpos = POSUP;
			sendInfo.pressInput = REPOSITION;
			//ID vai no X, a mudar
			sendInfo.x = AllGameData->identifier;
			sendInfo.y = -1;
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
		}

		break;
	case WM_LBUTTONDOWN:
		//localiza��o click
		
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);

		//separar os clicks
		int squareSize = 30;

		int direction=0;

		int dx = x - (FrogX + 10);
		int dy = y - (FrogY + 10);

		int absDx = abs(dx);
		int absDy = abs(dy);

		if (absDx > absDy) {
			if (dx > 0 && dx <= squareSize && absDy <= squareSize) {
				// Clicked to the right of the frog within the square
				currentFrogpos = POSRIGHT;
				sendInfo.pressInput = VK_RIGHT;
			}
			else if (dx < 0 && absDx <= squareSize && absDy <= squareSize) {
				// Clicked to the left of the frog within the square
				currentFrogpos = POSLEFT;
				sendInfo.pressInput = VK_LEFT;
			}
			else {
				// Clicked outside the valid range
				direction = 1;
			}
		}
		else {
			if (dy > 0 && dy <= squareSize && absDx <= squareSize) {
				// Clicked below the frog within the square
				currentFrogpos = POSDOWN;
				sendInfo.pressInput = VK_DOWN;
			}
			else if (dy < 0 && absDy <= squareSize && absDx <= squareSize) {
				// Clicked above the frog within the square
				currentFrogpos = POSUP;
				sendInfo.pressInput = VK_UP;
			}
			else {
				// Clicked outside the valid range
				direction = 1;
			}
		}

		if (direction != 1) {
			sendInfo.x = -1;
			sendInfo.y = -1;
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
		}
		
		break;
	case WM_KEYDOWN:
		
		switch (wParam)
		{
			// caso seja duplo clique
		case VK_UP:
			currentFrogpos = POSUP;
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			Sleep(100);
			break;
		case VK_DOWN:
			currentFrogpos = POSDOWN;
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			Sleep(100);

			break;
		case VK_LEFT:
			currentFrogpos = POSLEFT;
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			Sleep(100);

			break;
		case VK_RIGHT:
			currentFrogpos = POSRIGHT;
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			Sleep(100);
			break;
		}

		MSG msg;
		while (PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYUP, PM_REMOVE));
		break;

	case WM_CLOSE:
		// handle , texto da janela, titulo da janela, configura��es da MessageBox(botoes e icons)
		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirma��o"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			// o utilizador disse que queria sair da aplica��o
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:	// Destruir a janela e terminar o programa
		// "PostQuitMessage(Exit Status)"
		PostQuitMessage(0);
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// n�o � efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecess�rio por causa do return
	}
	return(0);
}

LRESULT CALLBACK TrataEventosInicial(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	BOOL result;
	TCHAR message[256];
	TCHAR messageMulti[256];

	switch (messg)
	{
	
	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
			case IDOK:
			result = GetDlgItemText(hWnd, IDC_EDIT_USERNAME, username, 16);
			wsprintf(message, TEXT("Hi %s"), username);
			if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SINGLEPLAYER), BM_GETCHECK, 0, 0) == BST_CHECKED && result>0)
			{
				GameOption = 0;
				MessageBox(hWnd, message, TEXT("Singleplayer"), MB_OK | MB_ICONINFORMATION);
			}
			else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_MULTIPLAYER), BM_GETCHECK, 0, 0) == BST_CHECKED && result > 0)
			{
				wsprintf(messageMulti, TEXT("Hi %s"), username);
				GameOption = 1;
				MessageBox(hWnd, messageMulti, TEXT("Multiplayer, Please wait"), MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				GameOption = -1;
				MessageBox(hWnd, TEXT("Please fill both name and gamemode"), TEXT("OOPS!"), MB_OK | MB_ICONINFORMATION);
				break;
			}
				EndDialog(hWnd, 0);
				//Avisar para come�ar
				froginitialdata.Gamemode = GameOption;
				_tcscpy_s(froginitialdata.username,sizeof(username), username);
				WriteFile(hPipe, &froginitialdata, sizeof(FrogInitialdata), 0, NULL);
				SetEvent(hcommand);
				ResetEvent(hcommand);
			break;
			
			case IDCANCEL:
				if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirma��o"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
					// o utilizador disse que queria sair da aplica��o
					DestroyWindow(hWnd);
				}
				break;
		}
		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirma��o"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			// o utilizador disse que queria sair da aplica��o
			DestroyWindow(hWnd);
		}
	case WM_DESTROY:	// Destruir a janela e terminar o programa
		// "PostQuitMessage(Exit Status)"
		PostQuitMessage(0);
		break;
	}

	return FALSE;
}
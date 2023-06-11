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

// ============================================================================
// FUN��O DE IN�CIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa come�a sempre a sua execu��o na fun��o WinMain()que desempenha
// o papel da fun��o main() do C em modo consola WINAPI indica o "tipo da fun��o" (WINAPI
// para todas as declaradas nos headers do Windows e CALLBACK para as fun��es de
// processamento da janela)
// Par�metros:
//   hInst: Gerado pelo Windows, � o handle (n�mero) da inst�ncia deste programa 
//   hPrevInst: Gerado pelo Windows, � sempre NULL para o NT (era usado no Windows 3.1)
//   lpCmdLine: Gerado pelo Windows, � um ponteiro para uma string terminada por 0
//              destinada a conter par�metros para o programa 
//   nCmdShow:  Par�metro que especifica o modo de exibi��o da janela (usado em  
//        	   ShowWindow()

//BITMAP
// uma vez que temos de usar estas vars tanto na main como na funcao de tratamento de eventos
// nao ha uma maneira de fugir ao uso de vars globais, dai estarem aqui
HBITMAP hBmp; // handle para o bitmap
HDC bmpDC; // hdc do bitmap
BITMAP bmp; // informa��o sobre o bitmap
int xBitmap; // posicao onde o bitmap vai ser desenhado
int yBitmap;

int limDir; // limite direito
HWND hWndGlobal; // handle para a janela
HANDLE hMutex;

HDC memDC = NULL; // copia do device context que esta em memoria, tem de ser inicializado a null
HBITMAP hBitmapDB; // copia as filleracteristicas da janela original para a janela que vai estar em memoria
pPipeSendToClient AllGameData ;
HANDLE hPipe;

TCHAR username[16];
BOOL GameOption;

DWORD WINAPI mapPipe(LPVOID lpParam)
{
	//esta parte tem de ser numa thread
	int terminar = 0; // para acabar com tudo
	TCHAR buf[256];
	int i = 0;
	BOOL ret;
	DWORD n;
	HANDLE heventmapread;

	heventmapread = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoPipeRead"));

	while (1) {
		WaitForSingleObject(heventmapread, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		ret = ReadFile(hPipe, AllGameData, sizeof(PipeSendToClient), &n, NULL);
		InvalidateRect(hWndGlobal, NULL, FALSE);
		ReleaseMutex(hMutex);
	}
	//wait do evento de atualizacao do mapa
	//e mutex para esperar pela vez de desenhar

	//receber o mapa e atualizar a variavel que possui o mapa

	//invalidar a janela para o paint correr outra vez
	Sleep(1);

	return 0;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;		// hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg;		// MSG � uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX � uma estrutura cujos membros servem para
	// definir as filleracter�sticas da classe da janela
	//mapa
	
// ============================================================================
// 1. Defini��o das filleracter�sticas da janela "wcApp"
//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Inst�ncia da janela actualmente exibida
	// ("hInst" � par�metro de WinMain e vem
		  // inicializada da�)
	wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endere�o da fun��o de processamento da janela
	// ("TrataEventos" foi declarada no in�cio e
	// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
	// modificada horizontal ou verticalmente

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);   // "hIcon" = handler do �con normal
	// "NULL" = Icon definido no Windows
	// "IDI_AP..." �cone "aplica��o"
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION); // "hIconSm" = handler do �con pequeno
	// "NULL" = Icon definido no Windows
	// "IDI_INF..." �con de informa��o
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato)
	// "NULL" = Forma definida no Windows
	// "IDC_ARROW" Aspecto "seta"
	wcApp.lpszMenuName = NULL;			// Classe do menu que a janela pode ter
	// (NULL = n�o tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = 0;				// Livre, para uso particular
	wcApp.hbrBackground = CreateSolidBrush(RGB(125, 125, 125));
	//(HBRUSH)GetStockObject(WHITE_BRUSH);
// "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por
// "GetStockObject".Neste caso o fundo ser� branco

// ============================================================================
// 2. Registar a classe "wcApp" no Windows
// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);

	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("FROG"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posi��o x pixels (default=� direita da �ltima)
		CW_USEDEFAULT,		// Posi��o y pixels (default=abaixo da �ltima)
		600,		// Largura da janela (em pixels)
		300,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira,
		// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da inst�ncia do programa actual ("hInst" �
		// passado num dos par�metros de WinMain()
		0);				// N�o h� par�metros adicionais para a janela

	HWND hDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_INICIAL), hWnd, TrataEventosInicial,0);
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


	// ============================================================================
	// 4. Mostrar a janela
	// ============================================================================
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por
	// "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e.
	// normal/modal); � passado como par�metro de WinMain()
	UpdateWindow(hWnd);		// Refresfiller a janela (Windows envia � janela uma
	// mensagem para pintar, mostrar dados, (refresfiller)

// ============================================================================
// 5. Loop de Mensagens
// ============================================================================
// O Windows envia mensagens �s janelas (programas). Estas mensagens ficam numa fila de
// espera at� que GetMessage(...) possa ler "a mensagem seguinte"
// Par�metros de "getMessage":
// 1)"&lpMsg"=Endere�o de uma estrutura do tipo MSG ("MSG lpMsg" ja foi declarada no
//   in�cio de WinMain()):
//			HWND hwnd		handler da janela a que se destina a mensagem
//			UINT message		Identificador da mensagem
//			WPARAM wParam		Par�metro, p.e. c�digo da tecla premida
//			LPARAM lParam		Par�metro, p.e. se ALT tamb�m estava premida
//			DWORD time		Hora a que a mensagem foi enviada pelo Windows
//			POINT pt		Localiza��o do mouse (x, y)
// 2)handle da window para a qual se pretendem receber mensagens (=NULL se se pretendem
//   receber as mensagens para todas as
// janelas pertencentes � thread actual)
// 3)C�digo limite inferior das mensagens que se pretendem receber
// 4)C�digo limite superior das mensagens que se pretendem receber

// NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
// 	  terminando ent�o o loop de recep��o de mensagens, e o programa

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);	// Pr�-processamento da mensagem (p.e. obter c�digo
		// ASCII da tecla premida)
		DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
		// aguarda at� que a possa reenviar � fun��o de
		// tratamento da janela, CALLBACK TrataEventos (abaixo)
	}

	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	return((int)lpMsg.wParam);	// Retorna sempre o par�metro wParam da estrutura lpMsg
}

// ============================================================================
// FUN��O DE PROCESSAMENTO DA JANELA
// Esta fun��o pode ter um nome qualquer: Apenas � neces�rio que na inicializa��o da
// estrutura "wcApp", feita no in�cio de // WinMain(), se identifique essa fun��o. Neste
// caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pr�-processadas
// no loop "while" da fun��o WinMain()
// Par�metros:
//		hWnd	O handler da janela, obtido no CreateWindow()
//		messg	Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
//		wParam	O par�metro wParam da estrutura messg (a mensagem)
//		lParam	O par�metro lParam desta mesma estrutura
//
// NOTA:Estes par�metros est�o aqui acess�veis o que simplifica o acesso aos seus valores
//
// A fun��o EndProc � sempre do tipo "switch..." com "cases" que descriminam a mensagem
// recebida e a tratar.
// Estas mensagens s�o identificadas por constantes (p.e.
// WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h
// ============================================================================



LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	//handle para o device context
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	MINMAXINFO* mmi;
	//keyup
	HANDLE hcommand = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoSapo"));
	TCHAR* message;
	DWORD n;

	static HDC bmpDC = NULL;
	static HDC bmpDC2 = NULL;
	static HDC bmpDC3 = NULL;
	static HDC bpmDC4 = NULL;
	static HDC bpmDC5 = NULL;

	HBITMAP hBmp = NULL;
	HBITMAP hcar = NULL;
	HBITMAP hroad = NULL;
	HBITMAP hfrog = NULL;
	HBITMAP hlimit = NULL;
	HBITMAP hbeginend = NULL;

	static BITMAP bmp;
	static BITMAP car;
	static BITMAP frog;
	static BITMAP road;
	static BITMAP limit;
	static BITMAP beginend;


	//buffer
	static HDC memDC = NULL;

	srand((unsigned)time(NULL));


	//send info struct
	PipeFroggeInput sendInfo;


	switch (messg)
	{

	case WM_CREATE:
		
		AllGameData = (PipeSendToClient*)malloc(sizeof(PipeSendToClient));

		if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		}
		hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hPipe == NULL) {
			//tratar
		}
		hMutex = CreateMutex(NULL, FALSE, TEXT("MutexClientPipe"));

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

		hcar = (HBITMAP)LoadImage(NULL, TEXT("car.bmp"),IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		//hcar = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_CAR_RIGHT));
		GetObject(hcar, sizeof(car), &car);

		hfrog = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_SAPO));

		GetObject(hfrog, sizeof(frog), &frog);

		hroad = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_ROAD));

		GetObject(hroad, sizeof(road), &road);

		hlimit = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_LIMIT));

		GetObject(hlimit, sizeof(limit), &limit);

		hbeginend = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_BEGINEND));

		GetObject(hbeginend, sizeof(beginend), &beginend);

		hdc = GetDC(hWnd);
		bmpDC = CreateCompatibleDC(hdc);
		SelectObject(bmpDC, hcar);

		bmpDC2 = CreateCompatibleDC(hdc);
		SelectObject(bmpDC2, hfrog);

		bmpDC3 = CreateCompatibleDC(hdc);
		SelectObject(bmpDC3, hroad);

		bpmDC4 = CreateCompatibleDC(hdc);
		SelectObject(bpmDC4, hlimit);

		bpmDC5 = CreateCompatibleDC(hdc);
		SelectObject(bpmDC5, hbeginend);

		ReleaseDC(hWnd, hdc);
		break;

		// evento que � disparado sempre que o sistema pede um refrescamento da janela
	case WM_PAINT:
		// Inicio da pintura da janela, que substitui o GetDC
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);

		// se a copia estiver a NULL, significa que � a 1� vez que estamos a passar no WM_PAINT e estamos a trabalhar com a copia em memoria
		if (memDC == NULL) {
			memDC = CreateCompatibleDC(hdc);
			hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			SelectObject(memDC, hBmp);
			DeleteObject(hBmp);
			
			// Cleanup
		}
		FillRect(memDC, &rect, CreateSolidBrush(RGB(0, 0, 255)));
		// Set the brush color and style

		for (int i = 0; i < AllGameData->numRoads + 4; i++) {
			for (int j = 0; j < MAX_COLS; j++) {
				TCHAR buffer = AllGameData->map[i][j];
				if (buffer == CAR_ELEMENT)
				{
					BitBlt(memDC, j * 20, i * 20, car.bmWidth, car.bmHeight, bmpDC, 0, 0, SRCCOPY);
				}
				else if (buffer == ROAD_ELEMENT) {
					BitBlt(memDC, j * 20, i * 20, road.bmWidth, road.bmHeight, bmpDC3, 0, 0, SRCCOPY);
				}
				else if (buffer == BEGIN_END_ELEMENT) {
					BitBlt(memDC, j * 20 ,i * 20, beginend.bmWidth, beginend.bmHeight, bpmDC5, 0, 0, SRCCOPY);
				}
				else if (buffer == BLOCK_ELEMENT) {
					BitBlt(memDC, j * 20, i * 20, limit.bmWidth, limit.bmHeight, bpmDC4, 0, 0, SRCCOPY);
				}
				else if (buffer == FROGGE_ELEMENT) {
					BitBlt(memDC, j * 20, i * 20 ,frog.bmWidth, frog.bmHeight, bmpDC2, 0, 0, SRCCOPY);
				}
			}
		}

		BitBlt(hdc, 0, 0, rect.right, rect.bottom,memDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		return TRUE;

	// redimensiona e calcula novamente o centro
	//case WM_SIZE:
	//	memDC = NULL; // metemos novamente a NULL para que caso haja um resize na janela no WM_PAINT a janela em memoria � sempre atualizada com o tamanho novo
	//	break;
	case WM_KEYDOWN:
		
		switch (wParam)
		{
			// caso seja duplo clique
		case VK_UP:
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			break;
		case VK_DOWN:
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			break;
		case VK_LEFT:
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
			break;
		case VK_RIGHT:
			sendInfo.pressInput = wParam;
			sendInfo.x = -1;
			sendInfo.y = -1;
			//DWORD messageSize = sizeof(char) * _tcslen(wParam);
			WaitForSingleObject(hMutex, INFINITE);
			WriteFile(hPipe, &sendInfo, sizeof(PipeFroggeInput), &n, NULL);
			ReleaseMutex(hMutex);
			SetEvent(hcommand);
			ResetEvent(hcommand);
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
	HANDLE hcommand = CreateEvent(NULL, TRUE, FALSE, TEXT("eventoSapo"));
	FrogInitialdata froginitialdata;

	switch (messg)
	{
	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
			case IDOK:
			result = GetDlgItemText(hWnd, IDC_EDIT_USERNAME, username, 16);
			wsprintf(message, TEXT("[S] Hi %s"), username);
			if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SINGLEPLAYER), BM_GETCHECK, 0, 0) == BST_CHECKED && result>0)
			{
				GameOption = 0;
				MessageBox(hWnd, message, TEXT("Singleplayer"), MB_OK | MB_ICONINFORMATION);
			}
			else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_MULTIPLAYER), BM_GETCHECK, 0, 0) == BST_CHECKED && result > 0)
			{
				GameOption = 1;
				MessageBox(hWnd, message, TEXT("Multiplayer"), MB_OK | MB_ICONINFORMATION);
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
				_tcscpy_s(froginitialdata.Username,sizeof(username), username);
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
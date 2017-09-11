#include "main.h"
const int KEY_UP = 72;
const int KEY_DOWN = 80;
const int KEY_LEFT = 75;
const int KEY_RIGHT = 77;
const float DEGREES_15 =  15 * M_PI / 180.0f;
VOID DrawSurface(HWND hwnd)
{
	RECT rc;
	HDC hdc;
	double t, t1, t2;
	COLORREF clr;
	hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	TransformView(rc.right, rc.bottom);
	PatBlt(hdc, 0, 0, rc.right, rc.bottom, WHITENESS);
	Surface(hdc, 200, 200, 5, 5);
	ReleaseDC(hwnd, hdc);
}

#pragma region Функция OnCommand обработки сообщения  WM_COMMAND

VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{

	switch (id)
	{
	case 1:
		DrawSurface(hwnd);
		break;

	case 2:
		DestroyWindow(hwnd);
		break;
	case 3:
		SetPhiAndTeta(phi + DEGREES_15, teta);
		DrawSurface(hwnd);

		break;
	}
}

#pragma endregion 

void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	float degrees = 15;
	switch (vk)
	{
	case 39:
		phi += degrees * M_PI / 180.0f;;
		break;
	case 37:
		phi -= degrees * M_PI / 180.0f;;
		break;
	case 38: 
		teta += degrees * M_PI / 180.0f;;
		break;
	case 40:
		teta -= degrees * M_PI / 180.0f;;
		break;

	
	}
	DrawSurface(hwnd);	
	


}

#pragma region Функция OnContextMenu обработки сообщения  WM_CONTEXTMENU

VOID OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
	// Извлекаем координаты курсора мыши из lParam
	POINT pt;
	pt.x = xPos;
	pt.y = yPos;

	if (pt.x == -1 && pt.y == -1)
	{
		// Вызов с клавиатуры
		RECT rect;
		GetWindowRect(hwnd, &rect);
		// Выводим меню рядом с левым верхним углом окна
		pt.x = rect.left + 5;
		pt.y = rect.top + 5;
	}
	// Отображаем меню
	TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

}

#pragma endregion 

#pragma region Функция обработки сообщения WM_DESTROY

VOID OnDestroy(HWND)
{
	PostQuitMessage(0);
}

#pragma endregion 




#pragma region 0конная процедура WndProc

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_CONTEXTMENU, OnContextMenu);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hwnd, WM_KEYDOWN, OnKeyDown);
	default:
		return(DefWindowProc(hwnd, msg, wParam, lParam));
	}
}

#pragma endregion 

#pragma region Главная функция WinMain

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	/*Создание контекстного меню */
	hPopupMenu = CreatePopupMenu();
	AppendMenu(hPopupMenu, MF_STRING, 1, "Start");
	AppendMenu(hPopupMenu, MF_STRING, 3, "PHI");
	AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hPopupMenu, MF_STRING, 2, "Exit");
		
	MSG Msg;
	WNDCLASS wc;

	/*3арегистрируем класс главного окна*/
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = "Graphics";
	RegisterClass(&wc);

	/*Создадим и покажем главное окно*/
	HWND hwnd = CreateWindow("Graphics", "Graphics 3D", WS_POPUPWINDOW | WS_CAPTION,
		150, 150, 960, 720, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, SW_MAXIMIZE);

	/*Войдем в цикл обработки сообщений*/
	while (GetMessage(&Msg, NULL, 0, 0))
		DispatchMessage(&Msg);
	return 0;
}

#pragma endregion 














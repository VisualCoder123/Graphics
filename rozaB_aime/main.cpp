#include "main.h"


VOID DrawRozaA(HWND hWnd)
{
	RECT rc;
	HDC hdc;
	double t, t1, t2;
	COLORREF clr;


	hdc = GetDC(hWnd);
	GetClientRect(hWnd, &rc);
	cx = rc.right;
	cy = rc.bottom;
	zoomView = (double)rc.right / 800;
	InitViewTranform(27, 50, rc.right, 11 * rc.bottom / 10);
	PatBlt(hdc, 0, 0, rc.right, rc.bottom, WHITENESS);
	depthBuffer = new float[cx*cy];
	if (depthBuffer == NULL) return;
	InitBMP();
	for (int i = 0; i <= 100; i++)
	{
		t = (double)i / 100.0;
		if (depthBuffer == NULL) return;
		for (int k = 0; k < cx*cy; k++)
			depthBuffer[k] = -10000;

		memset(bmp, 255, bytesRow*cy);
		clr = RGB((1 - 0.3*t) * 255, 0, (1 - 0.3*t) * 128);
		for (int a = 0; a < 360; a += 90)
		{
			DefineSurfacePoints(t, a);
			DrawBezierSurface(clr);
		}
		t1 = t - 0.2;
		if (t1 < 0) t1 = 0;

		clr = RGB((1 - 0.3*t1) * 255, 0, (1 - 0.3*t1) * 128);
		for (int a = 45; a < 360; a += 90)
		{
			DefineSurfacePoints(t1, a);
			DrawBezierSurface(clr);
		}

		t2 = t - 0.6;
		if (t2 < 0) t2 = 0;
		clr = RGB(255, 0, 128);
		if (t1>0)
			for (int a = 0; a < 360; a += 90)
			{
				DefineSurfacePoints(t2, a);
				DrawBezierSurface(clr);
			}
		StretchDIBits(hdc, 0, 0, rc.right, rc.bottom, 0, 0, rc.right, rc.bottom, bmp, bmpinfo, DIB_RGB_COLORS, SRCCOPY);
	}

	delete[]bmp;
	delete[]bmpinfo;
	delete[]depthBuffer;
	ReleaseDC(hWnd, hdc);

}

#pragma region Функция OnCommand обработки сообщения  WM_COMMAND

VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case 1:
		DrawRozaA(hwnd);
		break;

	case 2:
		DestroyWindow(hwnd);
		break;
	}
}

#pragma endregion 

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
	wc.lpszClassName = "MainWindow";
	RegisterClass(&wc);

	/*Создадим и покажем главное окно*/
	HWND hwnd = CreateWindow("MainWindow", "Roza 3D Anime", WS_POPUPWINDOW | WS_CAPTION, 150, 150, 960, 720, HWND_DESKTOP, NULL, hInst, NULL);
	ShowWindow(hwnd, SW_MAXIMIZE);

	/*Войдем в цикл обработки сообщений*/
	while (GetMessage(&Msg, NULL, 0, 0))
		DispatchMessage(&Msg);
	return 0;
}

#pragma endregion 








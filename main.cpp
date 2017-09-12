#include  "main.h " 

VOID drawStatic(HWND hwnd)
{
	RECT rc;
	int k = -1;
	Shape *shape[100];
	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	for (int i = 0; i < 100; i++)
		shape[i] = NULL;

	shape[++k] = new Area;
	shape[k]->Move(0, 0, -150);
	shape[k]->Size(40);
	shape[k]->Color(0, 255, 0);

	for (int y = -200; y <= 200; y += 400)
	for (int x = -200; x <= 200; x += 400)
	{
		shape[++k] = new Column;
		shape[k]->Move(x, y, -80);
		shape[k]->Size(20);
		shape[k]->Color(255, 255, 0);

	}
	shape[++k] = new Pyramida;
	shape[k]->Move(0, 0, 150);
	shape[k]->Size(250);
	shape[k]->Color(255, 160, 0);

	shape[++k] = new Sphere;
	shape[k]->Move(0, 0, 20);
	shape[k]->Size(125);
	shape[k]->Color(255, 0, 0);

	HDC hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	cx = rc.right;
	cy = rc.bottom;

	depthBuffer = new float[cx*cy];
	if (depthBuffer == NULL)return;
	InitBMP();

	for (int i = 0; i < cx*cy; i++)
		depthBuffer[i] = -10000;

	memset(bmp, 255, 3 * bmpWidth*cy);

	viewMatrix(phi, teta, rc.right, rc.bottom);
	for (int i = 0; i <= k; i++)
	if (shape[i])
		shape[i]->Draw(hdc);

	StretchDIBits(hdc, 0, 0, rc.right, rc.bottom, 0, 0, rc.right, rc.bottom, bmp, bmpinfo, DIB_RGB_COLORS, SRCCOPY);


	for (int i = 0; i <= k; i++)
		delete[] shape[i];

	delete[] bmp;
	delete[] bmpinfo;
	delete[] depthBuffer;

	ReleaseDC(hwnd, hdc);
	SetCursor(hCursor);

}

VOID drawAnimation(HWND hwnd)
{	
	RECT rc;
	int k = -1;
	Shape *shape[100];
	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	for (int i = 0; i < 100; i++)
		shape[i] = NULL; 

	shape[++k] = new Area;
	shape[k]->Move(0, 0, -150);
	shape[k]->Size(40);
	shape[k]->Color(0, 255, 0);

	for (int y = -200; y <= 200; y+=400)
	for (int x = -200; x <= 200; x += 400)
	{
		shape[++k] = new Column;
		shape[k]->Move(x, y, -80);
		shape[k]->Size(20);
		shape[k]->Color(255, 255, 0);

	}
	shape[++k] = new Pyramida;
	shape[k]->Move(0, 0, 150);
	shape[k]->Size(250);
	shape[k]->Color(255, 160, 0);

	shape[++k] = new Sphere;
	shape[k]->Move(0, 0, 20);
	shape[k]->Size(125);
	shape[k]->Color(255, 0, 0);

	HDC hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	cx = rc.right;
	cy = rc.bottom;

	depthBuffer = new float[cx*cy];
	if (depthBuffer == NULL)return;
	InitBMP();

	for (int phi = 10, teta = 75; phi <= 370; phi++, teta++)
	{
		for (int i = 0; i < cx*cy; i++)
			depthBuffer[i] = -10000;

		memset(bmp, 255, 3 * bmpWidth*cy);

		viewMatrix(phi, teta, rc.right, rc.bottom);
		for (int i = 0; i <= k;i++)
		if (shape[i])
			shape[i]->Draw(hdc);

		StretchDIBits(hdc, 0, 0, rc.right, rc.bottom, 0, 0, rc.right, rc.bottom, bmp, bmpinfo, DIB_RGB_COLORS, SRCCOPY);

	}
	for (int i = 0; i <= k; i++)
		delete[] shape[i];

	delete[] bmp;
	delete[] bmpinfo;
	delete[] depthBuffer;

	ReleaseDC(hwnd, hdc);
	SetCursor(hCursor);

}


#pragma region Функция OnCommand обработки сообщения  WM_COMMAND

VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case 1:
		drawAnimation(hwnd);


		break;

	case 2:
		DestroyWindow(hwnd);
		break;
	}
}

#pragma endregion 

void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk)
	{
	case 39: // ->
		phi += angleSpeed * M_PI / 180.0f;;
		break;
	case 37: // <-
		phi -= angleSpeed * M_PI / 180.0f;;
		break;
	case 38: // Вверх
		teta += angleSpeed * M_PI / 180.0f;;
		break;
	case 40: // Вниз
		teta -= angleSpeed * M_PI / 180.0f;;
		break;

	case 87: // w
		transY += moveSpeed;
		break;
	case 65: // a
		transX -= moveSpeed;
		break;
	case 83: // s
		transY -= moveSpeed;
		break;
	case 68: // d
		transX += moveSpeed;
		break; 

	case 82: // r
		transZ -= moveSpeed;
		break;

	case 70: // f
		transZ += moveSpeed;
		break;


	}

	drawStatic(hwnd);

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
	wc.hbrBackground = GetStockBrush(WHITE_BRUSH);
	wc.lpszClassName = "MainWindow";
	RegisterClass(&wc);

	/*Создадим и покажем главное окно*/
	HWND hwnd = CreateWindow("MainWindow", "Использование абстрактных классов для создания 3D Anime", WS_POPUPWINDOW | WS_CAPTION,
		150, 150, 960, 720, HWND_DESKTOP, NULL, hInst, NULL);
	ShowWindow(hwnd, SW_MAXIMIZE);

	/*Войдем в цикл обработки сообщений*/
	while (GetMessage(&Msg, NULL, 0, 0))
		DispatchMessage(&Msg);
	return 0;
}

#pragma endregion 









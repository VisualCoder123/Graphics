#pragma once
#include <windows.h> 
#include <windowsx.h> 
#include <math.h>

HINSTANCE hInstance;
HMENU hPopupMenu;

#define M_PI 3.1415

double Aview, Bview, Cview, Dview, Eview, Fview, Gview, Hview, Xview, Yview;
double kxView = 30, kyView = 30, kzView = 300;
double kxFunc = 0.1, kyFunc = 0.1, dxFunc = 0, dyFunc = 0;
double phi = 27*M_PI/180.0f;
double teta = 65 * M_PI / 180.0f;

//---- определение коэффициентов преобразования -----
//---------- для экранных координат -------------

void SetPhiAndTeta(double newPhi, double newTeta)
{
	phi = newPhi;
	teta = newTeta;
}
VOID TransformView(int cx, int cy)
{
	Aview = cos(phi);
	Bview = -sin(phi);
	Cview = sin(phi)*cos(teta);
	Dview = cos(phi)*cos(teta);
	Eview = -sin(teta);
	Fview = sin(phi)*sin(teta);
	Gview = cos(phi)*sin(teta);
	Hview = cos(teta);
	Xview = cx / 2;
	Yview = cy / 2;

}

//------ преобразование мировых координат в экранные ------
VOID ScreenCoords(POINT &pt, double x, double y, double z)
{
	x *= kxView;
	y *= kyView;
	z *= kzView;
	pt.x = x*Aview + y*Bview + Xview;
	pt.y = x*Cview + y*Dview + z*Eview + Yview;
}

//------------------z = f(x,y)------------------
double Cosc(double x, double y)
{
	double r, b = 1, c = 1;
	r = sqrt(x*x + y*y);
	return cos(r*b) / (r*c + 1);
}

//--- пересчет индексов узлов в координаты поверхности ---
double X(double x)
{
	return x * kxFunc + dxFunc;
}

double Y(double y)
{
	return y * kyFunc + dyFunc;
}


VOID Surface(HDC hdc, int nx, int ny, int dx, int dy)
{
	int indx, indy;
	double x, y, z;
	POINT pt[4];

	for (indy = -ny; indy <= ny;indy+=dy)
	for (indx = -nx; indx <= nx; indx += dx)
	{
		x = X(indx);
		y = Y(indy);
		z = Cosc(x, y);
		ScreenCoords(pt[0], x, y, z);
		x = X(indx + dx);
		y = Y(indy);
		z = Cosc(x, y);
		ScreenCoords(pt[1], x, y, z);
		x = X(indx + dx);
		y = Y(indy + dy);
		z = Cosc(x, y);
		ScreenCoords(pt[2], x, y, z);
		x = X(indx);
		y = Y(indy + dy);
		z = Cosc(x, y);
		ScreenCoords(pt[3], x, y, z);
		Polygon(hdc, pt, 4);
	}
}






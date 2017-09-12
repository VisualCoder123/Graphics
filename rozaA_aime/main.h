#include <windows.h>
#include <windowsx.h> 
#include <math.h>

HMENU hPopupMenu;

#define M_PI 3.1415


struct VERTEX
{
	double x, y, z;
};

double Aview, Bview, Cview, Dview, Eview, Fview, Gview, Hview, Xview, Yview;
double zoomView = 1;
float *depthBuffer = NULL;
BYTE *bmp = NULL;
BITMAPINFO *bmpinfo = NULL;
int cx, cy, bytesRow;

double Px[4][4];
double Py[4][4];
double Pz[4][4];



//Далее PPx, PPy, PPz - массивы опорных точек трансформации
//Четверки точек описывают кривые Безье, которые, в свою очередь,
//определяют точки-ориентиры Pij поверхности Безье
double PPx[64] =
	{1,1,1,1,//P00
	80,80,80,80,    //P01
	80,80,80,256,    //P02
	1,100,200,367,    //P03
	2,2,2,2,    //P10
	120,120,120,120,   //P11
	120,120,120,201,//P12
	2,100,200,300,	//P13
	2,2,2,2,    //P20
	120,120,120,120,	//P21
	120,120,120,189,	//P22
	2,80,160,262,    //P23
	1,1,1,1,    //P30
	80,80,80,80,    //P31
	80,80,80,174,    //P32
	1,60,160,240};	//P33

double PPy[64] = {
	-1, -1, -1, -1,
	-80, -80, -80, -80,
	-1, -30, -60, -131,
	-1, -1, -1, -1,
	-40, -40, -40, -40,
	-40, -40, -40, -36,
	-1, 0, 2, 3,
	1, 1, 1, 1,
	40, 40, 40, 40,
	40, 40, 40, 37,
	1, 10, 20, 29,
	1, 1, 1, 1,
	8, 80, 80, 80,
	80, 80, 80, 76,
	1, 10, 20, 87 };


double PPz[64] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	150, 150, 150, 117,
	150, 150, 150, 36,
	0, 0, 0, 0,
	0, 0, 0, 0,
	150, 150, 150, 150,
	150, 150, 150, 68,
	0, 0, 0, 0,
	0, 0, 0, 0,
	150, 150, 150, 150,
	150, 150, 150, 47,
	0, 0, 0, 0,
	0, 0, 0, 0,
	150, 150, 150, 89,
	150, 150, 150, 0 };



//---- определение коэффициентов преобразования -----
//---------- для экранных координат -------------
VOID InitViewTranform(double alpha, double beta, int cx, int cy)
{
	alpha *= M_PI / 180.0;
	beta *= M_PI / 180.0;

	Aview = cos(alpha);
	Bview = -sin(alpha);
	Cview = sin(alpha)*cos(beta);
	Dview = cos(alpha)*cos(beta);
	Eview = cos(alpha)*cos(beta);
	Eview = -sin(beta);
	Fview = sin(alpha)*sin(beta);
	Gview = cos(alpha)*sin(beta);
	Hview = cos(beta);
	Xview = cx / 2;
	Yview = cy / 2;
}

VOID SetPixelZ(int x, int y, float z, COLORREF clr)
{
	int indx, k;
	if ((x < 0) || (x >= cx) || (y < 0) || (y >= cy))
		return;
	indx = (int)y*cx + (int)x;
	if (z >= depthBuffer[indx])
	{
		depthBuffer[indx] = z;
		k = bytesRow * (cy - y - 1) + 3 * x;
		bmp[k] = GetBValue(clr);
		bmp[k + 1] = GetGValue(clr);
		bmp[k + 2] = GetRValue(clr);
	}

}

//------ преобразование мировых координат в экранные ------
VOID ScreenKoord(VERTEX &v)
{
	VERTEX tmp;
	tmp.x = v.x*Aview + v.y*Bview + Xview;
	tmp.y = v.x*Cview + v.y*Dview + v.z*Eview + Yview;
	tmp.z = v.x*Fview + v.y*Gview + v.z*Hview;
	v = tmp;
}

//-------- косинус угла нормали для треугольника ----------
double NormalVectorCos(VERTEX v1, VERTEX v2, VERTEX v3)
{
	double nx, ny, nz;
	v2.x -= v1.x;
	v2.y -= v1.y;
	v2.z -= v1.z;
	v3.x -= v1.x;
	v3.y -= v1.y;
	v3.z -= v1.z;
	nx = v2.y*v3.z - v3.y * v2.z;
	ny = v2.z*v3.x - v3.z*v2.x;
	nz = v2.x*v3.y - v3.x*v2.y;
	return 0.2 + 0.8*fabs(nz) / sqrt(nx*nx + ny*ny + nz*nz);
}

//---- горизонталь закрашивания полигона методом Гуро ----
VOID LineGouraud(int x1, int x2, int y, double z1, double z2, double k1, double k2, COLORREF clr)
{
	int x;
	double z, k, kz, kk;
	BYTE red, gre, blu;

	red = GetRValue(clr);
	gre = GetGValue(clr);
	blu = GetBValue(clr);

	if (x1 == x2)
	{
		SetPixelZ(x1, y, z1, RGB((BYTE)(k1*(double)red),
			(BYTE)(k1*(double)gre),
			(BYTE)(k1*(double)blu)));
		return;
	}

	kz = (z2 - z1) / (double)(x2 - x1);
	kk = (k2 - k1) / (double)(x2 - x1);

	if (x1 < x2)
	for (x = x1; x < x2; x++)
	{
		z = z1 + (double)(x - x1)*kz;
		k = k1 + (double)(x - x1)*kk;
		SetPixelZ(x, y, z, RGB((BYTE)(k*(double)red),
			(BYTE)(k*(double)gre),
			(BYTE)(k*(double)blu)));
	}

	if (x1>x2)
	for (x = x2; x < x1; x++)
	{
		z = z1 + (double)(x - x1)*kz;
		k = k1 + (double)(x - x1)*kk;
		SetPixelZ(x, y, z, RGB((BYTE)(k*(double)red),
			(BYTE)(k*(double)gre),
			(BYTE)(k*(double)blu)));
	}

}

//------------ рисование одной полигональной грани -------------
//--------------- v - это координаты вершин грани ---------------
//----- kr - коефициенты отражения света для каждой вершины ----
VOID PolygonGouraud(VERTEX *v, double *kr, int nv, COLORREF clr)
{
	double x[4], z[4], k[4];
	int i, y, nhor, minY, maxY, st, en;
	double x1, x2, y1, y2, z1, z2, k1, k2;

	minY = v[0].y;
	maxY = v[0].y;

	for (i = 1; i < nv; i++)
	{
		if (minY>v[i].y) minY = v[i].y;
		if (maxY < v[i].y) maxY = v[i].y;
	}

	for (y = minY; y <= maxY; y++)
	{
		nhor = 0;
		for (i = 0; i < nv; i++)
		{
			st = i;
			en = i + 1;
			if (en >= nv) en = 0;
			y1 = v[st].y;
			y2 = v[en].y;

			if ((y >= y1) && (y<y2) || (y <= y1) && (y>y2))
			if (y1 != y2)
			{
				x1 = v[st].x;
				z1 = v[st].z;
				k1 = kr[st];
				x2 = v[en].x;
				z2 = v[en].z;
				k2 = kr[en];
				x[nhor] = x1 + (x2 - x1)*(double)(y - y1) / (double)(y2 - y1);
				z[nhor] = z1 + (z2 - z1)*(double)(y - y1) / (double)(y2 - y1);
				k[nhor] = k1 + (k2 - k1)*(double)(y - y1) / (double)(y2 - y1);
				nhor++;
			}
		}
		if (nhor == 2) LineGouraud(x[0], x[1], y, z[0], z[1], k[0], k[1], clr);
	}
}

//-- вычисление текущих точек-ориентиров поверхности Безье --
VOID DefineSurfacePoints(double t, double ang)
{
	int i, j, ii, jj, index;
	double tmpX[4], tmpY[4], tmpZ[4];
	double A, B;

	ang *= M_PI / 180;
	A = cos(ang);
	B = sin(ang);
	for (i = 0; i < 4;i++)
	for (j = 0; j < 4; j++)
	{
		for (ii = 0; ii < 4; ii++)
		{
			index = i * 16 + j * 4 + ii;
			tmpX[ii] = A*PPx[index] + B*PPy[index];
			tmpY[ii] = -B*PPx[index] + A*PPy[index];
			tmpZ[ii] = PPz[index];
		}
		for (jj = 3; jj>0; jj--)
		for (ii = 0; ii < j; ii++)
		{
			tmpX[ii] = tmpX[ii] + t*(tmpX[ii + 1] - tmpX[ii]);
			tmpY[ii] = tmpY[ii] + t*(tmpY[ii + 1] - tmpY[ii]);
			tmpY[ii] = tmpZ[ii] + t*(tmpZ[ii + 1] - tmpZ[ii]);
		}

		Px[i][j] = zoomView*tmpX[0];
		Py[i][j] = zoomView*tmpY[0];
		Pz[i][j] = zoomView*tmpZ[0];
	}
}

//вычисление координат точки кубической поверхности Безье
VOID BezierPoint(VERTEX *v, double s, double t)
{
	int C[4] = { 1, 3, 3, 1 };
	double k, si[4], ti[4], ssi[4], tti[4];
	int i, j;

	si[0] = 1;
	ti[0] = 1;
	ssi[3] = 1;
	tti[3] = 1;
	
	for (i = 1; i < 4; i++)
	{
		si[i] = s*si[i - 1];
		ti[i] = t*ti[i - 1];
		ssi[3 - i] = (1 - s)*ssi[4 - i];
		tti[3 - i] = (1 - t)*tti[4 - i];
	}

	for (i = 0; i < 4; i++)
	{
		si[i] = (double)C[i] * si[i] * ssi[i];
		ti[i] = (double)C[i] * ti[i] * tti[i];
	}

	v->x = 0;
	v->y = 0;
	v->z = 0;


	for (i = 0; i <= 3;i++)
	for (j = 0; j <= 3; j++)
	{
		k = si[i] * ti[j];
		v->x += k*Px[i][j];
		v->y += k*Py[i][j];
		v->z += k*Pz[i][j];
	}
}

VOID DrawBezierSurface(COLORREF clr)
{
	int i, j, num;
	VERTEX gran[4];
	VERTEX vb[11][11];
	VERTEX v;
	double kgran[4];
	double kr[11][11];
	double kn;

	for (i = 0; i < 11;i++)
	for (j = 0; j < 11; j++)
	{
		BezierPoint(&v, (double)i / 10, (double)j / 10);
		ScreenKoord(v);
		vb[i][j] = v;
	}

	for (i = 0; i < 11;i++)
	for (j = 0; j < 11; j++)
	{
		num = 0;
		kn = 0;
		if ((i < 10) && (j < 10))
		{
			kn += NormalVectorCos(vb[i][j], vb[i + 1][j], vb[i][j + 1]);
			num++;
		}

		if ((i<10) && (j>0))
		{
			kn += NormalVectorCos(vb[i][j], vb[i + 1][j], vb[i][j - 1]);
			num++;
		}

		if ((i > 0) && (j > 0))
		{
			kn += NormalVectorCos(vb[i][j], vb[i - 1][j], vb[i][j - 1]);
			num++;
		}
		if ((i > 0) && (j < 10))
		{
			kn += NormalVectorCos(vb[i][j], vb[i - 1][j], vb[i][j + 1]);
			num++;
		}
		kr[i][j] =  kn / (double)num;
	}

	for (i = 0; i < 10;i++)
	for (j = 0; j < 10; j++)
	{
		gran[0] = vb[i][j];
		gran[1] = vb[i + 1][j];
		gran[2] = vb[i + 1][j + 1];
		gran[3] = vb[i][j + 1];
		kgran[0] = kr[i][j];
		kgran[1] = kr[i + 1][j];
		kgran[2] = kr[i + 1][j + 1];
		kgran[3] = kr[i][j + 1];

		PolygonGouraud(gran, kgran, 4, clr);
	}
}



BOOL InitBMP()
{
	int bmWidth;

	bmpinfo = new BITMAPINFO[sizeof(BITMAPINFO)];
	if (bmpinfo == NULL) return FALSE;
	bmWidth = cx & 0xfffc;
	bmWidth += 8;
	bytesRow = 3 * bmWidth;
	bmpinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo->bmiHeader.biWidth = bmWidth;
	bmpinfo->bmiHeader.biHeight = cy;
	bmpinfo->bmiHeader.biPlanes = 1;
	bmpinfo->bmiHeader.biBitCount = 24;
	bmpinfo->bmiHeader.biCompression = BI_RGB;
	bmp = new BYTE[bytesRow*cy];
	if (bmp == NULL)
	{
		delete[]bmpinfo;
		return FALSE;
	}

}







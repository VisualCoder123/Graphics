#include <windows.h>
#include <windowsx.h> 
#include <math.h>
  
HMENU hPopupMenu;

#define M_PI 3.1415

float *depthBuffer = NULL;
BYTE *bmp = NULL;
BITMAPINFO *bmpinfo = NULL;
UINT cx, cy, bytesRow, bmpWidth;
double phi = 10, teta = 75;
double transX = 0, transY = 0, transZ = 0;
double angleSpeed = 30;
double moveSpeed = 10;
double m[12];
int cxView, cyView;
struct VERTEX
{
	double x, y, z;
};

struct Shape
{
protected:
	VERTEX center;
	double size;
	BYTE red, green, blue;
	VOID TransformVertex(VERTEX &);
public:
	virtual VOID Move(double, double, double);
	virtual VOID Size(double);
	virtual VOID Color(BYTE,BYTE,BYTE);
	virtual VOID Draw(HDC)=0;
};

struct Sphere :public Shape { virtual VOID Draw(HDC); };
struct Pyramida :public Shape { virtual VOID Draw(HDC); };
struct Cube :public Shape { virtual VOID Draw(HDC); };
struct Column :public Cube { virtual VOID Draw(HDC); };
struct Area :public Cube { virtual VOID Draw(HDC); };

VOID viewMatrix(double phi, double teta, int cx, int cy)
{
	phi *= M_PI / 180.0;
	teta *= M_PI / 180.0;
	m[0] = cos(phi);
	m[1] = -sin(phi);
	m[2] = 0;
	m[3] = transX;

	m[4] = sin(phi)*cos(teta);
	m[5] = cos(phi)*cos(teta);
	m[6] = -sin(teta);
	m[7] = transY;

	m[8] = sin(phi)*sin(teta);
	m[9] = cos(phi)*sin(teta);
	m[10] = cos(teta);
	m[11] = transZ;
	
	cxView = cx;
	cyView = cy;
}

VOID ViewTransform(VERTEX &v)

{
	VERTEX v1;
	v1.x = v.x*m[0] + v.y*m[1] + v.z*m[2] + m[3];
	v1.y = v.x*m[4] + v.y*m[5] + v.z*m[6] + m[7];
	v1.z = v.x*m[8] + v.y*m[9] + v.z*m[10] + m[11];

	double zk = 2000, zpl = 600;
	v1.x *= (zk - zpl) / (zk - v1.z);
	v1.y *= (zk - zpl) / (zk - v1.z);
	v1.z -= zpl;

	v1.x += cxView / 2;
	v1.y += cyView / 2;
	v = v1;

}

COLORREF ReflectionColor(COLORREF clr, VERTEX v1, VERTEX v2, VERTEX v3)
{
	double nx, ny, nz, k;

	v2.x -= v1.x;
	v2.y -= v1.y;
	v2.z -= v1.z;
	v3.x -= v1.x;
	v3.y -= v1.y;
	v3.z -= v1.z;
	nx = v2.y*v3.z - v3.y*v2.z;
	ny = -v2.x*v3.z +v3.x*v2.z;
	nz = v2.x*v3.y -v3.x*v2.y;
	k = 0.2 + 0.8*fabs(nz) / sqrt(nx*nx + ny*ny + nz*nz);
	return RGB(k*(double)GetRValue(clr), k*(double)GetGValue(clr), k*(double)GetBValue(clr));


}

VOID SetPixelZ(UINT x, UINT y, float z, COLORREF clr)
{

	UINT indx, k;

	if ((x < 0) || (x >= cx) || (y < 0) || (y >= cy))return;
	indx = y*cx + x;
	if (z >= depthBuffer[indx])
	{
		depthBuffer[indx] = z;
		k = bytesRow*(cy - y - 1) + 3 * x;
		bmp[k] = GetBValue(clr);
		bmp[k+1] = GetGValue(clr);
		bmp[k+2] = GetRValue(clr);

	}
}
BOOL InitBMP()
{
	bmpinfo = new BITMAPINFO[sizeof(BITMAPINFO)];
	if (bmpinfo == NULL)return FALSE;
	bmpWidth = cx & 0xfffc;
	bmpWidth += 8;
	bytesRow = 3 * bmpWidth;

	bmpinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo->bmiHeader.biWidth = bmpWidth;
	bmpinfo->bmiHeader.biHeight = cy;
	bmpinfo->bmiHeader.biPlanes = 1;
	bmpinfo->bmiHeader.biBitCount = 24;
	bmpinfo->bmiHeader.biCompression = BI_RGB;

	bmp = new BYTE[3 * bmpWidth*cy];
	if (bmp == NULL)
	{
		delete[]bmpinfo;
		return FALSE;
	}

	return TRUE;
}

VOID Line(int x1, int x2, int y, float z1, float z2, COLORREF clr)
{
	int x;
	float k, z, zz;
	UINT indx, k1;
	BYTE r, g, b;
	if ((y < 0) || (y >= cy))return;
	if (x1 == x2)
	{
		SetPixelZ(x1, y, z1, 0);
		return;
	}
	if (x1 > x2)
	{
		x = x1;
		x1 = x2;
		x2 = x;
		z = z1;
		z1 = z2;
		z2 = z;
	}
	if (x1 >= cx)return;
	if (x2 < 0)return;
	r = GetRValue(clr);
	g = GetGValue(clr);
	b = GetBValue(clr);
	k = (z2 - z1) / (float)(x2 - x1);
	zz = z1 - (float)x1*k;
	indx = (UINT)y*cx +(UINT)x1;
	k1 = bytesRow*(cy - y - 1) + 3 * x1;
	for (x = x1 + 1; x < x2; x++)
	{
		indx++;
		k1 += 3;
		if (x >= cx)break;
		if (x >= 0)
		{
			z = zz + (float)x*k;
			if (z >= depthBuffer[indx])
			{
				depthBuffer[indx] = z;
				bmp[k1] = b;
				bmp[k1 + 1] = g;
				bmp[k1 + 2] = r;
			}
		}
	}
	SetPixelZ(x1, y, z1, 0);
	SetPixelZ(x2, y, z2, 0);
}

VOID Polygon(VERTEX *v, int nv, COLORREF clr)
{
	double x[4], z[4];
	int i, y, n, minY, maxY, st, en;
	double x1, x2, y1, y2, z1, z2;

	clr = ReflectionColor(clr, v[0], v[1], v[2]);
	minY = (int)v[0].y;
	maxY = (int)v[0].y;
	for (i = 1; i < nv; i++)
	{
		if (minY > (int)v[i].y) minY = (int)v[i].y;
		if (maxY <(int)v[i].y) maxY = (int)v[i].y;

	}
	for (y = minY; y <= maxY; y++)
	{
		n = 0;
		for (i = 0; i < nv; i++)
		{
			st = i;
			en = i + 1;
			if (en >= nv) en = 0;
			x1 = v[st].x;
	y1 = v[st].y;
	z1 = v[st].z;
	x2 = v[en].x;
	y2 = v[en].y;
	z2 = v[en].z;
	if((y>=y1)&&(y<y2)||(y<=y1)&&(y>y2))
		if (y1 != y2)
		{
			x[n] = x1 + (x2 - x1)*(double)(y - y1) / (double)(y2 - y1);
			z[n] = z1 + (z2 - z1)*(double)(y - y1) / (double)(y2 - y1);
			n++;

		}
		}
		if (n == 2)Line((int)x[0], (int)x[1], (int)y, (float)z[0], (float)z[1], clr);
	}
}
VOID Shape::Move(double x, double y, double z)
{
	center.x = x;
	center.y = y;
	center.z = z;
}

VOID Shape::Size(double s)
{
	size = s;

}
VOID Shape::Color(BYTE r, BYTE g, BYTE b)
{
	red = r; green = g; blue = b;
}

VOID Shape::TransformVertex(VERTEX &v)
{
	VERTEX v1;
	v1.x = size*v.x + center.x;
	v1.y = size*v.y + center.y;
	v1.z = size*v.z + center.z;
	ViewTransform(v1);
	v = v1;

}
VOID Sphere::Draw(HDC hdc)
{
	COLORREF clr;
	double R, R2, r2, z, k;
	int x, y;
	VERTEX c = center, v = center;
	v.z += size;
	ViewTransform(v);
	ViewTransform(c);
	R2 = (c.x - v.x)*(c.x - v.x) + (c.y - v.y)*(c.y - v.y) + (c.z - v.z)*(c.z - v.z);
	R = sqrt(R2);

	for (x = 0; x <= R;x++)
	for (y = 0; y <= x; y++)
	{
		r2 = x*x + y*y;
		if (r2 > R2)break;
		z = sqrt(R2 - r2);
		k = 1 - r2 / R2;
		clr = RGB((BYTE)(k*(double)red), (BYTE)(k*(double)green), (BYTE)(k*(double)blue));
		int czz(c.z + z);
		SetPixelZ((int)(c.x + x), (int)(c.y + y), (float)(czz), clr);
		SetPixelZ((int)(c.x + x), (int)(c.y - y), (float)(czz), clr);
		SetPixelZ((int)(c.x - x), (int)(c.y + y), (float)(czz), clr);
		SetPixelZ((int)(c.x -x), (int)(c.y - y), (float)(czz), clr);
		SetPixelZ((int)(c.x +y ), (int)(c.y + x), (float)(czz), clr);
		SetPixelZ((int)(c.x + y), (int)(c.y -x), (float)(czz), clr);
		SetPixelZ((int)(c.x -y), (int)(c.y + x), (float)(czz), clr);
		SetPixelZ((int)(c.x -y), (int)(c.y -x), (float)(czz), clr);
	}
}

VOID Pyramida::Draw(HDC hdc)
{
	VERTEX v[5] = { { -1, 1, 0 }, { 1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 }, { 0, 0, 0.5 } };
	char nv[5] = { 4, 3, 3, 3, 3 };
	char indices[16] = { 0, 1, 2, 3, 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4 };

	VERTEX  face[4];

	int pos = 0;

	for (int i = 0; i < 5; i++)
		TransformVertex(v[i]);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < nv[i]; j++)
			face[j] = v[indices[pos + j]];

		Polygon(face, nv[i], RGB(red, green, blue));
		pos += nv[i];
	}
}

VOID Cube::Draw(HDC hdc)
{
	VERTEX v[8] = { { -1, 1, -1 }, { -1, 1, 1 }, { 1, 1, 1 }, { 1, 1, -1 }, { -1, -1, -1 }, { -1, -1, 1 }, { 1, -1, 1 }, { 1, -1, -1 } };
	char indices[24] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 5, 4, 1, 5, 6, 2, 2, 6, 7, 3, 0, 4, 7, 3 };
	VERTEX face[4];

	int pos = 0;

	for (int i = 0; i < 8; i++)
		TransformVertex(v[i]);
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
			face[j] = v[indices[pos + j]];

		Polygon(face, 4, RGB(red, green, blue));
		pos += 4;
	}

}

VOID Column::Draw(HDC hdc)
{
	double step = 2.5*size;
	VERTEX oldCentr = center;

	for (int i = 0; i < 5; i++)
	{
		Move(oldCentr.x, oldCentr.y, oldCentr.z + i*step);
		Cube::Draw(hdc);
	}
	center = oldCentr;
}

VOID Area::Draw(HDC hdc)
{
	double step = 2.5*size;
	VERTEX oldCentr = center;

	for (int j = -2; j < 3;j++)
	for (int i = -2; i < 3; i++)
	{
		Move(oldCentr.x + i*step, oldCentr.y + j*step, oldCentr.z);
		Cube::Draw(hdc);

	}
	center = oldCentr;

}
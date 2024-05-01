#include "Engine.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <strstream>
#include <string>

using namespace std;

// Structs

struct point
{
	float x, y, z;
};

struct triangle
{
	point p[3];
};

struct mesh
{
	vector<triangle> tris;
};

struct mat4x4
{
	float m[4][4] = { 0 };
};

// Constants

short fSizeX = 8; // Font size
short fSizeY = 8;
short width = 120; // Window size
short height = 120;

float fNear = 0.1f;
float fFar = 1000.0f;
float fFov = 90.0f;
float fAspectRatio = (float)height / (float)width;
float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

mesh objModel;

// Functions

COORD ToCoord(short x, short y) {
	COORD conv = { x, y };
	return conv;
};

	void MultiplyMatrixVector(point& i, point& o, mat4x4& m)
	{
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f)
		{
			o.x /= w; o.y /= w; o.z /= w;
		}
	};

bool LoadObjFile() {
	vector<point> pointBuffer;

	string dir;

	cout << "Read directory: ";
	cin >> dir;

	ifstream objfile(dir);

	if (!objfile.is_open()) {
		cout << "Error could not open file" << endl;
		return false;
	}


	string line;
	while (!objfile.eof())
	{

		char fileline[200];
		objfile.getline(fileline, 200);

		strstream ss;
		ss << fileline;

		char temp;

		// Vertices data
		if (fileline[0] == 'v') {
			point v;
			ss >> temp >> v.x >> v.y >> v.z;
			pointBuffer.push_back(v);
		}
		// Triangle data
		if (fileline[0] == 'f') {
			int f[3];
			ss >> temp >> f[0] >> f[1] >> f[2];
			objModel.tris.push_back({ pointBuffer[f[0] - 1], pointBuffer[f[1] - 1], pointBuffer[f[2] - 1] }); // .obj format of triangle faces starts at 1
		}

	}

	return true;

};

int main() {

	bool loaded = false;
	while (!loaded) {
		loaded = LoadObjFile();
	}

	Engine render3D(fSizeX, fSizeY, width, height);

	// Projection Matrix

	mat4x4 matRotZ, matRotX;

	mat4x4 matProj;

	matProj.m[0][0] = fAspectRatio * fFovRad;
	matProj.m[1][1] = fFovRad;
	matProj.m[2][2] = fFar / (fFar - fNear);
	matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matProj.m[2][3] = 1.0f;
	matProj.m[3][3] = 0.0f;

	float fTheta = 0.0;
	while (true) {
		// Render step
		fTheta += 0.025;

		// Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		// Rotation X
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(fTheta * 0.5f);
		matRotX.m[1][2] = sinf(fTheta * 0.5f);
		matRotX.m[2][1] = -sinf(fTheta * 0.5f);
		matRotX.m[2][2] = cosf(fTheta * 0.5f);
		matRotX.m[3][3] = 1;

		// Draw Triangles
		for (auto tri : objModel.tris)
		{
			triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

			// Rotate in Z-Axis
			MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

			// Rotate in X-Axis
			MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

			// Offset into the screen
			triTranslated = triRotatedZX;
			triTranslated.p[0].z = triRotatedZX.p[0].z + 8.0f;
			triTranslated.p[1].z = triRotatedZX.p[1].z + 8.0f;
			triTranslated.p[2].z = triRotatedZX.p[2].z + 8.0f;

			// Project triangles from 3D --> 2D
			MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
			MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
			MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

			// Scale into view
			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

			triProjected.p[0].x *= 0.5f * (float)width;
			triProjected.p[0].y *= 0.5f * (float)height; 
			
			triProjected.p[1].x *= 0.5f * (float)width;
			triProjected.p[1].y *= 0.5f * (float)height;

			triProjected.p[2].x *= 0.5f * (float)width;
			triProjected.p[2].y *= 0.5f * (float)height;

			// Rasterize triangle
			render3D.DrawTriangle(219, ToCoord(triProjected.p[0].x, triProjected.p[0].y),
				ToCoord(triProjected.p[1].x, triProjected.p[1].y),
				ToCoord(triProjected.p[2].x, triProjected.p[2].y));

		}
		render3D.DisplayFrame();
		Sleep(1 / 60.0f * 1000.0f);
	}



	return 0;
}
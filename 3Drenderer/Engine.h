#pragma once

#include <Windows.h>

using namespace std;

class Engine
{
	HANDLE outH;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	char* screenBuffer;

public:

	int columns, rows; // In num of chars

	Engine(short fontW, short fontH, short screenW, short screenH) {
		// Handle initialization
		outH = GetStdHandle(STD_OUTPUT_HANDLE);

		// Window tweaking
		FontSize(fontW, fontH);
		ScreenSize(screenW, screenH);

		GetConsoleBufferDimensions();
		SetConsoleActiveScreenBuffer(outH);

		// Memory allocation for the buffer
		screenBuffer = (char*)malloc(columns * rows);
		// Set buffer content to blank
		memset(screenBuffer, ' ', columns * rows);

	};

	// Set screen size of console
	void ScreenSize(short w, short h) {
		COORD size = { w, h };
		_SMALL_RECT window;

		window.Top = 0;
		window.Left = 0;
		window.Right = w - 1;
		window.Bottom = h - 1;

		SetConsoleWindowInfo(outH, true, &window);
		SetConsoleScreenBufferSize(outH, size);

	}

	// Set font size of console
	void FontSize(short w, short h) {
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.dwFontSize.X = w;
		cfi.dwFontSize.Y = h;

		SetCurrentConsoleFontEx(outH, FALSE, &cfi);

	}

	void GetConsoleBufferDimensions() {
		// Get info
		GetConsoleScreenBufferInfo(outH, &csbi);
		// Update
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	};

	// Draws one character to a coordinate
	void DrawCharacter(char charType, COORD pos) {
		// Out of bounds
		if (pos.X < 0 || pos.X > columns || pos.Y < 0 || pos.Y > rows)
			return;
		// Turn 2d coordinates into 1d array position
		int bufferIndex = (pos.Y * columns) + pos.X;

		if (bufferIndex < 0 || bufferIndex > columns * rows)
			return;

		screenBuffer[bufferIndex] = charType;
	};

	// Bresenham's line algorithm
	void DrawLine(char charType, COORD p0, COORD p1) {
		int dx = abs(p1.X - p0.X);
		int dy = -1 * abs(p1.Y - p0.Y); // Difference between the two points, dy is negative for comparison

		int sx = (p0.X < p1.X) ? 1 : -1;
		int sy = (p0.Y < p1.Y) ? 1 : -1; // Sign variables indicating direction of movement across x and y axis (up down and left right)

		int err = dx + dy; // Culmulative variable that will determine when to move through x or y axis

		while (true) {
			DrawCharacter(charType, p0); // Draw character

			if (p0.X == p1.X && p0.Y == p1.Y) // End case
				break;

			int e2 = 2 * err; // Simplifies the comparison

			if (e2 >= dy) { // Moves by x axis since dx is positive
				if (p0.X == p1.X) // Edge case
					break;
				err += dy; // Influence err to favour more moving through y axis instead of x axis
				p0.X += sx; // Changes x position
			}
			if (e2 <= dx) {
				if (p0.Y == p1.Y)
					break;
				err += dx;
				p0.Y += sy;
			}
		}
	};

	void DrawTriangle(char charType, COORD p0, COORD p1, COORD p2) {
		DrawLine(charType, p0, p1);
		DrawLine(charType, p1, p2);
		DrawLine(charType, p2, p0);
	}

	void DisplayFrame() {

		DWORD numChars; // Not needed but not optional
		COORD origin = { 0, 0 };

		// Set cursor position to origin and write contents of screen buffer to console
		SetConsoleCursorPosition(outH, origin);
		WriteConsoleA(outH, screenBuffer, columns * rows, &numChars, NULL);

		// Reset screen buffer
		memset(screenBuffer, ' ', columns * rows);
	};

	~Engine() {
		// Destroy screen buffer
		delete[] screenBuffer;
	};
};
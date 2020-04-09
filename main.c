#include <stdio.h>
#include "raylib.h"

#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

int main() {
	int windowWidth = 3000;
	int windowHeight = 1500;

	int overlayFontSize = 30;
	int overlayInset = 10;

	InitWindow(windowWidth, windowHeight, "Window");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		// Draw
		BeginDrawing();

		ClearBackground(WHITE);
		DrawRectangle(100, 100, 100, 100, RED);

		char overlayText[100];
		snprintf(overlayText, NELEMS(overlayText) - 1, "overlay");
		DrawText(overlayText, overlayInset, overlayInset, overlayFontSize, DARKGREEN);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

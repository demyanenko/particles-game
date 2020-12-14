#include <stdio.h>
#include "raylib.h"
#include "particles.c"

#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

int main() {
	float scaleFactor = 1.75;
	float width = 900;
	float height = 600;
	int windowWidth = width * scaleFactor;
	int windowHeight = height * scaleFactor;

	World world;
	worldInit(&world, scaleFactor, width, height);

	InitWindow(windowWidth, windowHeight, "Window");
	SetTargetFPS(60);

	ClearBackground(BLACK);

	while (!WindowShouldClose()) {
		worldUpdate(&world);

		// Draw
		BeginDrawing();

		worldRender(&world);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

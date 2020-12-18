#include <stdio.h>
#include <vector>
#include "perfSidebar.cpp"
#include "raylib.h"
#include "world.cpp"
#include "worldRender.cpp"
#include "worldUpdate.cpp"

void printParticleTypes(World *world, std::vector<int> requestedTypes)
{
	printf("t\tst\t");
	for (int j = 0; j < int(requestedTypes.size()); j++)
	{
		printf("%i_f\t%i_r\t", requestedTypes[j], requestedTypes[j]);
	}
	printf("\n");

	for (int i = 0; i < int(requestedTypes.size()); i++)
	{
		printf(
			"%i\t%i\t",
			requestedTypes[i],
			int(world->particleTypes[requestedTypes[i]].steps / world->config.physicsStepsPerFrame));
		for (int j = 0; j < int(requestedTypes.size()); j++)
		{
			printf(
				"%lf\t%lf\t",
				world->particleTypes[requestedTypes[i]].force[requestedTypes[j]],
				world->particleTypes[requestedTypes[i]].radius[requestedTypes[j]]);
		}
		printf("\n");
	}
}

int main()
{
	float scaleFactor = 5;
	int width = 400;
	int height = 400;
	int rawWidth = width * scaleFactor;
	int sidebarRawWidth = 300;
	int windowWidth = rawWidth + sidebarRawWidth;
	int windowHeight = height * scaleFactor;

	int overlayFontSize = 30;
	int overlayInset = 10;
	int sidebarInset = 50;

	World world;
	worldInit(&world, scaleFactor, width, height);
	PerfSidebar perfSidebar;
	
    srand(time(0));

	InitWindow(windowWidth, windowHeight, "Window");
	SetTargetFPS(60);

	ClearBackground(BLACK);

	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_R))
		{
			worldInit(&world, scaleFactor, width, height);
		}

		if (IsKeyPressed(KEY_P))
		{
			std::vector<int> requestedParticleTypes;
			if (IsKeyDown(KEY_ONE))
			{
				requestedParticleTypes.push_back(1);
			}
			if (IsKeyDown(KEY_TWO))
			{
				requestedParticleTypes.push_back(2);
			}
			if (IsKeyDown(KEY_THREE))
			{
				requestedParticleTypes.push_back(3);
			}
			if (IsKeyDown(KEY_FOUR))
			{
				requestedParticleTypes.push_back(4);
			}
			if (IsKeyDown(KEY_FIVE))
			{
				requestedParticleTypes.push_back(5);
			}
			if (IsKeyDown(KEY_SIX))
			{
				requestedParticleTypes.push_back(6);
			}
			printParticleTypes(&world, requestedParticleTypes);
		}

		int saveSlot = -1;
		if (IsKeyPressed(KEY_F1))
		{
			saveSlot = 1;
		}
		else if (IsKeyPressed(KEY_F2))
		{
			saveSlot = 2;
		}
		else if (IsKeyPressed(KEY_F3))
		{
			saveSlot = 3;
		}
		else if (IsKeyPressed(KEY_F4))
		{
			saveSlot = 4;
		}
		else if (IsKeyPressed(KEY_F5))
		{
			saveSlot = 5;
		}
		else if (IsKeyPressed(KEY_F6))
		{
			saveSlot = 6;
		}
		else if (IsKeyPressed(KEY_F7))
		{
			saveSlot = 7;
		}
		else if (IsKeyPressed(KEY_F8))
		{
			saveSlot = 8;
		}
		else if (IsKeyPressed(KEY_F9))
		{
			saveSlot = 9;
		}
		else if (IsKeyPressed(KEY_F10))
		{
			saveSlot = 10;
		}
		else if (IsKeyPressed(KEY_F11))
		{
			saveSlot = 11;
		}
		else if (IsKeyPressed(KEY_F12))
		{
			saveSlot = 12;
		}

		if (saveSlot != -1)
		{
			if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
			{
				worldInit(&world, scaleFactor, width, height);
				loadParticleTypes(world.particleTypes, saveSlot);
			}
			else
			{
				saveParticleTypes(world.particleTypes, saveSlot);
			}
		}

		if (IsKeyPressed(KEY_B))
		{
			// Breakpoint
			int a = 0;
			a = a;
		}

		int playerAngleDelta = (int)(IsKeyDown(KEY_A)) - (int)(IsKeyDown(KEY_D));
		int playerThrottle = IsKeyDown(KEY_W);
		GrowthMode growthMode = IsKeyDown(KEY_SPACE)
									? GrowthMode::Growing
									: IsKeyDown(KEY_C)
										  ? GrowthMode::Shedding
										  : GrowthMode::Maintaining;
		int isPlayerShooting = IsMouseButtonDown(0);
		double mouseX = (double)GetMouseX() / scaleFactor;
		double mouseY = (double)GetMouseY() / scaleFactor;
		PlayerInput playerInput = playerInputCreate(
			playerThrottle, playerAngleDelta, growthMode, isPlayerShooting, mouseX, mouseY);

		double updateParticleInteractionsTime, updateSnappedParticlesTime, applySnapPointsTime;
		double worldUpdateStart = GetTime();
		worldUpdate(
			&world, playerInput,
			&updateParticleInteractionsTime, &updateSnappedParticlesTime, &applySnapPointsTime);
		double worldUpdateEnd = GetTime();
		double worldUpdateTime = worldUpdateEnd - worldUpdateStart;

		// Draw
		BeginDrawing();

		double worldRenderStart = GetTime();
		worldRender(&world);
		double worldRenderEnd = GetTime();
		double worldRenderTime = worldRenderEnd - worldRenderStart;
		
		DrawRectangle(rawWidth, 0, windowWidth - rawWidth, sidebarInset, BLACK);

		int fps = GetFPS();
		char overlayText[100];
		snprintf(overlayText, sizeof(overlayText) - 1, "FPS %i", fps);
		DrawText(overlayText, rawWidth + sidebarInset + overlayInset, overlayInset, overlayFontSize, WHITE);

		DrawRectangle(rawWidth, 0, sidebarInset, windowHeight, BLACK);
		perfSidebar.render(
			rawWidth + sidebarInset, sidebarInset, windowWidth - rawWidth - sidebarInset,
			worldUpdateTime, updateParticleInteractionsTime, updateSnappedParticlesTime,
			applySnapPointsTime, worldRenderTime);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

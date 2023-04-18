/******************************************************************************
filename    main.c
author      Justin Chambers
DP email    justin.chambers@digipen.edu
course      GAM100 ** Do not use this code in your team project

Brief Description:
This file implements the main function and game loop.

******************************************************************************/
#include "stdafx.h" /* NULL, bool */
#include "InputProvider.h" /* GetDefaultInputFunc, ParseCommand */
#include "CommandData.h" /* struct CommandData */
#include "CommandListFactory.h" /* CreateCommandList */
#include "CommandList.h" /* CommandList_Free */
#include "CommandHandler.h" /* HandleCommand */
#include "GameStateFactory.h" /* CreateInitialGameState */
#include "GameState.h" /* struct GameState */
#include "WorldDataFactory.h" /* CreateTestWorldData */
#include "WorldData.h" /* WorldData_PrintIntroduction, WorldData_Free */
#include <Windows.h>
HWND window;
float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}
struct Vector2 {
	float x;
	float y;
};
DWORD WINAPI Thread_TransitionToScreenSize(void *dataPass) {
	float cycle = 0;
	RECT startRect;
	GetWindowRect(window, &startRect);
	float sizeX = startRect.right - startRect.left;
	float sizeY = startRect.bottom - startRect.top;

	struct Vector2 deRefData = (*(struct Vector2*)dataPass);
	
	float targetX = deRefData.x;
	float targetY = deRefData.y;
	
	while (true) {
		Sleep(16.67);
		
		SetWindowPos(window, HWND_TOP, 0, 0, sizeX, sizeY, SWP_NOMOVE);
		
		sizeX = lerp(sizeX, targetX, 0.1f);
		sizeY = lerp(sizeY, targetY, 0.1f);
		cycle += 1;
		if (round(sizeX) == targetX && round(sizeY) == targetY) {
			SetWindowPos(window, HWND_TOP, 0, 0, targetX, targetY, SWP_NOMOVE);
			free((struct Vector2*)dataPass);
			return 0;
		}
	}
	
}
HANDLE SetScreenSize(int x, int y) {
	struct Vector2* v = (struct Vector2*)malloc(sizeof(struct Vector2));
	if (v == NULL) return;
	v->x = x;
	v->y = y;
	HANDLE h = CreateThread(NULL, 0, Thread_TransitionToScreenSize, v, 0, NULL);
	return h;
}
/* The main program loop */
int main()
{
	//window setup
	window = GetConsoleWindow();
	SetWindowPos(window, HWND_TOP, 250, 250, 500, 500, SWP_SHOWWINDOW);

	//thread setup
	HANDLE thread = SetScreenSize(250, 250);
	WaitForSingleObject(thread, INFINITE);

	/* ------------------------------------------------------------------------ */
	/* Initilization and Setup */
	/* ------------------------------------------------------------------------ */
	GameState* gameState; /* The game state, reflecting the user's progress */
	WorldData* worldData; /* The world data */
	CommandList* commandList; /* The set of supported commands */
	CommandData command; /* The command entered by the user */

	/* create the initial game objects */
	gameState = CreateInitialGameState();
	worldData = CreateInitialWorldData();
	commandList = CreateCommandList();

	/* initialize the command CommandData values */
	command.commandList = commandList;
	command.context = CommandContext_User;

	/* print the world introduction*/
	WorldData_PrintIntroduction(worldData, gameState->currentRoomIndex);



	/* ------------------------------------------------------------------------ */
	/* GAME LOOP */
	/* ------------------------------------------------------------------------ */
	/* while the game is running, accept input and handle commands */
	while (gameState->isRunning)
	{
		/* print command prompt */
		printf("\nWhat is your command?\n>> ");

		/* accept user input */
		ParseCommand(GetDefaultInputFunc(), &command);

		/* include an empty line before printing the result of a command */
		printf("\n");

		/* handle the command */
		HandleCommand(&command, gameState, worldData);
	}



	/* ------------------------------------------------------------------------ */
	/* Cleanup and Exit */
	/* ------------------------------------------------------------------------ */
	WorldData_Free(&worldData);
	GameState_Free(&gameState);
	CommandList_Free(&commandList);

	/* wait for input before closing the command window */
	getchar();

	/* cleanly exit */
	return 0;
}
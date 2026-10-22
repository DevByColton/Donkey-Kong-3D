#ifndef GAME_H
#define GAME_H

#include "Types.h"

struct GameButton
{
	// TODO: Convert these to flags?
	bool endedDown = {};
	bool wasDown = {};
	bool isDown = {};
};

struct GameMouseInput
{
	int32 x = {};
	int32 y = {};
	int32 z = {};

	union
	{
		GameButton buttons[5];
		struct
		{
			GameButton leftClick;
			GameButton middleClick;
			GameButton rightClick;
			GameButton xButton1;
			GameButton xButton2;
		};
	};
};

struct GameControllerInput
{
	bool isConnected = false;
	Vector2 leftThumbstick = {};
	Vector2 rightThumbstick = {};
	float leftTrigger = {};
	float rightTrigger = {};
	float leftMotorVibrationSpeed = {};
	float rightMotorVibrationSpeed = {};
	
	union
	{
		// NOTE: Make sure to update button count when modifying buttons
		GameButton buttons[36];
		struct
		{
			// Gamepad buttons
			GameButton primary;
			GameButton secondary;
			GameButton tertiary;
			GameButton quaternary;
			GameButton leftShoulder;
			GameButton rightShoulder;
			GameButton leftThumbClick;
			GameButton rightThumbClick;
			GameButton dPadUp;
			GameButton dPadDown;
			GameButton dPadLeft;
			GameButton dPadRight;
			GameButton leftThumbstickUp;
			GameButton leftThumbstickDown;
			GameButton leftThumbstickLeft;
			GameButton leftThumbstickRight;
			GameButton back;
			GameButton start;

			// Keyboard buttons
			GameButton w;
			GameButton a;
			GameButton s;
			GameButton d;
			GameButton k;
			GameButton c;
			GameButton p;
			GameButton m;
			GameButton escape;
			GameButton space;
			GameButton up;
			GameButton down;
			GameButton left;
			GameButton right;
			GameButton enter;
			GameButton shift;
			GameButton one;
			GameButton two;
		};
	};
};

struct GameInput
{
	// index 0 = keyboard
	// index 1 - 4 = controllers
	GameControllerInput controllers[5] = {};
	GameMouseInput mouse = {};
};

struct GameStatus
{
	float deltaTime = {};
	float totalTime = {};

	// TODO: Covert these to flags?
	bool isRunning = false;
};

#endif // GAME_H
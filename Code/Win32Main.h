#ifndef WIN32MAIN_H
#define WIN32MAIN_H

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Direct3DRenderer.h"
#include "Types.h"

#define WM_APP_SIZE (WM_APP + 1)
#define WM_APP_ACTIVATEAPP (WM_APP + 2)
#define WM_APP_CLOSE (WM_APP + 3)
#define WM_APP_DESTROY (WM_APP + 4)
#define WM_APP_PAINT (WM_APP + 5)
#define WM_APP_SETCURSOR (WM_APP + 6)

#define ASSERT_OK(hResult, errorMessage) if(FAILED(hResult)){assert(!errorMessage);}

global_variable bool isWindowResizing = false;

struct GameButton
{
	// TODO: Convert these to flags?
	bool endedDown = {};
	bool wasDown = {};
	bool isDown = {};
};

struct GameMouseInput
{
	int x = {};
	int y = {};
	int z = {};

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

struct Sprite
{
    RenderData renderData = {};
    Texture2D texture = {};
    Transform transform = {};
};

struct Cube
{
    RenderData renderData = {};
    Texture2D texture = {};
};

struct GameState
{
	float deltaTime = {};
	float totalTime = {};
	bool isRunning = false;
};

#endif // WIN32MAIN_H
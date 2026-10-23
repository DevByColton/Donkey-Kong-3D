#include "Win32Main.h"
#include "Direct3DRenderer.cpp"
#include "Game.cpp"

internal_link void
Win32ProcessButtonMessage(GameButton* newState, bool isDown)
{
	if (newState->endedDown != isDown)
	{
		newState->endedDown = isDown;
	}
}

internal_link void
Win32ProcessPendingMessages(HWND window, D3DResource* d3dResource, GameMouseInput* mouse, GameControllerInput* keyboardController, GameStatus* gameStatus)
{
	MSG message = {};

	while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
			// When the window is resized
			case WM_APP_SIZE:
			{
				// TODO: This isn't working super properly when handling resizing
				// it doesnt seem to get picked up here when manually resizing
				// Maybe because its firing so many times??
				OutputDebugStringA("Window was resized \n");
			} break;

			// When the window is closed
			case WM_APP_CLOSE:
			{
				gameStatus->isRunning = false;
			} break;

			// When windows delets this window
			case WM_APP_DESTROY:
			{
				gameStatus->isRunning = false;
			} break;

			// When the window active state changes, focused or unfocused
			case WM_APP_ACTIVATEAPP:
			{
				if (message.wParam)
				{
					OutputDebugStringA("GAINED focused\n");
				}
				else
				{
					// Todo: Handled lost foucs state, pause game, reset inputs etc
					OutputDebugStringA("LOST focus\n");
				}
			} break;

			// case WM_APP_SETCURSOR:
			// {
			// 	if (stateshowCursor)
			// 	{
			// 	}
			// 	SetCursor(state->cursor);
			// 	OutputDebugStringA("Setting the cursor\n");
			// } break;

			// Mouse position
			case WM_MOUSEMOVE:
			{
				ScreenToClient(window, &message.pt);
				mouse->x = message.pt.x;
				mouse->y = message.pt.y;
			} break;

			// Mouse scroll wheel
			case WM_MOUSEWHEEL:
			{
				mouse->z = GET_WHEEL_DELTA_WPARAM(message.wParam);
			} break;

			// Mouse buttons
			case WM_LBUTTONUP:
			case WM_LBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_XBUTTONDOWN:
			{
				// TODO: If, for whatever reason, there needs to be an ability to hold down
				//       left, middle, and right click all at the same time there is a "bug?"
				//       those causes them all to reset when all three are clicked. So that 
				//       need to be solved
				WPARAM virtualKeyCode = message.wParam;
				Win32ProcessButtonMessage(&mouse->leftClick, virtualKeyCode & MK_LBUTTON);
				Win32ProcessButtonMessage(&mouse->middleClick, virtualKeyCode & MK_MBUTTON);
				Win32ProcessButtonMessage(&mouse->rightClick, virtualKeyCode & MK_RBUTTON);
				Win32ProcessButtonMessage(&mouse->xButton1, virtualKeyCode & MK_XBUTTON1);
				Win32ProcessButtonMessage(&mouse->xButton2, virtualKeyCode & MK_XBUTTON2);
			} break;

			// Keyboard input
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYUP:
			{
				WPARAM virtualKeyCode = message.wParam;
				
				int wasDown = ((message.lParam & (1 << 30)) != 0);
				int isDown = ((message.lParam & (1 << 31)) == 0);

				// NOTE: If I ever want to split these out
				//       Win32 held delay keeps this from being reliable
				//       even though its in my own loop
				// bool pressed = (!wasDown && isDown);
				// bool held = (wasDown && isDown);
				// bool released = (wasDown && !isDown);

				if (wasDown != isDown)
				{
					if (virtualKeyCode == 'W') 
					{	
						Win32ProcessButtonMessage(&keyboardController->w, isDown);
					}
					else if (virtualKeyCode == 'A') 
					{	
						Win32ProcessButtonMessage(&keyboardController->a, isDown);
					}
					else if (virtualKeyCode == 'S') 
					{	
						Win32ProcessButtonMessage(&keyboardController->s, isDown);
					}
					else if (virtualKeyCode == 'D') 
					{	
						Win32ProcessButtonMessage(&keyboardController->d, isDown);
					}
					else if (virtualKeyCode == 'C') 
					{
						Win32ProcessButtonMessage(&keyboardController->c, isDown);
					}
					else if (virtualKeyCode == 'K') 
					{	
						Win32ProcessButtonMessage(&keyboardController->k, isDown);
					}
					else if (virtualKeyCode == 'P') 
					{	
						Win32ProcessButtonMessage(&keyboardController->p, isDown);
					}
					else if (virtualKeyCode == 'M')
					{	
						Win32ProcessButtonMessage(&keyboardController->m, isDown);
					}
					else if (virtualKeyCode == VK_ESCAPE) 
					{	
						Win32ProcessButtonMessage(&keyboardController->escape, isDown);
					}
					else if (virtualKeyCode == VK_SHIFT) 
					{	
						Win32ProcessButtonMessage(&keyboardController->shift, isDown);
					}
					else if (virtualKeyCode == VK_SPACE) 
					{	
						Win32ProcessButtonMessage(&keyboardController->space, isDown);
					}
					else if (virtualKeyCode == VK_UP) 
					{
						Win32ProcessButtonMessage(&keyboardController->up, isDown);
					}
					else if (virtualKeyCode == VK_DOWN) 
					{	
						Win32ProcessButtonMessage(&keyboardController->down, isDown);
					}
					else if (virtualKeyCode == VK_LEFT)
					{
						Win32ProcessButtonMessage(&keyboardController->left, isDown);
					}
					else if (virtualKeyCode == VK_RIGHT) 
					{
						Win32ProcessButtonMessage(&keyboardController->right, isDown);
					}
					else if (virtualKeyCode == VK_RETURN)
					{
						Win32ProcessButtonMessage(&keyboardController->enter, isDown);
					}
					else if (virtualKeyCode == '1')
					{
						Win32ProcessButtonMessage(&keyboardController->one, isDown);
					}
					else if (virtualKeyCode == '2')
					{
						Win32ProcessButtonMessage(&keyboardController->two, isDown);
					}

					// Hard kill, handle alt F4
					int altKeyWasDown = ((message.lParam & (1 << 29)) != 0);
					if ((virtualKeyCode == VK_F4) && altKeyWasDown)
					{
						gameStatus->isRunning = false;
					}

					// Standard windows keys to full screen
					if (keyboardController->enter.endedDown && altKeyWasDown)
					{
						// TODO: Figure this out
						// ToggleFullscreen(d3dResource);
					}
				}
			} break;

			// All other messages
			default:
			{
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} break;
		}
	}
}

internal_link LRESULT CALLBACK
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			PostMessageA(window, WM_APP_SIZE, wParam, lParam);
		} break;

		case WM_CLOSE:
		{
			PostMessageA(window, WM_APP_CLOSE, wParam, lParam);
		} break;

		case WM_DESTROY:
		{
			PostMessageA(window, WM_APP_DESTROY, wParam, lParam);
		} break;

		// When the window active state changes, focused or unfocused
		case WM_ACTIVATEAPP:
		{
			PostMessageA(window, WM_APP_ACTIVATEAPP, wParam, lParam);
		} break;

		case WM_SETCURSOR:
		{
			PostMessageA(window, WM_APP_SETCURSOR, wParam, lParam);
		} break;

		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDOWN:
		{
			// assert(!"Mouse event came through in a non dispatch method!");
		} break;

		// Keyboard input key DOWN
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			// assert(!"Keyboard input came through in a non dispatch method!");
		} break;

		// Every other unhandled message, there is a lot
		default:
		{
			// Catch all other messages, default windows behavior for the messages
			result = DefWindowProcA(window, message, wParam, lParam);
		} break;
	}

	return result;
}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
	WNDCLASSEXA windowClass = {};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "Donkey Kong 3D";
	windowClass.hCursor= LoadCursor(0, IDC_ARROW);
	// windowClass.hIcon;
	// windowClass.hIconSm;

	if (RegisterClassExA(&windowClass))
	{
		HWND window = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"Donkey Kong 3D",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, // 200, 100,
			1920, 1080,
			0, 0, instance, 0
		);

		if (window)
		{
			D3DResource d3dResource = {};
			HRESULT result = D3DInitialize(&d3dResource);
			ASSERT_OK(result, "Unable to initialize D3D");

			result = D3DCreateSwapChain(&d3dResource, window);
			ASSERT_OK(result, "Unable to create swap chain");

			LARGE_INTEGER frequency = {};
			LARGE_INTEGER perviousFrameTime = {};
			LARGE_INTEGER currentFrameTime = {};
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&perviousFrameTime);

			GameInput input = {};
			GameStatus gameStatus = {};
			gameStatus.isRunning = true;

			// NOTE: Testing geometry
			Cube cubes[4] = {0};
			CreateCubes(cubes, d3dResource.device, &d3dResource.backBufferDescription);

			ConvexPolygon polygon = {};
			CreatePolygon(d3dResource.device, &polygon, &d3dResource.backBufferDescription);

			while (gameStatus.isRunning)
			{
				QueryPerformanceCounter(&currentFrameTime);
				gameStatus.deltaTime = ((float)(currentFrameTime.QuadPart - perviousFrameTime.QuadPart) / (float)frequency.QuadPart);
				gameStatus.totalTime += gameStatus.deltaTime;
				perviousFrameTime = currentFrameTime;

				GameMouseInput* mouse = &input.mouse;
				GameControllerInput* keyboard = &input.controllers[0];
				Win32ProcessPendingMessages(window, &d3dResource, mouse, keyboard, &gameStatus);

				if (keyboard->escape.endedDown)
				{
					gameStatus.isRunning = false;
				}

				if (keyboard->k.endedDown)
				{
					D3DSetRasterizerWireframeMode(&d3dResource);
				}

				if (keyboard->m.endedDown)
				{
					D3DSetRasterizerSolidMode(&d3dResource);
				}

				if (keyboard->one.endedDown)
				{
					if (D3DGoWindowed(&d3dResource))
					{
						OutputDebugStringA("going windowed\n");
						for (int cubeIndex = 0; cubeIndex < ARRAYSIZE(cubes); ++cubeIndex)
						{
							Cube* cube = cubes + cubeIndex;
							if (cube)
							{
								CreateViewAndPerspective(cube->eye, cube->at, cube->up, &cube->renderData.constantBufferData, &d3dResource.backBufferDescription);
							}
						}
						CreateOrthograhpicView(&polygon.renderData.constantBufferData, &d3dResource.backBufferDescription);
					}
				}

				if (keyboard->two.endedDown)
				{
					if (D3DGoFullscreen(&d3dResource))
					{
						OutputDebugStringA("going fullscreen\n");
						for (int cubeIndex = 0; cubeIndex < ARRAYSIZE(cubes); ++cubeIndex)
						{
							Cube* cube = cubes + cubeIndex;
							if (cube)
							{
								CreateViewAndPerspective(cube->eye, cube->at, cube->up, &cube->renderData.constantBufferData, &d3dResource.backBufferDescription);
							}
						}
						CreateOrthograhpicView(&polygon.renderData.constantBufferData, &d3dResource.backBufferDescription);
					}
				}

				Vector4 backBufferColor = Vector4{ 0.f, 0.96f, 0.831f, 1.f }; // 0,245,212
				if (keyboard->w.endedDown)
				{
					backBufferColor = Vector4{ 0.f, 0.733f, 0.976f, 1.f }; // 0,187,249
				}
				if (keyboard->a.endedDown)
				{
					backBufferColor = Vector4{ 0.996f, 0.894f, 0.251f, 1.f }; // 254,228,64
				}
				if (keyboard->s.endedDown)
				{
					backBufferColor = Vector4{ 0.945f, 0.356f, 0.709f, 1.f }; // 241,91,181
				}
				if (keyboard->d.endedDown)
				{
					backBufferColor = Vector4{ 0.607f, 0.364f, 0.898f, 1.f }; // 155,93,229
				}
				D3DClearRenderTarget(&d3dResource, backBufferColor);

				// GameUpdate(mouse, keyboard, &gameStatus);
				// GameRender(&d3dResource);
 
 				for (int cubeIndex = 0; cubeIndex < ARRAYSIZE(cubes); ++cubeIndex)
 				{
 					Cube* cube = cubes + cubeIndex;
 					if (cube)
 					{
 						UpdateCube(cube, cubeIndex % 2 == 0 ? 20.f : -40.f, &gameStatus);
 						D3DRender(d3dResource.context, &cube->renderData);
 					}
 				}
 				
 				UpdatePolygon(&polygon, &gameStatus);
				D3DRender(d3dResource.context, &polygon.renderData, &polygon.texture);
				
				D3DPresent(d3dResource.dxgiSwapChain);
			}
		
			D3DReleaseAllResources(&d3dResource);
		}
		else
		{
			OutputDebugStringA("CreateWindowExA failed\n");
		}
	}
	else
	{
		OutputDebugStringA("RegisterClassExA failed\n");
	}

	return 0;
}
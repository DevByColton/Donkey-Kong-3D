#include "Win32Main.h"

internal_link void
Win32ToggleFullScreen(HWND window, Win32State* state)
{
	WINDOWPLACEMENT* windowPosition = &state->windowPosition;
	DWORD style = GetWindowLong(window, GWL_STYLE);

	// NOTE: Windowed
	if (style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };

		if (GetWindowPlacement(window, windowPosition) && 
			GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
		{
			SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP,
						 monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
						 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
						 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

			state->fullscreen = true;
		}
	}
	else
	{
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, windowPosition);
		SetWindowPos(window, 0, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | 
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		state->fullscreen = false;
	}
}

internal_link void
Win32ResizeDIBSection(Win32OffscreenBuffer *buffer, int width, int height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	// NOTE: When the biHeight field is negative, this tells windows to treat
	// the bitmap as top down, not bottom up. Meaning that the first 3 bytes
	// of the image are the color for the top left pixel in the bitmap
	buffer->width = width;
	buffer->height = height;
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = buffer->height; // Top down
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	
	// NOTE: 32 bit count, 8 r, 8 g, 8 b, and 8 padding
	buffer->pitch = (((buffer->width*BITMAP_BYTES_PER_PIXEL) + SIMD_BYTE_ALIGNMENT_SIZE) & ~SIMD_BYTE_ALIGNMENT_SIZE);
	int bitmapMemorySize = (buffer->pitch*buffer->height);
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal_link void
Win32ProcessButtonMessage(GameButton* newState, bool isDown)
{
	if (newState->endedDown != isDown)
	{
		newState->endedDown = isDown;
	}
}

internal_link void
Win32ProcessPendingMessages(HWND window, Win32OffscreenBuffer* backBuffer, GameMouseInput* mouse, 
						    GameControllerInput* keyboardController, GameStatus* gameStatus, Win32State* state)
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

			// Use windows graphics to draw
			case WM_APP_PAINT:
			{
				// PAINTSTRUCT paint = {};
				// HDC deviceContext = BeginPaint(window, &paint);
				// Win32WindowDimensions dimensions = Win32GetWindowDimensions(window);
				// Win32DisplayBufferToWindow(state, backBuffer, deviceContext, dimensions.width, dimensions.height);
				// EndPaint(window, &paint);
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
				
				int32 wasDown = ((message.lParam & (1 << 30)) != 0);
				int32 isDown = ((message.lParam & (1 << 31)) == 0);

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
					int32 altKeyWasDown = ((message.lParam & (1 << 29)) != 0);
					if ((virtualKeyCode == VK_F4) && altKeyWasDown)
					{
						gameStatus->isRunning = false;
					}

					// Standard windows keys to full screen
					if (keyboardController->enter.endedDown && altKeyWasDown)
					{
						Win32ToggleFullScreen(window, state);
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

		// Use windows graphics to draw
		case WM_PAINT:
		{
			PostMessageA(window, WM_APP_PAINT, wParam, lParam);
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
			ASSERT("Mouse event came through in a non dispatch method!");
		} break;

		// Keyboard input key DOWN
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			ASSERT("Keyboard input came through in a non dispatch method!");
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
			200, 100,
			2240, 1280,
			0, 0, instance, 0
		);

		if (window)
		{
			HDC deviceContext = GetDC(window);

			// NOTE: Set up frame rate
			int defaultRefreshHz = 60;
			int win32RefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
			if (win32RefreshRate > 1)
			{
				defaultRefreshHz = win32RefreshRate;
			}

			float gameUpdateHz = (float)defaultRefreshHz;
			float targetSecondsElapsedPerFrame = 1.f / gameUpdateHz;

			Win32OffscreenBuffer backBuffer = {};
			Win32ResizeDIBSection(&backBuffer, 1920, 1080);

			OutputDebugStringA("Made the window\n");
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
#include "Win32Main.h"
#include "Direct3DRenderer.cpp"

internal_link void
Win32ProcessButtonMessage(GameButton* newState, bool isDown)
{
	if (newState->endedDown != isDown)
	{
		newState->endedDown = isDown;
	}
}

internal_link void
Win32ProcessPendingMessages(HWND window, D3DResource* d3dResource, GameMouseInput* mouse, GameKeyboardInput* keyboard, GameState* gameState)
{
	MSG message = {};

	while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
			// When the window is resized
			case WM_APP_SIZE:
			{
				if (!d3dResource->isWindowResizing && gameState->isRunning)
				{
					OutputDebugStringA("IN WM_APP_SIZE\n");
					d3dResource->isWindowResizing = true;
					unsigned int width = LOWORD(message.lParam);
					unsigned int height = HIWORD(message.lParam);
					ResizeBackBuffer(d3dResource, width, height);
					OutputDebugStringA("DONE WM_APP_SIZE\n\n");
				}
			} break;

			// When the window is closed
			case WM_APP_CLOSE:
			{
				gameState->isRunning = false;
			} break;

			// When windows delets this window
			case WM_APP_DESTROY:
			{
				gameState->isRunning = false;
			} break;

			// When the window active state changes, focused or unfocused
			case WM_APP_ACTIVATEAPP:
			{
				if (message.wParam)
				{
					gameState->isFocused = true;
					OutputDebugStringA("GAINED focused\n");
				}
				else
				{
					// Todo: Handled lost foucs state, pause game, reset inputs etc
					gameState->isFocused = false;
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
				if (gameState->isFocused)
				{
					ScreenToClient(window, &message.pt);
					mouse->x = message.pt.x;
					mouse->y = message.pt.y;
				}
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
						Win32ProcessButtonMessage(&keyboard->w, isDown);
					}
					else if (virtualKeyCode == 'A') 
					{	
						Win32ProcessButtonMessage(&keyboard->a, isDown);
					}
					else if (virtualKeyCode == 'S') 
					{	
						Win32ProcessButtonMessage(&keyboard->s, isDown);
					}
					else if (virtualKeyCode == 'D') 
					{	
						Win32ProcessButtonMessage(&keyboard->d, isDown);
					}
					else if (virtualKeyCode == 'C') 
					{
						Win32ProcessButtonMessage(&keyboard->c, isDown);
					}
					else if (virtualKeyCode == 'K') 
					{	
						Win32ProcessButtonMessage(&keyboard->k, isDown);
					}
					else if (virtualKeyCode == 'P') 
					{	
						Win32ProcessButtonMessage(&keyboard->p, isDown);
					}
					else if (virtualKeyCode == 'M')
					{	
						Win32ProcessButtonMessage(&keyboard->m, isDown);
					}
					else if (virtualKeyCode == VK_ESCAPE) 
					{	
						Win32ProcessButtonMessage(&keyboard->escape, isDown);
					}
					else if (virtualKeyCode == VK_SHIFT) 
					{	
						Win32ProcessButtonMessage(&keyboard->shift, isDown);
					}
					else if (virtualKeyCode == VK_SPACE) 
					{	
						Win32ProcessButtonMessage(&keyboard->space, isDown);
					}
					else if (virtualKeyCode == VK_UP) 
					{
						Win32ProcessButtonMessage(&keyboard->up, isDown);
					}
					else if (virtualKeyCode == VK_DOWN) 
					{	
						Win32ProcessButtonMessage(&keyboard->down, isDown);
					}
					else if (virtualKeyCode == VK_LEFT)
					{
						Win32ProcessButtonMessage(&keyboard->left, isDown);
					}
					else if (virtualKeyCode == VK_RIGHT) 
					{
						Win32ProcessButtonMessage(&keyboard->right, isDown);
					}
					else if (virtualKeyCode == VK_RETURN)
					{
						Win32ProcessButtonMessage(&keyboard->enter, isDown);
					}
					else if (virtualKeyCode == '1')
					{
						Win32ProcessButtonMessage(&keyboard->one, isDown);
					}
					else if (virtualKeyCode == '2')
					{
						Win32ProcessButtonMessage(&keyboard->two, isDown);
					}

					// Hard kill, handle alt F4
					int altKeyWasDown = ((message.lParam & (1 << 29)) != 0);
					if ((virtualKeyCode == VK_F4) && altKeyWasDown)
					{
						gameState->isRunning = false;
					}

					// Standard windows keys to full screen
					if (keyboard->enter.endedDown && altKeyWasDown)
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

		case WM_SYSCOMMAND:
		{
			if ((wParam & 0xFFF0) == SC_MOVE)
			{
				// NOTE: When the window moves, nothing do for now
			    OutputDebugStringA("SC_MOVE\n");
			}
#if 0			
			else if ((wParam & 0xFFF0) == SC_SIZE)
			{
				PostMessageA(window, WM_APP_SIZE, wParam, lParam);
				OutputDebugStringA("SC_SIZE\n");
			}
			else if ((wParam & 0xFFF0) == SC_MAXIMIZE)
			{
				PostMessageA(window, WM_APP_SIZE, wParam, lParam);
				OutputDebugStringA("SC_MAXIMIZE\n");
			}
			else if ((wParam & 0xFFF0) == SC_MINIMIZE)
			{
				PostMessageA(window, WM_APP_SIZE, wParam, lParam);
				OutputDebugStringA("SC_MINIMIZE\n");
			}
#endif			
		};

		// Every other unhandled message, there is a lot
		default:
		{
			// Catch all other messages, default windows behavior for the messages
			result = DefWindowProcA(window, message, wParam, lParam);
		} break;
	}

	return result;
}

internal_link void
Win32ProcessXInputButton(DWORD xInputButton, DWORD buttonBit, GameButton* previousState, GameButton* newState)
{
	newState->endedDown = ((xInputButton & buttonBit) == buttonBit);
}

internal_link float
Win32ProcessXInputStickPosition(SHORT value, SHORT deadZoneThreshHold)
{
	float result = 0.f;

	if ((value < -deadZoneThreshHold) || (value > deadZoneThreshHold))
	{	
		result = (float)value / 32768.f;
	}

	return result;
}

internal_link float
Win32ProcessXInputTriggerPosition(BYTE value)
{
	float result = 0.f;

	if (value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		result = (float)value / 255.f;
	}

	return result;
}

internal_link void
Win32ProcessXInputControllers(GameInput* oldInput, GameInput* newInput)
{
	for (unsigned int controllerIndex = 0; controllerIndex < ARRAYSIZE(newInput->controllers); ++controllerIndex)
	{
		GameControllerInput* previousController = oldInput->controllers + controllerIndex;
		GameControllerInput* newController = newInput->controllers + controllerIndex;

		XINPUT_STATE controllerState = {};
		if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
		{
			// Controller is connected
			XINPUT_GAMEPAD* gamepad = &controllerState.Gamepad;
			newController->isConnected = true;

			// Triggers
			newController->leftTrigger = Win32ProcessXInputTriggerPosition(gamepad->bLeftTrigger);
			newController->rightTrigger = Win32ProcessXInputTriggerPosition(gamepad->bRightTrigger);

			// Left thumbstick x and y
			newController->leftThumbstick = Vector2{Win32ProcessXInputStickPosition(gamepad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
													Win32ProcessXInputStickPosition(gamepad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)};

			// Right thumbstick x and y
			newController->rightThumbstick = Vector2{Win32ProcessXInputStickPosition(gamepad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
													 Win32ProcessXInputStickPosition(gamepad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)};

			// Thumbstick actions (just left thumbstick for now)
			float threshold = 0.5f;
			Win32ProcessXInputButton((newController->leftThumbstick.x< -threshold) ? 1 : 0, 1,
									 &previousController->leftThumbstickLeft, &newController->leftThumbstickLeft);

			Win32ProcessXInputButton((newController->leftThumbstick.x > threshold) ? 1 : 0, 1,
									 &previousController->leftThumbstickRight, &newController->leftThumbstickRight);

			Win32ProcessXInputButton((newController->leftThumbstick.y < -threshold) ? 1 : 0, 1,
									 &previousController->leftThumbstickDown, &newController->leftThumbstickDown);

			Win32ProcessXInputButton((newController->leftThumbstick.y > threshold) ? 1 : 0, 1,
									 &previousController->leftThumbstickUp, &newController->leftThumbstickUp);

			// All buttons and dpad
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_DPAD_UP, &previousController->dPadUp, &newController->dPadUp);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_DPAD_DOWN, &previousController->dPadDown, &newController->dPadDown);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_DPAD_LEFT,&previousController->dPadLeft, &newController->dPadLeft);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, &previousController->dPadRight, &newController->dPadRight);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_A, &previousController->primary, &newController->primary);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_B, &previousController->secondary, &newController->secondary);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_X, &previousController->tertiary, &newController->tertiary);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_Y, &previousController->quaternary, &newController->quaternary);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_START, &previousController->start, &newController->start);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_BACK, &previousController->back, &newController->back);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_LEFT_THUMB, &previousController->leftThumbClick, &newController->leftThumbClick);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_RIGHT_THUMB, &previousController->rightThumbClick, &newController->rightThumbClick);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &previousController->leftShoulder, &newController->leftShoulder);
			Win32ProcessXInputButton(gamepad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &previousController->rightShoulder, &newController->rightShoulder);

			// Set the controller vibration state
			XINPUT_VIBRATION vibration = {};

			if (newController->leftMotorVibrationSpeed > 0.f)
			{
				vibration.wLeftMotorSpeed = (WORD)(newController->leftMotorVibrationSpeed * 65535);
			}

			if (newController->rightMotorVibrationSpeed > 0.f)
			{
				vibration.wRightMotorSpeed = (WORD)(newController->rightMotorVibrationSpeed * 65535);
			}

			XInputSetState(controllerIndex, &vibration);
		}
		else
		{
			newController->isConnected = false;
			// Controller is not connected
			// This is where I can take action if a controller
			// is not plugged in
		}
	}
}

internal_link void
UpdateInputState(GameInput* previousInput, GameInput* newInput)
{
	//
	// NOTE: Udpate mouse
	//
	newInput->mouse = {};
	newInput->mouse.x = previousInput->mouse.x;
	newInput->mouse.y = previousInput->mouse.y;
	newInput->mouse.z = previousInput->mouse.z;
	for (unsigned int buttonIndex = 0; buttonIndex < ARRAYSIZE(newInput->mouse.buttons); ++buttonIndex)
	{
		newInput->mouse.buttons[buttonIndex].endedDown = previousInput->mouse.buttons[buttonIndex].endedDown;
	}

	//
	// NOTE: Update keyboard
	//
	newInput->keyboard = {};
	newInput->keyboard.isConnected = true; // Assuming a keyboard is always connected!
	for (unsigned int buttonIndex = 0; buttonIndex < ARRAYSIZE(newInput->keyboard.buttons); ++buttonIndex)
	{
		newInput->keyboard.buttons[buttonIndex].endedDown = previousInput->keyboard.buttons[buttonIndex].endedDown;
	}

	//
	// NOTE: Update controllers
	//
	for (unsigned int controllerIndex = 0; controllerIndex < ARRAYSIZE(newInput->controllers); ++controllerIndex)
	{
		GameControllerInput* previousController = previousInput->controllers + controllerIndex;
		GameControllerInput* newController = newInput->controllers + controllerIndex;

		if (previousController->isConnected)
		{
			*newController = {};
			newController->isConnected = previousController->isConnected;
			newController->leftThumbstick = previousController->leftThumbstick;
			newController->rightThumbstick = previousController->rightThumbstick;
			newController->leftTrigger = previousController->leftTrigger;
			newController->rightTrigger = previousController->rightTrigger;
			newController->leftMotorVibrationSpeed = previousController->leftMotorVibrationSpeed;
			newController->rightMotorVibrationSpeed = previousController->rightMotorVibrationSpeed;

			for (unsigned int buttonIndex = 0; buttonIndex < ARRAYSIZE(newController->buttons); ++buttonIndex)
			{
				newController->buttons[buttonIndex].endedDown = previousController->buttons[buttonIndex].endedDown;
			}
		}
		else
		{
			*previousController = {};
			*newController = {};
		}
	}
}

internal_link void
HandleCameraMovement(GameState* gameState, GameInput* previousInput, GameInput* newInput, Camera* camera, 
					 float cameraSpeed, float mouseSensitivity, float thumbstickSensitivity)
{
	cameraSpeed *= gameState->deltaTime;
	mouseSensitivity *= gameState->deltaTime;
	thumbstickSensitivity *= gameState->deltaTime;

	//
	// NOTE: Handle look
	//
	float yaw = camera->yaw;
	float pitch = camera->pitch;

	if (gameState->isFocused)
	{
		// NOTE: Mouse look
		float xMouseOffset = (float)newInput->mouse.x - (float)previousInput->mouse.x;
		float yMouseOffset = (float)previousInput->mouse.y - (float)newInput->mouse.y;

		// NOTE: Controller look
		float xRightThumbstick = 0.f;
		float yRightThumbstick = 0.f;
		for (unsigned int controllerIndex = 0; controllerIndex < ARRAYSIZE(newInput->controllers); ++controllerIndex)
		{
			GameControllerInput* newController = newInput->controllers + controllerIndex;
			
			if (newController->isConnected)
			{
				xRightThumbstick += newController->rightThumbstick.x;
				yRightThumbstick += newController->rightThumbstick.y;
			}
		}

		yaw += ((xMouseOffset * mouseSensitivity) + (xRightThumbstick * thumbstickSensitivity));
		pitch += ((yMouseOffset * mouseSensitivity) + (yRightThumbstick * thumbstickSensitivity));

		// NOTE: Allow the camera to look up and down but clamping before hitting 90 where the LookAt would flip
		if(pitch > 89.f)
		{
			pitch = 89.f;
		}
		if(pitch < -89.f)
		{
			pitch = -89.f;
		}
	}

	// NOTE: Store yaw and pitch before converting to radians
	camera->yaw = yaw;
	camera->pitch = pitch;

	yaw = DirectX::XMConvertToRadians(yaw);
	pitch = DirectX::XMConvertToRadians(pitch);
	float x = DirectX::XMScalarCos(yaw) * DirectX::XMScalarCos(pitch);
	float y = DirectX::XMScalarSin(pitch);
	float z = DirectX::XMScalarSin(yaw) * DirectX::XMScalarCos(pitch);
	DirectX::XMVECTOR direction = DirectX::XMVectorSet(x, y, z, 0.f);

	//
	// NOTE: Handle directional movement
	//
	float moveRight = 0.0f;
	float moveFront = 0.0f;
	for (unsigned int controllerIndex = 0; controllerIndex < ARRAYSIZE(newInput->controllers); ++controllerIndex)
	{
		GameControllerInput* controller = newInput->controllers + controllerIndex;
		if (controller->isConnected)
		{
			moveRight += controller->leftThumbstick.x;
			moveFront += controller->leftThumbstick.y;
		}
	}

	DirectX::FXMVECTOR previousPosition = DirectX::XMLoadFloat3(&camera->position);
	DirectX::FXMVECTOR front = DirectX::XMVector3Normalize(direction);
	DirectX::FXMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
	DirectX::FXMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(front, up));
	
	// NOTE: Factor in summed controller movement
	DirectX::XMVECTOR newMovement = DirectX::XMVectorAdd(DirectX::XMVectorScale(right, moveRight), DirectX::XMVectorScale(front, moveFront));

	// NOTE: Keyboard movement
	if (newInput->keyboard.w.endedDown)
	{
		newMovement = DirectX::XMVectorAdd(newMovement, front);
	}
	if (newInput->keyboard.a.endedDown)
	{
		newMovement = DirectX::XMVectorSubtract(newMovement, right);
	}
	if (newInput->keyboard.s.endedDown)
	{
		newMovement = DirectX::XMVectorSubtract(newMovement, front);
	}
	if (newInput->keyboard.d.endedDown)
	{
		newMovement = DirectX::XMVectorAdd(newMovement, right);
	}
	
	float newMovementLengthSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(newMovement));
	if (newMovementLengthSq > 1.f)
	{
		newMovement = DirectX::XMVector3Normalize(newMovement);
	}

	DirectX::XMVECTOR newPosition = DirectX::XMVectorAdd(previousPosition, DirectX::XMVectorScale(newMovement, cameraSpeed));
	DirectX::XMStoreFloat3(&camera->position, newPosition);
	DirectX::XMStoreFloat3(&camera->front, front);
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
			WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE,
			0, 0, // 200, 100,
			0, 0, // 1920, 1080,
			0, 0, instance, 0
		);

		if (window)
		{
			D3DResource d3dResource = {};
			HRESULT result = D3DInitialize(&d3dResource);
			ASSERT_OK(result, "Unable to initialize D3D");

			result = D3DCreateSwapChain(&d3dResource, window);			
			ASSERT_OK(result, "Unable to create swap chain");

			int mouseX = d3dResource.backBufferDescription.Width / 2;
			int mouseY = d3dResource.backBufferDescription.Height / 2;
			SetCursorPos(mouseX, mouseY);
			GameInput previousInput = {};
			previousInput.mouse.x = mouseX;
			previousInput.mouse.y = mouseY;
			GameInput newInput = {};

			GameState gameState = {};
			gameState.isRunning = true;

			Cube cube = {};
			CreateTexturedCube(d3dResource.device, &cube, 1.f);
			Transform cubeTransforms[] = 
			{
				{DirectX::XMFLOAT3(0.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
				{DirectX::XMFLOAT3(5.f, -3.5f, -8.f), DirectX::XMFLOAT3(20.f, 17.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
				{DirectX::XMFLOAT3(-3.f, -2.5f, -4.f), DirectX::XMFLOAT3(60.f, 35.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
				{DirectX::XMFLOAT3(4.f, 4.f, -6.f), DirectX::XMFLOAT3(19.f, 19.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
				{DirectX::XMFLOAT3(-5.f, 7.f, -10.f), DirectX::XMFLOAT3(42.5f, 35.7f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
				{DirectX::XMFLOAT3(-8.3f, 8.f, -6.f), DirectX::XMFLOAT3(78.f, 45.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
				{DirectX::XMFLOAT3(1.f, 0.f, -7.6f), DirectX::XMFLOAT3(89.f, 49.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
			};

			Sprite sprite = {};
			CreateSprite(d3dResource.device, &sprite);
			ScaleUniform(&sprite.transform, 0.5f);

			Camera camera = {};
			camera.yaw = -90.f;
			camera.pitch = 0.f;
			camera.position = DirectX::XMFLOAT3(0.f, 0.f, 3.f);
			camera.front = DirectX::XMFLOAT3(0.f, 0.f, -1.f);
			camera.up = DirectX::XMFLOAT3(0.f, 1.f, 0.f);

			Vector4 backBufferColor = Vector4{0.f, 0.96f, 0.831f, 1.f};
			D3DClearRenderTarget(&d3dResource, backBufferColor);

			LARGE_INTEGER frequency = {};
			LARGE_INTEGER perviousFrameTime = {};
			LARGE_INTEGER currentFrameTime = {};
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&perviousFrameTime);

			while (gameState.isRunning)
			{
				QueryPerformanceCounter(&currentFrameTime);
				gameState.deltaTime = ((float)(currentFrameTime.QuadPart - perviousFrameTime.QuadPart) / (float)frequency.QuadPart);
				gameState.totalTime += gameState.deltaTime;
				perviousFrameTime = currentFrameTime;

				//
				// NOTE: Windows message and input processing
				//
				UpdateInputState(&previousInput, &newInput);
				Win32ProcessPendingMessages(window, &d3dResource, &newInput.mouse, &newInput.keyboard, &gameState);
				Win32ProcessXInputControllers(&previousInput, &newInput);

				if (newInput.keyboard.escape.endedDown)
				{
					gameState.isRunning = false;
				}

				if (newInput.keyboard.k.endedDown)
				{
					D3DSetRasterizerWireframeMode(&d3dResource);
				}

				if (newInput.keyboard.m.endedDown)
				{
					D3DSetRasterizerSolidMode(&d3dResource);
				}

				if (newInput.keyboard.one.endedDown)
				{
					D3DGoWindowed(&d3dResource);
				}

				if (newInput.keyboard.two.endedDown)
				{
					D3DGoExclusiveFullscreen(&d3dResource);
				}

				HandleCameraMovement(&gameState, &previousInput, &newInput, &camera, 5.f, 30.f, 100.f);
				D3DClearRenderTarget(&d3dResource, backBufferColor);

				//
				// NOTE: Update and render
				//
				float padding = 20.f;
				sprite.transform.position = DirectX::XMFLOAT3(
				    (d3dResource.backBufferDescription.Width / 2.f - (sprite.transform.scale.x * sprite.texture.width) / 2.f) - padding,
				    (d3dResource.backBufferDescription.Height / 2.f - (sprite.transform.scale.y * sprite.texture.height) / 2.f) - padding,
				    0.f
				);
				ApplyWorldTransform(&sprite.transform, &sprite.renderData.constantBufferData);
 				OrthographicProjection(&sprite.renderData.constantBufferData, d3dResource.backBufferDescription.Width, d3dResource.backBufferDescription.Height);
				D3DRender(d3dResource.context, &sprite.renderData, &sprite.texture);

				for (int transformIndex = 0; transformIndex < ARRAYSIZE(cubeTransforms); ++transformIndex)
				{
					Transform* cubeTransform = cubeTransforms + transformIndex;
					ApplyWorldTransform(cubeTransform, &cube.renderData.constantBufferData);
					ApplyCameraTransform(&cube.renderData.constantBufferData, &camera);
					PerspectiveProjection(&cube.renderData.constantBufferData, d3dResource.backBufferDescription.Width, d3dResource.backBufferDescription.Height);
					D3DRender(d3dResource.context, &cube.renderData, &cube.texture);
				}

				//
				// NOTE: Note present the buffer
				//
				D3DPresent(&d3dResource);

				// NOTE: Swap input state
				GameInput temp = newInput;
				newInput = previousInput;
				previousInput = temp;
			}
			
			//
			// NOTE: On quit release resources
			//
			D3DClearRenderTarget(&d3dResource, Vector4{0.5f, 0.5f, 0.5f, 1.f});
			D3DPresent(&d3dResource);
			D3DReleaseAllResources(&d3dResource);
			DestroyWindow(window);
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
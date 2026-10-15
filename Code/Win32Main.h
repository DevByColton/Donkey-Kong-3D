#ifndef WIN32MAIN_H
#define WIN32MAIN_H

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include "Game.h"

#define WM_APP_SIZE (WM_APP + 1)
#define WM_APP_ACTIVATEAPP (WM_APP + 2)
#define WM_APP_CLOSE (WM_APP + 3)
#define WM_APP_DESTROY (WM_APP + 4)
#define WM_APP_PAINT (WM_APP + 5)
#define WM_APP_SETCURSOR (WM_APP + 6)

#endif // WIN32MAIN_H
#include "Game.h"

internal_link void
GameUpdate(GameMouseInput* mouse, GameControllerInput* keyboard, GameStatus* status)
{	
	// Nothing do
}

internal_link void
GameRender(D3DResource* d3dResource, Vector4 color)
{
	D3DRenderAndPresent(d3dResource, color);
}
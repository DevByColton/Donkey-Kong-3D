#ifndef DIRECTX_H
#define DIRECTX_H

#include <d3d11.h>
#include <dxgi1_3.h>

struct D3DResource
{
    UINT deviceFlags;
    D3D_FEATURE_LEVEL featureLevel;
    D3D11_VIEWPORT viewport;
    D3D11_TEXTURE2D_DESC backBufferDescription;
    
    ID3D11Device* device = {0};
    ID3D11DeviceContext* context = {0};
    IDXGISwapChain* dxgiSwapChain = {0};

    ID3D11Texture2D* backBuffer = {0};
    ID3D11RenderTargetView* renderTarget = {0};

    ID3D11Texture2D* depthStencil = {0};
    ID3D11DepthStencilView* depthStencilView = {0};
};

#endif // DIRECTX_H
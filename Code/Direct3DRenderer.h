#ifndef DIRECTX_H
#define DIRECTX_H

#include <d3d11.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>

struct VertexPositionColor
{
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 color = {};
};

struct ConstantBufferData
{
    DirectX::XMFLOAT4X4 world = {};
    DirectX::XMFLOAT4X4 view = {};
    DirectX::XMFLOAT4X4 projection = {};
};
static_assert((sizeof(ConstantBufferData) % 16) == 0, "ConstantBufferData size must be 16-byte aligned");

struct RenderData
{
    uint32 indexCount = {};
    ID3D11Buffer* vertexBuffer = {0};
    ID3D11Buffer* indexBuffer = {0};
    ID3D11Buffer* constantBuffer = {0};
    ConstantBufferData constantBufferData = {};
    ID3D11VertexShader* vertexShader = {0};
    ID3D11PixelShader* pixelShader = {0};
    ID3D11InputLayout* inputLayout = {0};
};

struct Cube
{
    RenderData renderData = {};
    DirectX::XMVECTOR eye = {};
    DirectX::XMVECTOR at = {};
    DirectX::XMVECTOR up = {};
    float rotation = {};
};

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
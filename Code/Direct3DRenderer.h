#ifndef DIRECTX_H
#define DIRECTX_H

struct LightData
{
    DirectX::XMFLOAT4 ambientStrength = {};
    DirectX::XMFLOAT4 lightColor = {};
    DirectX::XMFLOAT4 lightPosition = {};
};
static_assert((sizeof(LightData) % 16) == 0, "LightData size must be 16-byte aligned");

struct LightSource
{
    ID3D11Buffer* buffer = {0};
    LightData data = {};
};

struct ModelViewProjectionData
{
    DirectX::XMFLOAT4X4 model = {};
    DirectX::XMFLOAT4X4 view = {};
    DirectX::XMFLOAT4X4 projection = {};
};
static_assert((sizeof(ModelViewProjectionData) % 16) == 0, "ModelViewProjectionData size must be 16-byte aligned");

struct RenderData
{
    int indexCount = {};
    ID3D11Buffer* vertexBuffer = {0};
    ID3D11Buffer* indexBuffer = {0};
    ID3D11Buffer* modelViewProjectionBuffer = {0};
    ModelViewProjectionData modelViewProjectionData = {};
    ID3D11VertexShader* vertexShader = {0};
    ID3D11PixelShader* pixelShader = {0};
    ID3D11InputLayout* inputLayout = {0};
};

struct D3DResource
{
    UINT deviceFlags;
    D3D_FEATURE_LEVEL featureLevel;
    D3D11_VIEWPORT viewport;
    D3D11_TEXTURE2D_DESC backBufferDescription;
    bool isWindowResizing = false;
    
    ID3D11Device* device = {0};
    ID3D11DeviceContext* context = {0};
    IDXGISwapChain1* dxgiSwapChain = {0};

    ID3D11Texture2D* backBuffer = {0};
    ID3D11RenderTargetView* renderTarget = {0};

    ID3D11Texture2D* depthStencil = {0};
    ID3D11DepthStencilView* depthStencilView = {0};

    ID3D11RasterizerState* rasterizerState = {0};
};

struct VertexShader
{
    ID3D11InputLayout* inputLayout = {0};
    ID3D11VertexShader* shader = {0};
};

struct Shaders
{
    VertexShader baseVertexShader = {};
    VertexShader baseTextureVertexShader = {};

    ID3D11PixelShader* basePixelShader = {0};
    ID3D11PixelShader* lightingPixelShader = {0};
    ID3D11PixelShader* baseTexturePixelShader = {0};
    ID3D11PixelShader* lightingTexturePixelShader = {0};
};

struct Texture2D
{
    int width = {};
    int height = {}; 
    int numberOfColorChannels = {};
    ID3D11Texture2D* source = {0};
    ID3D11ShaderResourceView* view = {0};
    ID3D11SamplerState* sampler = {0};
    ID3D11BlendState* blend = {0};
};

struct Transform
{
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 rotation = {};
    DirectX::XMFLOAT3 scale = {};
};

struct Camera
{
    float pitch = {};
    float yaw = {};
    
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 front = {};
    DirectX::XMFLOAT3 up = {};
};

struct Vertex
{
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 color = {};
    DirectX::XMFLOAT2 texCoord = {};
    DirectX::XMFLOAT3 normal = {};
    DirectX::XMFLOAT3 worldPosition = {};
};

struct VertexColor
{
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 color = {};
};

struct Mesh
{
    unsigned int vertexCount = 0;    
    Vertex* vertices = {0};

    unsigned int indexCount = 0;
    unsigned short* indices = {0};

    RenderData renderData = {};

    ~Mesh() 
    { 
        free(vertices);
        free(indices);
    }
};

#endif // DIRECTX_H
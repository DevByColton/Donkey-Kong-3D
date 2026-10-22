#include "Direct3DRenderer.h"

internal_link void
CreateOrthograhpicView(ConstantBufferData* constantBufferData, D3D11_TEXTURE2D_DESC* backBuffer)
{
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	view = DirectX::XMMatrixTranspose(view);
	DirectX::XMStoreFloat4x4(&constantBufferData->view, view);

	DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicRH(
		1.f,
		1.f,
		0.01f,   // near plane
		100.f    // far plane
	);
	projection = DirectX::XMMatrixTranspose(projection);
	DirectX::XMStoreFloat4x4(&constantBufferData->projection, projection);
}

internal_link void
CreateViewAndPerspective(DirectX::XMVECTOR eye, DirectX::XMVECTOR at,  DirectX::XMVECTOR up,
						 ConstantBufferData* constantBufferData, D3D11_TEXTURE2D_DESC* backBuffer)
{
	// TODO: These can be in the same method for now because the camera is static
	// if the camera were to move then the view would need to be updated every frame

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eye, at, up);
	view = DirectX::XMMatrixTranspose(view);
	DirectX::XMStoreFloat4x4(&constantBufferData->view, view);

	float aspectRatioX = backBuffer->Width / (float)backBuffer->Height;
	float aspectRatioY = aspectRatioX < (16.f / 9.f) ? aspectRatioX / (16.f / 9.f) : 1.f;

	float fov = 2.f * std::atan(std::tan(DirectX::XMConvertToRadians(70) * 0.5f) / aspectRatioY);
	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatioX, 0.01f, 100.f);
	projection = DirectX::XMMatrixTranspose(projection);
	DirectX::XMStoreFloat4x4(&constantBufferData->projection, projection);
}

internal_link HRESULT
D3DConfigureBackBuffer(D3DResource* d3dResource)
{
	//
	// NOTE: Create the back buffer
	//
	HRESULT result = d3dResource->dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dResource->backBuffer);
	ASSERT_OK(result, "Cannot create the back buffer");

	//
	// NOTE: Create the render target
	//
	result = d3dResource->device->CreateRenderTargetView(d3dResource->backBuffer, 0, (ID3D11RenderTargetView**)&d3dResource->renderTarget);
	ASSERT_OK(result, "Cannot create the render target");
	d3dResource->backBuffer->GetDesc(&d3dResource->backBufferDescription);

	//
	// NOTE: Create the depth stencil
	//
	CD3D11_TEXTURE2D_DESC depthStencil(
	    DXGI_FORMAT_D24_UNORM_S8_UINT,
	    (UINT)d3dResource->backBufferDescription.Width,
	    (UINT)d3dResource->backBufferDescription.Height,
	    1, // This depth stencil view has only one texture.
	    1, // Use a single mipmap level.
	    D3D11_BIND_DEPTH_STENCIL
	);
	result = d3dResource->device->CreateTexture2D(&depthStencil, 0, &d3dResource->depthStencil);
	ASSERT_OK(result, "Cannot create the depth stencil");

	//
	// NOTE: Create depth stencil view
	//
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilView(D3D11_DSV_DIMENSION_TEXTURE2D);
	result = d3dResource->device->CreateDepthStencilView(d3dResource->depthStencil, &depthStencilView, &d3dResource->depthStencilView);
	ASSERT_OK(result, "Cannot create the depth stencil view");

	//
	// NOTE: Create the viewport
	//
	ZeroMemory(&d3dResource->viewport, sizeof(D3D11_VIEWPORT));
	d3dResource->viewport.Height = (float)d3dResource->backBufferDescription.Height;
	d3dResource->viewport.Width = (float)d3dResource->backBufferDescription.Width;
	d3dResource->viewport.MinDepth = 0;
	d3dResource->viewport.MaxDepth = 1;

	//
	// NOTE: Set the viewport on the context
	//
	d3dResource->context->RSSetViewports(1, &d3dResource->viewport);

	return result;
}

inline void
D3DSetRasterizerWireframeMode(D3DResource* d3dResource)
{
	D3D11_RASTERIZER_DESC currentRasterizerDescription = {};
	d3dResource->rasterizerState->GetDesc(&currentRasterizerDescription);

	if (currentRasterizerDescription.FillMode != D3D11_FILL_WIREFRAME)
	{
		D3D11_RASTERIZER_DESC rasterizerDescription(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE);
		d3dResource->device->CreateRasterizerState(&rasterizerDescription, &d3dResource->rasterizerState);
		d3dResource->context->RSSetState(d3dResource->rasterizerState);
	}
}

inline void
D3DSetRasterizerSolidMode(D3DResource* d3dResource)
{
	D3D11_RASTERIZER_DESC currentRasterizerDescription = {};
	d3dResource->rasterizerState->GetDesc(&currentRasterizerDescription);

	if (currentRasterizerDescription.FillMode != D3D11_FILL_SOLID)
	{
		D3D11_RASTERIZER_DESC rasterizerDescription(D3D11_FILL_SOLID, D3D11_CULL_NONE);
		d3dResource->device->CreateRasterizerState(&rasterizerDescription, &d3dResource->rasterizerState);
		d3dResource->context->RSSetState(d3dResource->rasterizerState);
	}
}

internal_link HRESULT
D3DInitialize(D3DResource* d3dResource)
{
	D3D_FEATURE_LEVEL featureLevels[] = {
	    D3D_FEATURE_LEVEL_11_1,
	    D3D_FEATURE_LEVEL_11_0,
	    D3D_FEATURE_LEVEL_10_1,
	    D3D_FEATURE_LEVEL_10_0,
	    D3D_FEATURE_LEVEL_9_3,
	    D3D_FEATURE_LEVEL_9_2,
	    D3D_FEATURE_LEVEL_9_1
	};

	d3dResource->deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if DEBUG
	d3dResource->deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result = D3D11CreateDevice(
    	0,
    	D3D_DRIVER_TYPE_HARDWARE,
    	0,
    	d3dResource->deviceFlags,
    	featureLevels,
    	ARRAYSIZE(featureLevels),
    	D3D11_SDK_VERSION,
    	&d3dResource->device,
    	&d3dResource->featureLevel,
    	&d3dResource->context
    );
    ASSERT_OK(result, "Cannot create the D3 device");

    D3D11_RASTERIZER_DESC rasterizerDescription(D3D11_FILL_SOLID, D3D11_CULL_NONE);
    d3dResource->device->CreateRasterizerState(&rasterizerDescription, &d3dResource->rasterizerState);
    d3dResource->context->RSSetState(d3dResource->rasterizerState);

    return result;
}

internal_link HRESULT
D3DCreateSwapChain(D3DResource* d3dResource, HWND window)
{
	DXGI_SWAP_CHAIN_DESC dxgiDescription;
	ZeroMemory(&dxgiDescription, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiDescription.Windowed = TRUE;
	dxgiDescription.BufferCount = 2;
	dxgiDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	dxgiDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiDescription.SampleDesc.Count = 1;
	dxgiDescription.SampleDesc.Quality = 0;
	dxgiDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	dxgiDescription.OutputWindow = window;

	// NOTE: To manually set the frame rate
	// dxgiDescription.BufferDesc.RefreshRate.Numerator = 60;
	// dxgiDescription.BufferDesc.RefreshRate.Denominator = 1;

	IDXGIDevice3* dxgiDevice = {0};
	HRESULT result = d3dResource->device->QueryInterface(__uuidof(IDXGIDevice3), (void **)&dxgiDevice);
	ASSERT_OK(result, "Cannot create dxgiDevice");

	// Create swap chain.
	IDXGIAdapter* dxgiAdapter = {0};
	IDXGIFactory* dxgiFactory = {0};

	//
	// NOTE: Get the adapter
	//
	result = dxgiDevice->GetAdapter(&dxgiAdapter);
	ASSERT_OK(result, "Cannot create dxgiAdapter");

	//
	// NOTE: Create the swap chain
	//
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	result = dxgiFactory->CreateSwapChain(d3dResource->device, &dxgiDescription, &d3dResource->dxgiSwapChain);
	ASSERT_OK(result, "Cannot create dxgiSwapChain");

	D3DConfigureBackBuffer(d3dResource);

	return result;
}

inline void
D3DReleaseBackBuffer(D3DResource* d3dResource)
{
	d3dResource->renderTarget->Release();
	d3dResource->backBuffer->Release();
	d3dResource->depthStencilView->Release();
	d3dResource->depthStencil->Release();
	d3dResource->context->Flush();
}

inline bool
D3DGoFullscreen(D3DResource* d3dResource)
{
	BOOL isFullscreen;
	d3dResource->dxgiSwapChain->GetFullscreenState(&isFullscreen, 0);
	
	if (!isFullscreen)
	{
		HRESULT result = d3dResource->dxgiSwapChain->SetFullscreenState(TRUE, NULL);
		ASSERT_OK(result, "Unable to SetFullscreenState to TRUE on dxgiSwapChain");

		D3DReleaseBackBuffer(d3dResource);

		d3dResource->dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		result = D3DConfigureBackBuffer(d3dResource);
		ASSERT_OK(result, "Unable to configure back buffer after Fullscreen attempt");

		return true;
	}

	return false;
}

inline bool
D3DGoWindowed(D3DResource* d3dResource)
{
	BOOL isFullscreen;
	d3dResource->dxgiSwapChain->GetFullscreenState(&isFullscreen, 0);

	if (isFullscreen)
	{
		HRESULT result = d3dResource->dxgiSwapChain->SetFullscreenState(FALSE, NULL);
		ASSERT_OK(result, "Unable to SetFullscreenState to FALSE on dxgiSwapChain");

		D3DReleaseBackBuffer(d3dResource);

		d3dResource->dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		result = D3DConfigureBackBuffer(d3dResource);
		ASSERT_OK(result, "Unable to configure back buffer after Fullscreen attempt");

		return true;
	}

	return false;
}

inline void
D3DClearRenderTarget(D3DResource* d3dResource, Vector4 color)
{
	ID3D11DeviceContext* context = d3dResource->context;
	ID3D11RenderTargetView* renderTarget = d3dResource->renderTarget;
	ID3D11DepthStencilView* depthStencilView = d3dResource->depthStencilView;

	context->ClearRenderTargetView(renderTarget, color.e);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencilView);
}

internal_link void
D3DRender(ID3D11DeviceContext* context, RenderData* renderData)
{
	//
	// NOTE: Refresh constant buffers with data from update loop
	//
	context->UpdateSubresource(renderData->constantBuffer, 0, 0, &renderData->constantBufferData, 0, 0);

	//
	// NOTE: Input-Assembler (IA) stage
	//
	UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &renderData->vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(renderData->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(renderData->inputLayout);

    //
    // NOTE: Vertex shader stage
    //
    context->VSSetShader(renderData->vertexShader, 0, 0);

    //
    // NOTE: Constant buffer data
    //
    context->VSSetConstantBuffers(0, 1, &renderData->constantBuffer);

    //
    // NOTE: Pixel shader stage
    //
    context->PSSetShader(renderData->pixelShader, 0, 0);

    //
    // NOTE: Do the actual draw
    //
    context->DrawIndexed(renderData->indexCount, 0, 0);
}

internal_link void
D3DPresent(IDXGISwapChain* dxgiSwapChain)
{
	//
	// NOTE: Presenting with vsync enabled
	//
	dxgiSwapChain->Present(1, 0);
}

internal_link void
D3DReleaseAllResources(D3DResource* d3dResource)
{
	d3dResource->context->Flush();
	d3dResource->renderTarget->Release();
	d3dResource->depthStencilView->Release();
	d3dResource->depthStencil->Release();
	d3dResource->backBuffer->Release();
	d3dResource->context->Release();
	d3dResource->device->Release();
	d3dResource->dxgiSwapChain->Release();
}

//
// NOTE: Cube methods
//

inline void
CreateVertexBuffer(ID3D11Device* device, VertexPositionColor verticies[], uint32 vertexCount, RenderData* renderData)
{
	CD3D11_BUFFER_DESC vertexBufferDescription(sizeof(VertexPositionColor) * vertexCount, D3D11_BIND_VERTEX_BUFFER);
	
	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexData.pSysMem = verticies;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&vertexBufferDescription, &vertexData, &renderData->vertexBuffer);
	if (FAILED(result))
	{
		OutputDebugStringA("Unable to create polygon vertex buffer\n");
	}
}

inline void
CreateIndexBuffer(ID3D11Device* device, ushort indices[], uint32 indexCount, RenderData* renderData)
{
	renderData->indexCount = indexCount;

	CD3D11_BUFFER_DESC indexBufferDescription(sizeof(ushort) * indexCount, D3D11_BIND_INDEX_BUFFER);

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    HRESULT result = device->CreateBuffer(&indexBufferDescription, &indexData, &renderData->indexBuffer);
    if (FAILED(result))
    {
    	OutputDebugStringA("Unable to create polygon index buffer\n");
    }
}

internal_link void
LoadShaders(ID3D11Device* device, RenderData* renderData, const char* vertexShaderFilename, const char* pixelShaderFilename)
{
	sizet destSize = 4096;
	sizet bytesRead = 0;

	FILE* vertexShaderFile = {0};
	FILE* pixelShaderFile = {0};
	BYTE* bytes = new BYTE[destSize];

	HRESULT result = S_OK;
	errno_t openFileResult = 0;

	//
	// NOTE: Vertex shader file load
	//
	if (vertexShaderFilename)
	{
		openFileResult = fopen_s(&vertexShaderFile, vertexShaderFilename, "rb");
		if (openFileResult == 0)
		{
			bytesRead = fread_s(bytes, destSize, 1, 4096, vertexShaderFile);

			result = device->CreateVertexShader(bytes, bytesRead, nullptr, &renderData->vertexShader);
			if (FAILED(result))
			{
				OutputDebugStringA("Unable to read: ");
				OutputDebugStringA(vertexShaderFilename);
				OutputDebugStringA("\n");
			}
		}

		D3D11_INPUT_ELEMENT_DESC inputAssemblerDescription[] =
		{
		    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		    { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		result = device->CreateInputLayout(
			inputAssemblerDescription, 
			ARRAYSIZE(inputAssemblerDescription), 
			bytes, 
			bytesRead, 
			&renderData->inputLayout
		);
		if (FAILED(result))
		{
			OutputDebugStringA("Unable to create input layout\n");
		}

		delete bytes;
		bytes = new BYTE[destSize];
		bytesRead = 0;
	}

	//
	// NOTE: Pixel shader file load
	//
	if (pixelShaderFilename)
	{
		openFileResult = fopen_s(&pixelShaderFile, pixelShaderFilename, "rb");
		if (openFileResult == 0)
		{
			bytesRead = fread_s(bytes, destSize, 1, 4096, pixelShaderFile);

			result = device->CreatePixelShader(bytes, bytesRead, 0, &renderData->pixelShader);
			if (FAILED(result))
			{
				OutputDebugStringA("Unable to read: ");
				OutputDebugStringA(pixelShaderFilename);
				OutputDebugStringA("\n");
			}
		}

		delete bytes;

		CD3D11_BUFFER_DESC constantBufferDescription(sizeof(ConstantBufferData), D3D11_BIND_CONSTANT_BUFFER);
		result = device->CreateBuffer(&constantBufferDescription, 0, &renderData->constantBuffer);
		if (FAILED(result))
		{
			OutputDebugStringA("Unable to create constant buffer\n");
		}
	}

	fclose(vertexShaderFile);
	fclose(pixelShaderFile);
}

inline void
UpdateTriangle(ConvexPolygon* polygon, GameStatus* gameStatus)
{
	// NOTE: Just modulating the green channel to show change
	polygon->renderData.constantBufferData.colorModulation.x = 1.f;
	polygon->renderData.constantBufferData.colorModulation.y = (std::sin(gameStatus->totalTime) / 2.0f) + 0.5f;
	polygon->renderData.constantBufferData.colorModulation.z = 1.f;
	polygon->renderData.constantBufferData.colorModulation.w = 1.f;
}

internal_link void
UpdateCube(Cube* cube, float rotationSpeed, GameStatus* gameStatus)
{
	cube->rotation += rotationSpeed*gameStatus->deltaTime;
	float angle = DirectX::XMConvertToRadians(cube->rotation);
	
	DirectX::XMMATRIX axisRotation = DirectX::XMMatrixRotationY(angle);
	axisRotation = DirectX::XMMatrixTranspose(axisRotation);
	DirectX::XMStoreFloat4x4(&cube->renderData.constantBufferData.world, axisRotation);
}

internal_link void
CreateTriangle(ID3D11Device* device, ConvexPolygon* polygon, D3D11_TEXTURE2D_DESC* backBufferDescription)
{
#if 0	
	VertexPositionColor verticies[] =
	{
		{DirectX::XMFLOAT3(-0.5, -0.5f, 0.f), DirectX::XMFLOAT3(1.f, 0.f, 0.f)},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.f), DirectX::XMFLOAT3(0.f, 1.f, 0.f)},
		{DirectX::XMFLOAT3(0.f, 0.5f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, 1.f)},
	};

	ushort indices[] = 
	{
		0, 2, 1
	};
#else
	VertexPositionColor verticies[] =
	{
		{DirectX::XMFLOAT3(0.5, 0.5f, 0.f), DirectX::XMFLOAT3(0.443f, 0.404f, 0.482f)},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.f), DirectX::XMFLOAT3(0.659f, 0.624f, 0.592f)},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.f), DirectX::XMFLOAT3(0.843f, 0.659f, 0.494f)},
		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.f), DirectX::XMFLOAT3(0.565f, 0.369f, 0.373f)},
	};

	ushort indices[] = 
	{
		1, 2, 3,
		3, 0, 1
	};
#endif	

	CreateVertexBuffer(device, verticies, ARRAYSIZE(verticies), &polygon->renderData);
	CreateIndexBuffer(device, indices, ARRAYSIZE(indices), &polygon->renderData);

	CreateOrthograhpicView(&polygon->renderData.constantBufferData, backBufferDescription);
	LoadShaders(device, &polygon->renderData, "BasicOrthographicVertexShader.cso", "BasicPixelShader.cso");
}

internal_link void
CreateCube(ID3D11Device* device, Cube* cube, float size)
{
	VertexPositionColor verticies[] =
	{
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(0.f, 0.f, 0.f)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(0.f, 0.f, 1.f)},
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(0.f, 1.f, 0.f)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(0.f, 1.f, 1.f)},

		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(1.f, 0.f, 0.f)},
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(1.f, 0.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 0.f)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
	};

	ushort indices[] =
	{
		0,2,1, // -x
		1,2,3,

		4,5,6, // +x
		5,7,6,

		0,1,5, // -y
		0,5,4,

		2,6,7, // +y
		2,7,3,

		0,4,6, // -z
		0,6,2,

		1,3,7, // +z
		1,7,5,
	};

	CreateVertexBuffer(device, verticies, ARRAYSIZE(verticies), &cube->renderData);
	CreateIndexBuffer(device, indices, ARRAYSIZE(indices), &cube->renderData);
}

internal_link void
CreateCubes(Cube cubes[4], ID3D11Device* device, D3D11_TEXTURE2D_DESC* backBufferDescription)
{
	Cube cube0 = {};
	CreateCube(device, &cube0, 0.25f);
	cube0.eye = DirectX::XMVectorSet(0.f, 0.5f, 2.f, 0.f);
	cube0.at = DirectX::XMVectorSet(-1.f, 0.f, 0.f, 0.f);
	cube0.up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	CreateViewAndPerspective(cube0.eye, cube0.at, cube0.up, &cube0.renderData.constantBufferData, backBufferDescription);
	LoadShaders(device, &cube0.renderData, "BasicPerspectiveVertexShader.cso", "BasicPixelShader.cso");

	Cube cube1 = {};
	CreateCube(device, &cube1, 0.25f);
	cube1.eye = DirectX::XMVectorSet(0.f, -0.5f, 2.f, 0.f);
	cube1.at = DirectX::XMVectorSet(0.f, -0.8f, 0.f, 0.f);
	cube1.up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	CreateViewAndPerspective(cube1.eye, cube1.at, cube1.up, &cube1.renderData.constantBufferData, backBufferDescription);
	LoadShaders(device, &cube1.renderData, "BasicPerspectiveVertexShader.cso", "BasicPixelShader.cso");

	Cube cube2 = {};
	CreateCube(device, &cube2, 0.25f);
	cube2.eye = DirectX::XMVectorSet(0.f, 0.5f, 2.f, 0.f);
	cube2.at = DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f);
	cube2.up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	CreateViewAndPerspective(cube2.eye, cube2.at, cube2.up, &cube2.renderData.constantBufferData, backBufferDescription);
	LoadShaders(device, &cube2.renderData, "BasicPerspectiveVertexShader.cso", "BasicPixelShader.cso");

	Cube cube3 = {};
	CreateCube(device, &cube3, 0.25f);
	cube3.eye = DirectX::XMVectorSet(0.f, 0.5f, 2.f, 0.f);
	cube3.at = DirectX::XMVectorSet(0.f, 0.8f, 0.f, 0.f);
	cube3.up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	CreateViewAndPerspective(cube3.eye, cube3.at, cube3.up, &cube3.renderData.constantBufferData, backBufferDescription);
	LoadShaders(device, &cube3.renderData, "BasicPerspectiveVertexShader.cso", "BasicPixelShader.cso");

	cubes[0] = cube0;
	cubes[1] = cube1;
	cubes[2] = cube2;
	cubes[3] = cube3;
}
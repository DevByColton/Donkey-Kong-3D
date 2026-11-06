internal_link void
OrthographicProjection(ConstantBufferData* constantBufferData, unsigned int viewWidth, unsigned int viewHeight)
{
	float nearPlane = 0.01f;
	float farPlane = 100.f;

	DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicRH((float)viewWidth, (float)viewHeight, nearPlane, farPlane);
	DirectX::XMStoreFloat4x4(&constantBufferData->projection, DirectX::XMMatrixTranspose(projection));
}

internal_link void
PerspectiveProjection(ConstantBufferData* constantBufferData, unsigned int viewWidth, unsigned int viewHeight)
{
	float fov = DirectX::XMConvertToRadians(65.f);
	float aspectRatio = (float)viewWidth / (float)viewHeight;
	float nearPlane = 0.01f;
	float farPlane = 100.f;

	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
	DirectX::XMStoreFloat4x4(&constantBufferData->projection, DirectX::XMMatrixTranspose(projection));
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
	d3dResource->context->Flush();
	d3dResource->renderTarget->Release();
	d3dResource->backBuffer->Release();
	d3dResource->depthStencilView->Release();
	d3dResource->depthStencil->Release();
}

inline void
ResizeBackBuffer(D3DResource* d3dResource, unsigned int width, unsigned int height)
{
	D3DReleaseBackBuffer(d3dResource);

	HRESULT result = d3dResource->dxgiSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	ASSERT_OK(result, "Unable to resize back buffer\n");

	result = D3DConfigureBackBuffer(d3dResource);
	ASSERT_OK(result, "Unable to configure back buffer");

	isWindowResizing = false;
	OutputDebugStringA("Window was resized \n");
}

inline void
D3DGoFullscreen(D3DResource* d3dResource)
{
	BOOL isFullscreen;
	d3dResource->dxgiSwapChain->GetFullscreenState(&isFullscreen, 0);
	
	if (!isFullscreen && !isWindowResizing)
	{
		isWindowResizing = true;
		HRESULT result = d3dResource->dxgiSwapChain->SetFullscreenState(TRUE, NULL);
		ASSERT_OK(result, "Unable to SetFullscreenState to TRUE on dxgiSwapChain");

		ResizeBackBuffer(d3dResource, 0, 0);
	}
}

inline void
D3DGoWindowed(D3DResource* d3dResource)
{
	BOOL isFullscreen;
	d3dResource->dxgiSwapChain->GetFullscreenState(&isFullscreen, 0);

	if (isFullscreen && !isWindowResizing)
	{
		isWindowResizing = true;
		HRESULT result = d3dResource->dxgiSwapChain->SetFullscreenState(FALSE, NULL);
		ASSERT_OK(result, "Unable to SetFullscreenState to FALSE on dxgiSwapChain");

		ResizeBackBuffer(d3dResource, 1280, 720);
	}
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
D3DRender(ID3D11DeviceContext* context, RenderData* renderData, Texture2D* texture = 0)
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
    if (texture)
    {
    	context->PSSetShaderResources(0, 1, &texture->view);
    	context->PSSetSamplers(0, 1, &texture->sampler);
    	context->OMSetBlendState(texture->blend, NULL, 0xFFFFFFFF);
    }

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
	d3dResource->context->ClearState();
	d3dResource->context->Flush();
	HRESULT result = d3dResource->dxgiSwapChain->SetFullscreenState(FALSE, NULL);
	ASSERT_OK(result, "Unable to SetFullscreenState to FALSE on dxgiSwapChain");
	d3dResource->renderTarget->Release();
	d3dResource->depthStencilView->Release();
	d3dResource->depthStencil->Release();
	d3dResource->backBuffer->Release();
	d3dResource->context->Release();
	d3dResource->device->Release();
	d3dResource->dxgiSwapChain->Release();
}

inline void
CreateVertexBuffer(ID3D11Device* device, VertexPositionColor verticies[], int vertexCount, RenderData* renderData)
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
		OutputDebugStringA("Unable to create sprite vertex buffer\n");
	}
}

inline void
CreateIndexBuffer(ID3D11Device* device, unsigned short indices[], int indexCount, RenderData* renderData)
{
	renderData->indexCount = indexCount;

	CD3D11_BUFFER_DESC indexBufferDescription(sizeof(unsigned short) * indexCount, D3D11_BIND_INDEX_BUFFER);

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    HRESULT result = device->CreateBuffer(&indexBufferDescription, &indexData, &renderData->indexBuffer);
    if (FAILED(result))
    {
    	OutputDebugStringA("Unable to create sprite index buffer\n");
    }
}

internal_link void
LoadShaders(ID3D11Device* device, RenderData* renderData, const char* vertexShaderFilename, const char* pixelShaderFilename)
{
	size_t destSize = 4096;
	size_t bytesRead = 0;

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
		    { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

internal_link void
GenerateTexture(ID3D11Device* device, Texture2D* texture, const char* texture1FileName, const char* texture2FileName)
{
	// NOTE: Load 4 channels RGBA becuase we are using DXGI_FORMAT_R8G8B8A8_UNORM
	unsigned char* stbTexture1Data = stbi_load(texture1FileName, &texture->width, &texture->height, &texture->numberOfColorChannels, 4);
	if (!stbTexture1Data)
	{
		OutputDebugStringA("Unable to load image: ");
		OutputDebugStringA(texture1FileName);
		OutputDebugStringA("\n");
		return;
	}

	// TODO: This call to load is going to overwirte texture width and height, which is fine because I am using a texture 2D array
	// so the width and hieght of these need to be the same anyway. Just might want to add validation of some sort
	unsigned char* stbTexture2Data = stbi_load(texture2FileName, &texture->width, &texture->height, &texture->numberOfColorChannels, 4);
	if (!stbTexture2Data)
	{
		OutputDebugStringA("Unable to load image: ");
		OutputDebugStringA(texture2FileName);
		OutputDebugStringA("\n");
		return;
	}

	//
	// NOTE: Create the texture 2D
	//
	D3D11_TEXTURE2D_DESC textureDescription = {};
	textureDescription.Width = texture->width;
	textureDescription.Height = texture->height;
	textureDescription.ArraySize = 2;
	textureDescription.MipLevels = 1;
	textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDescription.SampleDesc.Count = 1;
	textureDescription.SampleDesc.Quality = 0;
	textureDescription.Usage = D3D11_USAGE_DEFAULT;
	textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDescription.CPUAccessFlags = 0;
	textureDescription.MiscFlags = 0;

	int rowPitch = texture->width * 4;

	D3D11_SUBRESOURCE_DATA textureData[2];
	ZeroMemory(&textureData, sizeof(D3D11_SUBRESOURCE_DATA));
	textureData[0].pSysMem = stbTexture1Data;
	textureData[0].SysMemPitch = rowPitch;
	textureData[0].SysMemSlicePitch = 0;
	textureData[1].pSysMem = stbTexture2Data;
	textureData[1].SysMemPitch = rowPitch;
	textureData[1].SysMemSlicePitch = 0;

	HRESULT result = device->CreateTexture2D(&textureDescription, textureData, &texture->source);
	ASSERT_OK(result, "Unable to create texture 2D");

	//
	// NOTE: Creat the shader resource
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC resourceDescription = {};
	resourceDescription.Format = textureDescription.Format;
	resourceDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	resourceDescription.Texture2DArray.MostDetailedMip = 0;
	resourceDescription.Texture2DArray.MipLevels = 1;
	resourceDescription.Texture2DArray.FirstArraySlice = 0;
	resourceDescription.Texture2DArray.ArraySize = textureDescription.ArraySize;

	result = device->CreateShaderResourceView(texture->source, &resourceDescription, &texture->view);
	ASSERT_OK(result, "Unable to create shader resource");

	//
	// NOTE: Create the sampler state
	//
	D3D11_SAMPLER_DESC samplerDescription = {};
	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescription.MinLOD = 0.f;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDescription, &texture->sampler);
	ASSERT_OK(result, "Unable to create shader sample state");

	//
	// NOTE: Create the blend state
	//
	D3D11_BLEND_DESC blendDescription = {};
	blendDescription.RenderTarget[0].BlendEnable = TRUE;
	blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = device->CreateBlendState(&blendDescription, &texture->blend);
	ASSERT_OK(result, "Unable to create blend description");

	stbi_image_free(stbTexture1Data);
	stbi_image_free(stbTexture2Data);
}

internal_link void
ApplyWorldTransform(Transform* transform, ConstantBufferData* constantBuffer)
{
	DirectX::XMMATRIX model = DirectX::XMMatrixIdentity();

	//
	// NOTE: Scale
	//
	DirectX::XMMATRIX scale = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(
		transform->scale.x, 
		transform->scale.y, 
		transform->scale.z, 
		1.f)
	);
	model = DirectX::XMMatrixMultiply(model, scale);

	//
	// NOTE: Rotate
	//
	// From the microsoft docs, negating since I am using a RH coordinate system
	// Angles are measured clockwise when looking along the rotation axis toward the origin. 
	// This is a left-handed coordinate system. 
	// To use right-handed coordinates, negate all three angles
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
		-DirectX::XMConvertToRadians(transform->rotation.x),
		-DirectX::XMConvertToRadians(transform->rotation.y),
		-DirectX::XMConvertToRadians(transform->rotation.z)
	);
	model = DirectX::XMMatrixMultiply(model, rotation);

	//
	// NOTE: Translate
	//
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(
		transform->position.x, 
		transform->position.y, 
		transform->position.z
	);
	model = DirectX::XMMatrixMultiply(model, translation);

	DirectX::XMStoreFloat4x4(&constantBuffer->model, DirectX::XMMatrixTranspose(model));
}

internal_link void
ApplyCameraTransform(ConstantBufferData* constantBuffer, Camera* camera)
{
	DirectX::FXMVECTOR position = DirectX::XMLoadFloat3(&camera->position);
	DirectX::FXMVECTOR front = DirectX::XMLoadFloat3(&camera->front);
	DirectX::FXMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
	DirectX::FXMVECTOR focalPoint = DirectX::XMVectorAdd(position, front);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(position, focalPoint, up);
	DirectX::XMStoreFloat4x4(&constantBuffer->view, DirectX::XMMatrixTranspose(view));	
}

inline void
ScaleUniform(Transform* transform, float scale)
{
	transform->scale.x = scale;
	transform->scale.y = scale;
	transform->scale.z = scale;
}

internal_link void
CreateSprite(ID3D11Device* device, Sprite* sprite)
{	
	sprite->transform.position = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	sprite->transform.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	sprite->transform.rotation = DirectX::XMFLOAT3(0.f, 0.f, 0.f);

	GenerateTexture(device, &sprite->texture, "WoodContainer.jpg", "AwesomeFace.png");

	// TODO: Probably need a better way to check validity?
	if (sprite->texture.source)
	{
		sprite->transform.position = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
		sprite->transform.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
		sprite->transform.rotation = DirectX::XMFLOAT3(0.f, 0.f, 0.f);

		float width = (float)sprite->texture.width;
		float height = (float)sprite->texture.height;
		float halfWidth = 0.5f*width;
		float halfHeight = 0.5f*height;

		VertexPositionColor verticies[] =
		{
			{DirectX::XMFLOAT3(halfWidth, halfHeight, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},
			{DirectX::XMFLOAT3(halfWidth, -halfHeight, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
			{DirectX::XMFLOAT3(-halfWidth, -halfHeight, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
			{DirectX::XMFLOAT3(-halfWidth, halfHeight, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},
		};

		unsigned short indices[] = 
		{
			0,1,2,
			0,2,3,
		};

		CreateVertexBuffer(device, verticies, ARRAYSIZE(verticies), &sprite->renderData);
		CreateIndexBuffer(device, indices, ARRAYSIZE(indices), &sprite->renderData);
		LoadShaders(device, &sprite->renderData, "BasicVertexShader.cso", "BasicTexturePixelShader.cso");
	}
}

internal_link void
CreateTexturedCube(ID3D11Device* device, Cube* cube, float size)
{
	VertexPositionColor verticies[] =
	{
		// -X face
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},

		// +X face
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},
		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},

		// -Y face
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},

		// +Y face
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},

		// -Z face
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},

		// +Z face
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},
	};

	unsigned short indices[] =
	{
		0,1,2, 2,1,3,       // -X
		4,5,6, 6,5,7,       // +X
		8,9,10, 10,9,11,    // -Y
		12,13,14, 14,13,15, // +Y
		16,17,18, 18,17,19, // -Z
		21,20,22, 22,21,23, // +Z
	};

	CreateVertexBuffer(device, verticies, ARRAYSIZE(verticies), &cube->renderData);
	CreateIndexBuffer(device, indices, ARRAYSIZE(indices), &cube->renderData);
	LoadShaders(device, &cube->renderData, "BasicVertexShader.cso", "BasicTexturePixelShader.cso");
	GenerateTexture(device, &cube->texture, "WoodContainer.jpg", "AwesomeFace.png");
}

#if 0
//
// TODO: Microsoft docs reccomend switching the IDXGISwapChain1 for more modern fullscreen behavior
// This is the base setup, but I need to implement all of the things for IDXGISwapChain1
//
internal_link HRESULT
D3DCreateSwapChain2(D3DResource* d3dResource, HWND window)
{
	DXGI_SWAP_CHAIN_DESC1 dxgiDescription;
	ZeroMemory(&dxgiDescription, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiDescription.Width = 0;
	dxgiDescription.Height = 0;
	dxgiDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	dxgiDescription.Stereo = TRUE;
	dxgiDescription.SampleDesc.Count = 1;
	dxgiDescription.SampleDesc.Quality = 0;
	dxgiDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiDescription.BufferCount = 2;
	dxgiDescription.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
	dxgiDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	dxgiDescription.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiFullScreenDescription;
	ZeroMemory(&dxgiFullScreenDescription, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiFullScreenDescription.RefreshRate.Numerator = 0;
	dxgiFullScreenDescription.RefreshRate.Denominator = 1;
	dxgiFullScreenDescription.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiFullScreenDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiFullScreenDescription.Windowed = FALSE;

	IDXGIDevice* dxgiDevice = {0};
	HRESULT result = d3dResource->device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
	ASSERT_OK(result, "Cannot create dxgiDevice");

	//
	// NOTE: Get the adapter
	//
	IDXGIAdapter* dxgiAdapter = {0};
	result = dxgiDevice->GetAdapter(&dxgiAdapter);
	ASSERT_OK(result, "Cannot create dxgiAdapter");

	//
	// NOTE: Create the swap chain
	//
	IDXGIFactory2* dxgiFactory = {0};
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	result = dxgiFactory->CreateSwapChainForHwnd(
		d3dResource->device,
		window,
		&dxgiDescription,
		&dxgiFullScreenDescription,
		NULL, 
		&d3dResource->dxgiSwapChain1
	);
	ASSERT_OK(result, "Cannot create dxgiSwapChain");

	D3DConfigureBackBuffer(d3dResource);

	return result;
}
#endif
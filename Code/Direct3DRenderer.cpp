#include "Direct3DRenderer.h"

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

internal_link void
D3DReleaseBackBuffer(D3DResource* d3dResource)
{
	d3dResource->renderTarget->Release();
	d3dResource->backBuffer->Release();
	d3dResource->depthStencilView->Release();
	d3dResource->depthStencil->Release();
	d3dResource->context->Flush();
}

internal_link void
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
	}
}

internal_link void
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
	}
}

internal_link void
D3DRenderAndPresent(D3DResource* d3dResource, Vector4 color)
{
	ID3D11DeviceContext* context = d3dResource->context;
	ID3D11RenderTargetView* renderTarget = d3dResource->renderTarget;
	ID3D11DepthStencilView* depthStencilView = d3dResource->depthStencilView;

	context->ClearRenderTargetView(renderTarget, color.e);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencilView);

	//
	// TODO: Index and vertex buffers, and shaders here
	//

	// NOTE: Presenting with vsync enabled
	d3dResource->dxgiSwapChain->Present(1, 0);
}

internal_link void
D3DReleaseAll(D3DResource* d3dResource)
{
	d3dResource->dxgiSwapChain->SetFullscreenState(FALSE, NULL);
	d3dResource->context->Flush();
	d3dResource->renderTarget->Release();
	d3dResource->depthStencilView->Release();
	d3dResource->depthStencil->Release();
	d3dResource->backBuffer->Release();
	d3dResource->context->Release();
	d3dResource->device->Release();
	d3dResource->dxgiSwapChain->Release();
}
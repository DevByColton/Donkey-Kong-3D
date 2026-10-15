#include "DirectX.h"

internal_link HRESULT
D3DInitialize(D3DResource* d3dResource, HWND window)
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

	//
	// NOTE: Create the device and context
	//
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

    if (SUCCEEDED(result))
    {
    	//
    	// NOTE: Create the swap chain
    	//
    	ZeroMemory(&d3dResource->dxgiDescription, sizeof(DXGI_SWAP_CHAIN_DESC));
    	d3dResource->dxgiDescription.Windowed = TRUE;
    	d3dResource->dxgiDescription.BufferCount = 2;
    	d3dResource->dxgiDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    	d3dResource->dxgiDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    	d3dResource->dxgiDescription.SampleDesc.Count = 1;
    	d3dResource->dxgiDescription.SampleDesc.Quality = 0;
    	d3dResource->dxgiDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    	d3dResource->dxgiDescription.OutputWindow = window;

    	result = d3dResource->device->QueryInterface(__uuidof(IDXGIDevice3), (void **)&d3dResource->dxgiDevice);

    	if (SUCCEEDED(result))
    	{
    		result = d3dResource->dxgiDevice->GetAdapter(&d3dResource->dxgiAdapter);

    		if (SUCCEEDED(result))
    		{
    			d3dResource->dxgiAdapter->GetParent(IID_PPV_ARGS(&d3dResource->dxgiFactory));
    			result = d3dResource->dxgiFactory->CreateSwapChain(d3dResource->device, &d3dResource->dxgiDescription, &d3dResource->dxgiSwapChain);

    			if (SUCCEEDED(result))
    			{
    				result = d3dResource->dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dResource->backBuffer);

    				if (SUCCEEDED(result))
    				{
    					result = d3dResource->device->CreateRenderTargetView(d3dResource->backBuffer, 0, (ID3D11RenderTargetView**)&d3dResource->renderTarget);

    					if (SUCCEEDED(result))
    					{
    						d3dResource->backBuffer->GetDesc(&d3dResource->backBufferDescription);

    						CD3D11_TEXTURE2D_DESC depthStencil(
    						    DXGI_FORMAT_D24_UNORM_S8_UINT,
    						    (UINT)d3dResource->backBufferDescription.Width,
    						    (UINT)d3dResource->backBufferDescription.Height,
    						    1, // This depth stencil view has only one texture.
    						    1, // Use a single mipmap level.
    						    D3D11_BIND_DEPTH_STENCIL
    						);

    						result = d3dResource->device->CreateTexture2D(&depthStencil, 0, &d3dResource->depthStencil);

    						if (SUCCEEDED(result))
    						{
    							CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilView(D3D11_DSV_DIMENSION_TEXTURE2D);

    							result = d3dResource->device->CreateDepthStencilView(d3dResource->depthStencil, &depthStencilView, &d3dResource->depthStencilView);

    							if (SUCCEEDED(result))
    							{
    								ZeroMemory(&d3dResource->viewport, sizeof(D3D11_VIEWPORT));
    								d3dResource->viewport.Height = (float)d3dResource->backBufferDescription.Height;
    								d3dResource->viewport.Width = (float)d3dResource->backBufferDescription.Width;
    								d3dResource->viewport.MinDepth = 0;
    								d3dResource->viewport.MaxDepth = 1;

    								d3dResource->context->RSSetViewports(1, &d3dResource->viewport);
    							}
    							else
    							{
    								OutputDebugStringA("Unable to create depth stencil view\n");
    							}
    						}
    						else
    						{
    							OutputDebugStringA("Unable to create depth stencil\n");
    						}
    					}
    					else
    					{
    						OutputDebugStringA("Unable to create render target\n");
    					}
    				}
    				else
    				{
    					OutputDebugStringA("Unable to create back buffer\n");
    				}
    			}
    			else
    			{
    				OutputDebugStringA("Unable to create swap chain\n");
    			}
    		}
    		else
    		{
    			OutputDebugStringA("Unable to aquire dxgiAdapter\n");
    		}
    	}
    	else
    	{
    		OutputDebugStringA("Unable to QueryInterface for IDXGIDevice3\n");
    	}
    }

	return result;
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

	d3dResource->dxgiSwapChain->Present(1, 0);
}
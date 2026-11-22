#ifndef LIGHT_H
#define LIGHT_H

struct LightData
{
	DirectX::XMFLOAT4 lightAmbient = {};
	DirectX::XMFLOAT4 lightColor = {};
    DirectX::XMFLOAT4 lightPosition = {};
    DirectX::XMFLOAT4 viewPosition = {};
};
static_assert((sizeof(LightData) % 16) == 0, "LightData size must be 16-byte aligned");

struct Light
{
    ID3D11Buffer* buffer = {0};
    LightData shaderData = {};
    RenderData renderData = {};
    Transform transform = {};
};

internal_link Light
CreateCubeLight(ID3D11Device* device, Shaders* shaders, float size)
{
	Light light = {};
	
	light.transform = {DirectX::XMFLOAT3(0.f, 2.5f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)};
	light.shaderData.lightAmbient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	light.shaderData.lightColor = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	light.shaderData.lightPosition = DirectX::XMFLOAT4(light.transform.position.x, light.transform.position.y, light.transform.position.z, 1.f);

	BaseVertex verticies[] =
	{
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},

		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(1.f, 1.f, 1.f)},
	};

	unsigned short indices[] =
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

	CreateVertexBuffer(device, verticies, sizeof(BaseVertex), ARRAYSIZE(verticies), &light.renderData);
	CreateIndexBuffer(device, indices, ARRAYSIZE(indices), &light.renderData);
	CreateConstantBuffer(device, &light.renderData.modelViewProjectionBuffer, sizeof(ModelViewProjectionData));
	CreateConstantBuffer(device, &light.buffer, sizeof(LightData));

	light.renderData.stride = sizeof(BaseVertex);
	light.renderData.offset = 0;
	light.renderData.vertexShader = shaders->baseVertexShader.shader;
	light.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	light.renderData.pixelShader = shaders->basePixelShader;

	return light;
}

internal_link void
UpdateRenderLight(Light* light, Camera* camera, D3DResource* d3dResource, GameState* gameState)
{
	//
	// NOTE: Update
	//
	light->transform.position.z = 4.f*DirectX::XMScalarCos(DirectX::XMConvertToRadians(50.f*gameState->totalTime));
	light->transform.position.y = 1.2f + (5.f*(0.5f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(50.f*gameState->totalTime)) + 0.5f));
	
	ApplyScaleRotationTranslation(&light->transform, &light->renderData.modelViewProjectionData);
	ApplyView(&light->renderData.modelViewProjectionData, camera);
	PerspectiveProjection(&light->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);

	DirectX::XMVECTOR lightPosition = DirectX::XMLoadFloat3(&light->transform.position);
	DirectX::XMStoreFloat4(&light->shaderData.lightPosition, lightPosition);

	DirectX::XMVECTOR viewPosition = DirectX::XMLoadFloat3(&camera->position);
	DirectX::XMStoreFloat4(&light->shaderData.viewPosition, viewPosition);
	
	//
	// NOTE: Render
	//
	d3dResource->context->UpdateSubresource(light->buffer, 0, 0, &light->shaderData, 0, 0);
    d3dResource->context->PSSetConstantBuffers(0, 1, &light->buffer);
	D3DRender(d3dResource->context, &light->renderData);
}

#endif // LIGHT_H
#ifndef LIGHT_H
#define LIGHT_H

struct LightData
{
	DirectX::XMFLOAT4 lightAmbient = {};
	DirectX::XMFLOAT4 lightColor = {};
    DirectX::XMFLOAT4 lightPosition = {};
    DirectX::XMFLOAT4 pointLightAttenuation = {};

    DirectX::XMFLOAT4 directionalLightAmbient = {};
	DirectX::XMFLOAT4 directionalLightColor = {};
    DirectX::XMFLOAT4 directionalLightDirection = {};

    DirectX::XMFLOAT4 spotLightPosition = {};
    DirectX::XMFLOAT4 spotLightDirection = {};
    DirectX::XMFLOAT4 spotLightCutoff = {};
    DirectX::XMFLOAT4 spotLightAttenuation = {};

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

struct DirectionalLightData
{
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 color = {};
    DirectX::XMFLOAT4 direction = {};
};

struct PointLightData
{
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 color = {};
    DirectX::XMFLOAT4 position = {};
    DirectX::XMFLOAT4 attenuation = {};
};

struct PointLight
{
	ID3D11Buffer* buffer = {0};
	PointLightData data = {};
	RenderData renderData = {};
	Transform transform = {};
};

struct SpotLightData
{
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 color = {};
	DirectX::XMFLOAT4 position = {};
	DirectX::XMFLOAT4 direction = {};
	DirectX::XMFLOAT4 cutoff = {};
	DirectX::XMFLOAT4 attenuation = {};
};

struct SpotLight
{
	ID3D11Buffer* buffer = {0};
	SpotLightData data = {};
	RenderData renderData = {};
	Transform transform = {};
};

#define POINT_LIGHT_COUNT 4
#define SPOT_LIGHT_COUNT 4

struct LightData2
{
	DirectX::XMFLOAT4 viewPosition = {};
	DirectionalLightData directionalLightData = {};
	PointLightData pointLights[POINT_LIGHT_COUNT];
	SpotLightData spotLights[SPOT_LIGHT_COUNT];
};
static_assert((sizeof(LightData2) % 16) == 0, "LightData2 size must be 16-byte aligned");

struct Lights
{
	DirectionalLightData directionalLightData = {};
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight spotLights[SPOT_LIGHT_COUNT];
};

internal_link DirectionalLightData
CreateDirectionalLight(DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 color, DirectX::XMFLOAT4 direction)
{
	DirectionalLightData data = {};

	data.ambient = ambient;
	data.color = color;
	data.direction = direction;

	return data;
}

internal_link PointLight 
CreatePointLight(ID3D11Device* device, Shaders* shaders, float size, 
				 DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 color, DirectX::XMFLOAT4 position, DirectX::XMFLOAT4 attenuation)
{
	PointLight light = {};

	light.data.ambient = ambient;
	light.data.color = color;
	light.data.position = position;
	light.data.attenuation = attenuation;

	BaseVertex verticies[] =
	{
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},

		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
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

internal_link SpotLight
CreateSpotLight(ID3D11Device* device, Shaders* shaders, float size, 
				DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 color, DirectX::XMFLOAT4 position, DirectX::XMFLOAT4 direction,
				DirectX::XMFLOAT4 cutoff, DirectX::XMFLOAT4 attenuation)
{
	SpotLight light = {};

	light.data.ambient = ambient;
	light.data.color = color;
	light.data.position = position;
	light.data.direction = direction;
	light.data.cutoff = cutoff;
	light.data.attenuation = attenuation;

	BaseVertex verticies[] =
	{
		{DirectX::XMFLOAT3(-size, -size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(-size, -size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(-size, size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(-size, size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},

		{DirectX::XMFLOAT3(size, -size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(size, -size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(size, size, -size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
		{DirectX::XMFLOAT3(size, size, size), DirectX::XMFLOAT3(color.x, color.y, color.z)},
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

internal_link Light
CreateCubeLight(ID3D11Device* device, Shaders* shaders, float size)
{
	Light light = {};
	
	light.transform = {DirectX::XMFLOAT3(0.f, 2.5f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, 0.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f)};
	light.shaderData.lightAmbient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	light.shaderData.lightColor = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	light.shaderData.lightPosition = DirectX::XMFLOAT4(light.transform.position.x, light.transform.position.y, light.transform.position.z, 1.f);

	light.shaderData.directionalLightDirection = DirectX::XMFLOAT4(0.2f, -1.f, -0.3f, 0.f);

	light.shaderData.pointLightAttenuation.x = 1.f;
	light.shaderData.pointLightAttenuation.y = 0.14f;
	light.shaderData.pointLightAttenuation.z = 0.07f;

	light.shaderData.spotLightAttenuation.x = 1.f;
	light.shaderData.spotLightAttenuation.y = 0.045f;
	light.shaderData.spotLightAttenuation.z = 0.0075f;

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

	DirectX::XMVECTOR cameraDirection = DirectX::XMLoadFloat3(&camera->front);
	DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&camera->position);
	DirectX::XMStoreFloat4(&light->shaderData.viewPosition, cameraPosition);
	DirectX::XMStoreFloat4(&light->shaderData.spotLightPosition, cameraPosition);
	DirectX::XMStoreFloat4(&light->shaderData.spotLightDirection, cameraDirection);
	DirectX::XMStoreFloat4(&light->shaderData.spotLightCutoff, DirectX::XMVectorSet(
		DirectX::XMScalarCos(DirectX::XMConvertToRadians(12.f)), 
		DirectX::XMScalarCos(DirectX::XMConvertToRadians(20.f)), 
		0.f, 
		0.f)
	);
	
	//
	// NOTE: Render
	//
	d3dResource->context->UpdateSubresource(light->buffer, 0, 0, &light->shaderData, 0, 0);
    d3dResource->context->PSSetConstantBuffers(0, 1, &light->buffer);
	D3DRender(d3dResource->context, &light->renderData);
}

#endif // LIGHT_H
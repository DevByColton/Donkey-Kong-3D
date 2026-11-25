#ifndef LIGHT_H
#define LIGHT_H

#define POINT_LIGHT_COUNT 4
#define SPOT_LIGHT_COUNT 4

struct DirectionalLightData
{
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 color = {};
    DirectX::XMFLOAT4 direction = {};
};

struct PointLightData
{
	DirectX::XMUINT4 flags = {};
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 color = {};
    DirectX::XMFLOAT4 position = {};
    DirectX::XMFLOAT4 attenuation = {};
};

struct PointLight
{
	bool active = false;
	PointLightData data = {};
	RenderData renderData = {};
	Transform transform = {};
};

struct SpotLightData
{
	DirectX::XMUINT4 flags = {};
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 color = {};
	DirectX::XMFLOAT4 position = {};
	DirectX::XMFLOAT4 direction = {};
	DirectX::XMFLOAT4 cutoff = {};
	DirectX::XMFLOAT4 attenuation = {};
};

struct SpotLight
{
	bool active = false;
	SpotLightData data = {};
	RenderData renderData = {};
	Transform transform = {};
};

struct LightData
{
	DirectX::XMFLOAT4 viewPosition = {};
	DirectionalLightData directionalLightData = {};
	PointLightData pointLights[POINT_LIGHT_COUNT] {};
	SpotLightData spotLights[SPOT_LIGHT_COUNT] {};
};
static_assert((sizeof(LightData) % 16) == 0, "LightData size must be 16-byte aligned");

struct Lights
{
	ID3D11Buffer* lightDataBuffer = {0};
	LightData shaderData = {};

	DirectionalLightData directionalLightData = {};
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight spotLights[SPOT_LIGHT_COUNT];
};

internal_link Lights
InitializeLights(ID3D11Device* device)
{
	Lights lights = {};

	CreateConstantBuffer(device, &lights.lightDataBuffer, sizeof(LightData));

	return lights;
}

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

	light.active = true;
	light.transform.position = DirectX::XMFLOAT3(position.x, position.y, position.z);
	light.transform.rotation = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	light.transform.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
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

	light.active = true;
	light.transform.position = DirectX::XMFLOAT3(position.x, position.y, position.z);
	light.transform.rotation = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	light.transform.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
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

	light.renderData.stride = sizeof(BaseVertex);
	light.renderData.offset = 0;
	light.renderData.vertexShader = shaders->baseVertexShader.shader;
	light.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	light.renderData.pixelShader = shaders->basePixelShader;

	return light;
}

internal_link void
UpdateRenderLights(Lights* lights, Camera* camera, D3DResource* d3dResource, GameState* gameState)
{
	DirectX::XMStoreFloat4(&lights->shaderData.viewPosition, DirectX::XMVectorSet(camera->position.x, camera->position.y, camera->position.z, 1.f));

	//
	// NOTE: Update directional light
	//
	lights->shaderData.directionalLightData = lights->directionalLightData;

	//
	// NOTE: Update render point lights
	//
	for (unsigned int pointLightIndex = 0; pointLightIndex < POINT_LIGHT_COUNT; ++pointLightIndex)
	{
		PointLight* pointLight = lights->pointLights + pointLightIndex;

		if (pointLight->active)
		{
			pointLight->transform.position.z = 4.f*DirectX::XMScalarCos(DirectX::XMConvertToRadians(50.f*gameState->totalTime));
			pointLight->transform.position.y = 1.2f + (5.f*(0.5f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(50.f*gameState->totalTime)) + 0.5f));

			lights->shaderData.pointLights[pointLightIndex].flags = DirectX::XMUINT4{1u, 0u, 0u, 0u};
			ApplyScaleRotationTranslation(&pointLight->transform, &pointLight->renderData.modelViewProjectionData);
			ApplyView(&pointLight->renderData.modelViewProjectionData, camera);
			PerspectiveProjection(&pointLight->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);

			DirectX::XMStoreFloat4(
				&lights->shaderData.pointLights[pointLightIndex].position, 
				DirectX::XMVectorSet(pointLight->transform.position.x, pointLight->transform.position.y, pointLight->transform.position.z, 1.f)
			);
			DirectX::XMStoreFloat4(&lights->shaderData.pointLights[pointLightIndex].ambient, DirectX::XMLoadFloat4(&pointLight->data.ambient));
			DirectX::XMStoreFloat4(&lights->shaderData.pointLights[pointLightIndex].color, DirectX::XMLoadFloat4(&pointLight->data.color));
			DirectX::XMStoreFloat4(&lights->shaderData.pointLights[pointLightIndex].attenuation, DirectX::XMLoadFloat4(&pointLight->data.attenuation));

			D3DRender(d3dResource->context, &pointLight->renderData);
		}
		else
		{
			lights->shaderData.pointLights[pointLightIndex].flags = DirectX::XMUINT4{0u, 0u, 0u, 0u};
		}
	}

	//
	// NOTE: Update render spot lights
	//
	for (unsigned int spotLightIndex = 0; spotLightIndex < SPOT_LIGHT_COUNT; ++spotLightIndex)
	{
		SpotLight* spotLight = lights->spotLights + spotLightIndex;

		if (spotLight->active)
		{
			ApplyScaleRotationTranslation(&spotLight->transform, &spotLight->renderData.modelViewProjectionData);
			ApplyView(&spotLight->renderData.modelViewProjectionData, camera);
			PerspectiveProjection(&spotLight->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);

			lights->shaderData.spotLights[spotLightIndex].flags = DirectX::XMUINT4{1u, 0u, 0u, 0u};
			DirectX::XMStoreFloat4(
				&lights->shaderData.spotLights[spotLightIndex].position, 
				DirectX::XMVectorSet(spotLight->transform.position.x, spotLight->transform.position.y, spotLight->transform.position.z, 1.f)
			);
			DirectX::XMStoreFloat4(&lights->shaderData.spotLights[spotLightIndex].ambient, DirectX::XMLoadFloat4(&spotLight->data.ambient));
			DirectX::XMStoreFloat4(&lights->shaderData.spotLights[spotLightIndex].color, DirectX::XMLoadFloat4(&spotLight->data.color));
			DirectX::XMStoreFloat4(&lights->shaderData.spotLights[spotLightIndex].direction, DirectX::XMLoadFloat4(&spotLight->data.direction));
			DirectX::XMStoreFloat4(&lights->shaderData.spotLights[spotLightIndex].cutoff, DirectX::XMLoadFloat4(&spotLight->data.cutoff));
			DirectX::XMStoreFloat4(&lights->shaderData.spotLights[spotLightIndex].attenuation, DirectX::XMLoadFloat4(&spotLight->data.attenuation));

			D3DRender(d3dResource->context, &spotLight->renderData);
		}
		else
		{
			lights->shaderData.spotLights[spotLightIndex].flags = DirectX::XMUINT4{0u, 0u, 0u, 0u};
		}
	}

	//
	// NOTE: Update lighting constant buffer
	//
	d3dResource->context->UpdateSubresource(lights->lightDataBuffer, 0, 0, &lights->shaderData, 0, 0);
	d3dResource->context->PSSetConstantBuffers(0, 1, &lights->lightDataBuffer);
}

#endif // LIGHT_H
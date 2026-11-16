#ifndef LIGHT_H
#define LIGHT_H

struct LightData
{
    DirectX::XMFLOAT4 ambientStrength = {};
    DirectX::XMFLOAT4 lightColor = {};
    DirectX::XMFLOAT4 lightPosition = {};
    DirectX::XMFLOAT4 viewPosition = {};
};
static_assert((sizeof(LightData) % 16) == 0, "LightData size must be 16-byte aligned");

struct Light
{
    ID3D11Buffer* buffer = {0};
    LightData shaderData = {};
    MeshData* mesh = {};
    Transform transform = {};
};

internal_link Light
CreateLight(ID3D11Device* device, Shaders* shaders)
{
	Light light = {};

	light.transform = {DirectX::XMFLOAT3(0.f, 2.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, 0.f), DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f)};
	light.shaderData.ambientStrength = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	light.shaderData.lightColor = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	light.shaderData.lightPosition = DirectX::XMFLOAT4(light.transform.position.x, light.transform.position.y, light.transform.position.z, 1.f);
	CreateConstantBuffer(device, &light.buffer, sizeof(LightData));

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
	
	ApplyScaleRotationTranslation(&light->transform, &light->mesh->renderData.modelViewProjectionData);
	ApplyView(&light->mesh->renderData.modelViewProjectionData, camera);
	PerspectiveProjection(&light->mesh->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);

	DirectX::XMVECTOR lightPosition = DirectX::XMLoadFloat3(&light->transform.position);
	DirectX::XMStoreFloat4(&light->shaderData.lightPosition, lightPosition);

	DirectX::XMVECTOR viewPosition = DirectX::XMLoadFloat3(&camera->position);
	DirectX::XMStoreFloat4(&light->shaderData.viewPosition, viewPosition);
	
	//
	// NOTE: Render
	//
	d3dResource->context->UpdateSubresource(light->buffer, 0, 0, &light->shaderData, 0, 0);
    d3dResource->context->PSSetConstantBuffers(0, 1, &light->buffer);
	D3DRender(d3dResource->context, &light->mesh->renderData);
}

#endif // LIGHT_H
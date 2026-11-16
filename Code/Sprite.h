#ifndef SPRITE_H
#define SPRITE_H

struct Sprite
{
    RenderData renderData = {};
    Texture2D texture = {};
    Transform transform = {};
};

//
// TODO: Make this generic sprite creation
//
internal_link Sprite
CreateSprite(ID3D11Device* device, Shaders* shaders)
{	
	Sprite sprite = {};

	Texture2D texture = GenerateTexture(device, 0, "WoodContainer.jpg", "AwesomeFace.png");

	// TODO: Probably need a better way to check validity?
	if (texture.source)
	{
		sprite.texture = texture;

		sprite.transform.position = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
		sprite.transform.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
		sprite.transform.rotation = DirectX::XMFLOAT3(0.f, 0.f, 0.f);

		float width = (float)sprite.texture.width;
		float height = (float)sprite.texture.height;
		float halfWidth = 0.5f*width;
		float halfHeight = 0.5f*height;

		Vertex verticies[] =
		{
			{DirectX::XMFLOAT3(halfWidth, halfHeight, 1.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 0.f)},
			{DirectX::XMFLOAT3(halfWidth, -halfHeight, 1.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(1.f, 1.f)},
			{DirectX::XMFLOAT3(-halfWidth, -halfHeight, 1.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 1.f)},
			{DirectX::XMFLOAT3(-halfWidth, halfHeight, 1.f), DirectX::XMFLOAT3(1.f, 1.f, 1.f), DirectX::XMFLOAT2(0.f, 0.f)},
		};

		unsigned short indices[] = 
		{
			0,1,2,
			0,2,3,
		};

		CreateVertexBuffer(device, verticies, ARRAYSIZE(verticies), &sprite.renderData);
		CreateIndexBuffer(device, indices, ARRAYSIZE(indices), &sprite.renderData);
		CreateConstantBuffer(device, &sprite.renderData.modelViewProjectionBuffer, sizeof(ModelViewProjectionData));

		sprite.renderData.vertexShader = shaders->baseVertexShader.shader;
		sprite.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
		sprite.renderData.pixelShader = shaders->baseTexturePixelShader;
	}

	return sprite;
}

internal_link void
UpdateRenderSprite(Sprite* sprite, D3DResource* d3dResource)
{
	ApplyScaleRotationTranslation(&sprite->transform, &sprite->renderData.modelViewProjectionData);
	ApplyView(&sprite->renderData.modelViewProjectionData);
	OrthographicProjection(&sprite->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);
	D3DRender(d3dResource->context, &sprite->renderData, &sprite->texture);
}

#endif // SPRITE_H
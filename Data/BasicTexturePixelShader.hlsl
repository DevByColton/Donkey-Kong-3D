Texture2DArray textures : register(t0);
SamplerState samplerState : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 texture1Color = textures.Sample(samplerState, float3(input.texCoord, 0.f));
    float4 texture2Color = textures.Sample(samplerState, float3(input.texCoord, 1.f));
    
    float4 blended = lerp(texture1Color, texture2Color, 0.2f);
    blended.a = 1.f;

    return blended;
}
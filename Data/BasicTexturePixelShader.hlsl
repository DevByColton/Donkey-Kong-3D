Texture2D targetTexture : register(t0);
SamplerState samplerState : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return targetTexture.Sample(samplerState, input.texCoord);
}
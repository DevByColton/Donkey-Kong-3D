Texture2DArray textures : register(t0);
SamplerState samplerState : register(s0);

cbuffer LightData : register(b0)
{
    float4 ambientStrength;
    float4 lightColor;
    float4 lightPosition;
    float4 worldLightPosition;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : WORLDPOS;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 texture1Color = textures.Sample(samplerState, float3(input.texCoord, 0.f));
    float4 texture2Color = textures.Sample(samplerState, float3(input.texCoord, 1.f));
    
    float4 blended = lerp(texture1Color, texture2Color, 0.2f);
    blended.a = 1.f;

    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition);
    float diffuseImpact = max(dot(normal, lightDirection), 0.f);
    float4 diffuseColor = diffuseImpact * lightColor;
    float4 ambientColor = ambientStrength * lightColor;
    
    float4 result = (ambientColor + diffuseColor) * (blended * input.color);
    result.a = 1.f;
    return result;
}
cbuffer LightData : register(b0)
{
    float4 ambientStrength;
    float4 lightColor;
    float4 lightPosition;
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
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition);
    
    float diffuse = max(dot(normal, lightDirection), 0.f);
    float3 diffuseColor = diffuse * lightColor.xyz;
    float3 ambientColor = ambientStrength.xyz * lightColor.xyz;

    float3 result = (ambientColor + diffuseColor) * input.color.xyz;
    return float4(result, 1.f);
}
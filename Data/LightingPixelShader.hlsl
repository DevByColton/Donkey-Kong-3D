cbuffer LightData : register(b0)
{
    float4 lightAmbient;
    float4 lightColor;
    float4 lightPosition;
    float4 viewPosition;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : WORLDPOS;
    float roughness : ROUGHNESS;
};

float4 main(PixelShaderInput input) : SV_TARGET
{  
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition);
    
    float diffuse = max(dot(normal, lightDirection), 0.f);

    float specularStrength = (1.f - input.roughness);
    float shininess = 64.f * (1.f - input.roughness);
    float3 viewDirection = normalize(viewPosition.xyz - input.worldPosition);
    float3 reflectDirection = reflect(-lightDirection, normal);
    float specular = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);

    float3 ambientColor = lightAmbient.xyz * lightColor.xyz;
    float3 diffuseColor = diffuse * lightColor.xyz;
    float3 specularColor = specularStrength * specular * lightColor.xyz;

    float3 result = input.color.xyz * (ambientColor + diffuseColor) + specularColor;
    return float4(result, 1.f);   
}
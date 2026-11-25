cbuffer LightData : register(b0)
{
    float4 lightAmbient;
    float4 lightColor;
    float4 lightPosition;
    float4 pointLightAttenuation;

    float4 directionalLightAmbient;
    float4 directionalLightColor;
    float4 directionalLightDirection;

    float4 spotLightPosition;
    float4 spotLightDirection;
    float4 spotLightCutoff;
    float4 spotLightAttenuation;

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
#if 0 // Point light
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition);

    float distance = length(lightPosition.xyz - input.worldPosition);
    float attenuation = 1.f / (pointLightAttenuation.x + pointLightAttenuation.y*distance + pointLightAttenuation.z*(distance*distance));
    
    float diffuse = max(dot(normal, lightDirection), 0.f);

    float specularStrength = (1.f - input.roughness);
    float shininess = 64.f * (1.f - input.roughness);
    float3 viewDirection = normalize(viewPosition.xyz - input.worldPosition);
    float3 reflectDirection = reflect(-lightDirection, normal);
    float specular = (diffuse > 0.f) ? pow(max(dot(viewDirection, reflectDirection), 0.0), shininess) : 0.f;

    float3 ambientColor = lightAmbient.xyz;;
    float3 diffuseColor = diffuse * lightColor.xyz;
    float3 specularColor = specularStrength * specular * lightColor.xyz;

    ambientColor *= attenuation;
    diffuseColor *= attenuation;
    specularColor *= attenuation;

    float3 result = input.color.xyz * (ambientColor + diffuseColor) + specularColor;
    return float4(result, 1.f);
#endif

#if 1 // Spot light
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(spotLightPosition.xyz - input.worldPosition);
    float distance = length(spotLightPosition.xyz - input.worldPosition);
    float attenuation = 1.f / (spotLightAttenuation.x + spotLightAttenuation.y*distance + spotLightAttenuation.z*(distance*distance));

    float innerCutoff = spotLightCutoff.x;
    float outerCutoff = spotLightCutoff.y;
    float theta = dot(lightDirection, normalize(-spotLightDirection.xyz));
    float epsilon = innerCutoff - outerCutoff;
    float intensity = clamp((theta - outerCutoff) / epsilon, 0.f, 1.f);

    float diffuse = max(dot(normal, lightDirection), 0.f);

    float specularStrength = (1.f - input.roughness);
    float shininess = 64.f * (1.f - input.roughness);
    float3 viewDirection = normalize(viewPosition.xyz - input.worldPosition);
    float3 reflectDirection = reflect(-lightDirection, normal);
    float specular = (diffuse > 0.f) ? pow(max(dot(viewDirection, reflectDirection), 0.0), shininess) : 0.f;

    float3 ambientColor = lightAmbient.xyz;
    float3 diffuseColor = diffuse * lightColor.xyz;
    float3 specularColor = specularStrength * specular * lightColor.xyz;

    ambientColor *= attenuation;
    
    diffuseColor *= attenuation;
    diffuseColor *= intensity;
    
    specularColor *= attenuation;
    specularColor *= intensity;

    float3 result = input.color.xyz * (ambientColor + diffuseColor) + specularColor;
    return float4(result, 1.f);
#endif      
}
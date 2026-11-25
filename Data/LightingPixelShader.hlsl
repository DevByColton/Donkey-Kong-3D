#define POINT_LIGHT_COUNT 4
#define SPOT_LIGHT_COUNT 4

struct ColorResult
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

struct DirectionalLightData
{
    float4 ambient;
    float4 color;
    float4 direction;
};

struct PointLightData
{
    uint4 flags;
    float4 ambient;
    float4 color;
    float4 position;
    float4 attenuation;
};

struct SpotLightData
{
    uint4 flags;
    float4 ambient;
    float4 color;
    float4 position;
    float4 direction;
    float4 cutoff;
    float4 attenuation;
};

cbuffer LightData : register(b0)
{
    float4 viewPosition;
    DirectionalLightData directionalLightData;
    PointLightData pointLights[POINT_LIGHT_COUNT];
    SpotLightData spotLights[SPOT_LIGHT_COUNT];
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

float CalculateSpecular(float diffuse, float3 lightDirection, float3 normal, float3 viewDirection, float roughness)
{
    float specularStrength = (1.f - roughness);
    float shininess = 64.f * (1.f - roughness);
    float3 reflectDirection = reflect(-lightDirection, normal);
    float specular = (diffuse > 0.f) ? pow(max(dot(viewDirection, reflectDirection), 0.0), shininess) : 0.f;

    return specularStrength * specular;
}

ColorResult CalculateDirectionalLight(DirectionalLightData directionalLight, float3 normal, float3 viewDirection, float roughness)
{
    float3 lightDirection = normalize(-directionalLight.direction.xyz);

    float diffuse = max(dot(normal, lightDirection), 0.f);
    float specular = CalculateSpecular(diffuse, lightDirection, normal, viewDirection, roughness);

    ColorResult result;
    result.ambient = directionalLight.ambient.xyz;
    result.diffuse = diffuse * directionalLight.color.xyz;
    result.specular = specular * directionalLight.color.xyz;

    return result;
}

ColorResult CalculatePointLight(PointLightData pointLight, float3 normal, float3 viewDirection, float roughness, float3 worldPosition)
{
    float3 lightDirection = normalize(pointLight.position.xyz - worldPosition);

    float distance = length(pointLight.position.xyz - worldPosition);
    float attenuation = 1.f / (pointLight.attenuation.x + pointLight.attenuation.y*distance + pointLight.attenuation.z*(distance*distance));
    
    float diffuse = max(dot(normal, lightDirection), 0.f);
    float specular = CalculateSpecular(diffuse, lightDirection, normal, viewDirection, roughness);

    ColorResult result;
    result.ambient = pointLight.ambient.xyz;
    result.ambient *= attenuation;

    result.diffuse = diffuse * pointLight.color.xyz;
    result.diffuse *= attenuation;

    result.specular = specular * pointLight.color.xyz;
    result.specular *= attenuation;

    return result;
}

ColorResult CalculateSpotLight(SpotLightData spotLight, float3 normal, float3 viewDirection, float roughness, float3 worldPosition)
{
    float3 lightDirection = normalize(spotLight.position.xyz - worldPosition);
    
    float distance = length(spotLight.position.xyz - worldPosition);
    float attenuation = 1.f / (spotLight.attenuation.x + spotLight.attenuation.y*distance + spotLight.attenuation.z*(distance*distance));

    float innerCutoff = spotLight.cutoff.x;
    float outerCutoff = spotLight.cutoff.y;
    float theta = dot(lightDirection, normalize(-spotLight.direction.xyz));
    float epsilon = innerCutoff - outerCutoff;
    float intensity = clamp((theta - outerCutoff) / epsilon, 0.f, 1.f);

    float diffuse = max(dot(normal, lightDirection), 0.f);
    float specular = CalculateSpecular(diffuse, lightDirection, normal, viewDirection, roughness);

    ColorResult result;
    result.ambient = spotLight.ambient.xyz;
    result.ambient *= attenuation;

    result.diffuse = diffuse * spotLight.color.xyz;
    result.diffuse *= attenuation;
    result.diffuse *= intensity;

    result.specular = specular * spotLight.color.xyz;
    result.specular *= attenuation;
    result.specular *= intensity;

    return result;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 viewDirection = normalize(viewPosition.xyz - input.worldPosition);

    ColorResult directionalLightResult = CalculateDirectionalLight(directionalLightData, normal, viewDirection, input.roughness);
    
    ColorResult pointLightsResult;
    pointLightsResult.ambient = 0.f;
    pointLightsResult.diffuse = 0.f;
    pointLightsResult.specular = 0.f;
    for (unsigned int pointLightIndex = 0; pointLightIndex < POINT_LIGHT_COUNT; ++pointLightIndex)
    {
        if (pointLights[pointLightIndex].flags.x)
        {
            ColorResult pointLightCalc = CalculatePointLight(pointLights[pointLightIndex], normal, viewDirection, input.roughness, input.worldPosition);
            pointLightsResult.ambient += pointLightCalc.ambient;
            pointLightsResult.diffuse += pointLightCalc.diffuse;
            pointLightsResult.specular += pointLightCalc.specular;
        }
    }
    
    ColorResult spotLightResult;
    spotLightResult.ambient = 0.f;
    spotLightResult.diffuse = 0.f;
    spotLightResult.specular = 0.f;
    for (unsigned int spotLightIndex = 0; spotLightIndex < SPOT_LIGHT_COUNT; ++spotLightIndex)
    {
        if (spotLights[spotLightIndex].flags.x)
        {
            ColorResult spotLightCalc = CalculateSpotLight(spotLights[spotLightIndex], normal, viewDirection, input.roughness, input.worldPosition);
            spotLightResult.ambient += spotLightCalc.ambient;
            spotLightResult.diffuse += spotLightCalc.diffuse;
            spotLightResult.specular += spotLightCalc.specular;
        }
    }

    ColorResult finalResult;
    finalResult.ambient = (directionalLightResult.ambient + pointLightsResult.ambient + spotLightResult.ambient);
    finalResult.diffuse = (directionalLightResult.diffuse + pointLightsResult.diffuse + spotLightResult.diffuse);
    finalResult.specular = (directionalLightResult.specular + pointLightsResult.specular + spotLightResult.specular);

    float3 result = input.color.xyz * (finalResult.ambient + finalResult.diffuse) + finalResult.specular;
    return float4(result, 1.f);     
}
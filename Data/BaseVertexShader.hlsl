cbuffer ModelViewProjectionData : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : WORLDPOS;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : WORLDPOS;
};

VertexShaderOutput main(VertexShaderInput input)
{
    float4 position = float4(input.position, 1.0f);
    position = mul(position, model);
    position = mul(position, view);
    position = mul(position, projection);
    
    VertexShaderOutput output;
    output.position = position;
    output.color = float4(input.color, 1.0f);
    output.texCoord = input.texCoord;
    output.normal = input.normal;
    output.worldPosition = input.worldPosition;

    // // Save world position for to keep it for the pixel shader
    // float4 worldPosition = mul(float4(input.position, 1.f), model);
    // float4 viewPosition = mul(worldPosition, view);
    // float4 clipPosition = mul(viewPosition, projection);
    
    // VertexShaderOutput output;
    // output.worldPosition = worldPosition.xyz;
    // output.position = clipPosition;
    // output.color = float4(input.color, 1.f);
    // output.texCoord = input.texCoord;
    // output.normal = input.normal;

    return output;
}
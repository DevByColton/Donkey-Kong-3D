cbuffer ModelViewProjectionData : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix normalModel;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
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
    float4 position = float4(input.position, 1.f);
    position = mul(position, model);
    position = mul(position, view);
    position = mul(position, projection);
    
    VertexShaderOutput output;
    output.position = position;
    output.color = float4(input.color, 1.f);
    output.texCoord = input.texCoord;
    output.normal = normalize(mul(input.normal, (float3x3)normalModel));
    output.worldPosition = mul(float4(input.position, 1.f), model).xyz;

    return output;
}
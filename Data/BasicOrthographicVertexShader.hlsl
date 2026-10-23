cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 colorModulation;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 texCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 position = float4(input.position, 1.0f);
    position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);
    output.position = position;

    output.color = float4(input.color, 1.0f);
    output.texCoord = input.texCoord;

    return output;
}
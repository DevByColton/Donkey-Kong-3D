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
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // Just pass through the color and position data
    output.position = float4(input.position, 1.0f);
    output.color = float4(input.color, 1.0f)*colorModulation;

    return output;
}
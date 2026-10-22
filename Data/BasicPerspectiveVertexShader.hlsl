cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix colorModulation;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;  // interpolated vertex position (system value)
    float4 color : COLOR;          // interpolated diffuse color
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 position = float4(input.position, 1.0f);

    // Transform the position from object space to homogeneous projection space
    position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);
    output.position = position;

    // Just pass through the color data
    output.color = float4(input.color, 1.0f);

    return output;
}
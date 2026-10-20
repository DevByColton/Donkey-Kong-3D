struct PixelShaderInput
{
    float4 position : SV_POSITION;  // interpolated vertex position (system value)
    float4 color : COLOR;          // interpolated diffuse color
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return input.color;
}
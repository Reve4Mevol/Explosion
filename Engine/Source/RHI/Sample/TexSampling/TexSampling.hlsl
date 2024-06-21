#include <Platform.h>

VkBinding(0, 0) Texture2D colorTex : register(t0);
VkBinding(1, 0) SamplerState colorSampler : register(s0);
VkBinding(2, 0) cbuffer passParams : register(b0)
{
	float4x4 model;
};

struct FragmentInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

FragmentInput VSMain(
    VkLocation(0) float4 position : POSITION,
    VkLocation(1) float2 uv : TEXCOORD)
{
    FragmentInput fragmentInput;
    fragmentInput.position = mul(model, position);
    fragmentInput.uv = uv;
#if VULKAN
    fragmentInput.uv.y = 1 - fragmentInput.uv.y;
#endif
    return fragmentInput;
}

float4 PSMain(FragmentInput input) : SV_TARGET
{
    return colorTex.Sample(colorSampler, input.uv);
}

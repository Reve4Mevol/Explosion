#include <Platform.h>

VkBinding(0, 0) Texture2D ssaoTex : register(t0);
VkBinding(1, 0) SamplerState ssaoSampler : register(s0);

struct VSOutput
{
	float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(
    VkLocation(0) float4 postion : POSITION,
    VkLocation(1) float2 uv : TEXCOORD)
{
	VSOutput output = (VSOutput)0;
	output.uv = uv;
	output.position = postion;

#if VULKAN
    output.uv.y = 1 - output.uv.y;
#endif

	return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	const int blurRange = 2;
	int n = 0;
	int2 texDim;
	ssaoTex.GetDimensions(texDim.x, texDim.y);
	float2 texelSize = 1.0 / (float2)texDim;
	float result = 0.0;
	for (int x = -blurRange; x < blurRange; x++) {
		for (int y = -blurRange; y < blurRange; y++) {
			float2 offset = float2(float(x), float(y)) * texelSize;
			result += ssaoTex.Sample(ssaoSampler, input.uv + offset).r;
			n++;
		}
	}
	return result / (float(n));
}
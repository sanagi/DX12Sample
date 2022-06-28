#include "SimpleShaderHeader.hlsli"

float4 PSMain(VSOutput vsout) : SV_TARGET
{
	return float4(vsout.UV, 1, 1);
}
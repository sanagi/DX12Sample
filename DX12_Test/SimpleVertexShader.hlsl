#include "SimpleShaderHeader.hlsli"

VSOutput VSMain( float4 pos : POSITION, float2 uv : TEXCOORD )
{
	VSOutput vsout = (VSOutput)0;
	vsout.Position = pos;
	vsout.UV = uv;
	return vsout;
}
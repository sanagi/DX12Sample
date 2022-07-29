#include"PMDToonShaderHeader.hlsli"

BasicType VSMain(
	float4 pos : POSITION, 
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneo : BONENO,
	min16uint weight : WEIGHT
	) {

	float w = weight / 100.0f;
	matrix bm = bones[boneo[0]] * w + bones[boneo[1]] * (1 - w);

	BasicType output;//ピクセルシェーダへ渡す値
	pos = mul(bm, pos);
	pos = mul(world, pos);
	output.svpos = mul(mul(proj, view),pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - eye);
	return output;
}
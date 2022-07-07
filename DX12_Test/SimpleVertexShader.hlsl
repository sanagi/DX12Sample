#include"SimpleShaderHeader.hlsli"
cbuffer cbuff0 : register(b0) {
	matrix world;//ワールド変換行列
	matrix viewproj;//ビュープロジェクション行列
};

BasicType VSMain(
	float4 pos : POSITION, 
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneo : BONE_NO,
	min16uint weight : WEIGHT
	) {
	BasicType output;//ピクセルシェーダへ渡す値
	output.svpos = mul(mul(viewproj, world),pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}
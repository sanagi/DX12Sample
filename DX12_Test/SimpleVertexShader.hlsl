#include"SimpleShaderHeader.hlsli"
cbuffer cbuff0 : register(b0) {
	matrix world;//���[���h�ϊ��s��
	matrix viewproj;//�r���[�v���W�F�N�V�����s��
};

BasicType VSMain(
	float4 pos : POSITION, 
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneo : BONE_NO,
	min16uint weight : WEIGHT
	) {
	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	output.svpos = mul(mul(viewproj, world),pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}
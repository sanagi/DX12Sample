#include"SimpleShaderHeader.hlsli"
cbuffer cbuff0 : register(b0) {
	matrix mat;//�ϊ��s��
};

BasicType VSMain(float4 pos : POSITION, float2 uv : TEXCOORD) {
	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	output.svpos = mul(mat,pos);
	output.uv = uv;
	return output;
}
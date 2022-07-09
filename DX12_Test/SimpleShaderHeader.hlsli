//���_�V�F�[�_���s�N�Z���V�F�[�_�ւ̂����Ɏg�p����
//�\����
struct BasicType {
	float4 svpos:SV_POSITION;//�V�X�e���p���_���W
	float4 normal:NORMAL;//�@���x�N�g��
	float2 uv:TEXCOORD;//UV�l
};

cbuffer cbuff0 : register(b0) {
	matrix world;//���[���h�ϊ��s��
	matrix viewproj;//�r���[�v���W�F�N�V�����s��
};

cbuffer Material : register(b1) {
	float4 diffuse;
	float4 specular;
	float3 ambient;
};

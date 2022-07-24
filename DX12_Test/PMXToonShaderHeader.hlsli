//���_�V�F�[�_���s�N�Z���V�F�[�_�ւ̂����Ɏg�p����
//�\����
struct BasicType {
	float4 svpos:SV_POSITION;//�V�X�e���p���_���W
	float4 pos:POSITION; //���_���W
	float4 normal:NORMAL;//�@���x�N�g��
	float4 vnormal:NORMAL1;//�r���[�ϊ���̖@���x�N�g��
	float2 uv:TEXCOORD;//UV�l
	float3 ray:VECTOR;//���_����̃��C
};

cbuffer cbuff0 : register(b0) {
	matrix world;//���[���h�ϊ��s��
	matrix view;//�r���[�s��
	matrix proj;//�r���[�v���W�F�N�V�����s��
	float3 eye;//���_
};

cbuffer Material : register(b1) {
	float4 diffuse;
	float alpha;
	float4 specular;
	float specularity;
	float3 ambient;
};

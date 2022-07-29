#include"PMDToonShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> sph:register(t1);//1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> spa:register(t2);//2�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)
Texture2D<float4> toon:register(t3);//2�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)
SamplerState smp:register(s0);//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��
SamplerState smpToon:register(s1);//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��

float4 PSMain(BasicType input) : SV_TARGET{
	float3 light = normalize(float3(1,-1,1)); //��

	float3 lightColor = float3(1, 1, 1); //���C�g�F

	//�f�B�t���[�Y
	float diffuseB = dot(-light, input.normal);

	//Toon
	float4 toonDiff = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//���˃x�N�g��
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	//�X�t�B�A�}�b�v�pUV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	//�e�N�X�`���J���[
	float4 texColor = tex.Sample(smp, input.uv);

	float4 diffuseColor = /*toonDiff * diffuse */ texColor; //diffuse
	float4 sphColor = sph.Sample(smp, sphereMapUV); //�X�t�B�A��Z
	float4 spaColor = spa.Sample(smp, sphereMapUV); //�X�t�B�A���Z
	float4 specularColor = float4(min(0, specularB * specular.rgb) * specularity, texColor.a); //�X�؃L����
	float4 ambientColor = float4((texColor * ambient).rgb, texColor.a);

	return diffuseColor * sphColor + spaColor + specularColor + ambientColor;
}
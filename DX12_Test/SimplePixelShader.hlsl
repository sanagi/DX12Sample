#include"SimpleShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> sph:register(t1);//1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> spa:register(t2);//2�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)
SamplerState smp:register(s0);//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��


float4 PSMain(BasicType input) : SV_TARGET{
	//return float4(.Sample(smp,input.uv));
	//return float4(input.normal.xyz,1);
	float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);

	float3 light = normalize(float3(1,-1,1));
	float brightness = dot(-light, input.normal);
	return float4(brightness, brightness, brightness, 1) * diffuse * tex.Sample(smp, input.uv) * sph.Sample(smp, normalUV) + spa.Sample(smp, normalUV);

}
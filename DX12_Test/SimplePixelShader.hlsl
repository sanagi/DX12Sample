#include"SimpleShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> sph:register(t1);//1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> spa:register(t2);//2�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)
SamplerState smp:register(s0);//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��


float4 PSMain(BasicType input) : SV_TARGET{

	return float4(1,0,0,1);
	//return float4(.Sample(smp,input.uv));
	//return float4(input.normal.xyz,1);
	/*float3 light = normalize(float3(1,-1,1)); //��

	float3 lightColor = float3(1, 1, 1); //���C�g�F

	//�f�B�t���[�Y
	float diffuseB = dot(-light, input.normal);

	//���˃x�N�g��
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	//�X�t�B�A�}�b�v�pUV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	//�e�N�X�`���J���[
	float4 texColor = tex.Sample(smp, input.uv);

	return max(diffuseB * diffuse * texColor //diffuse
		* sph.Sample(smp, sphereMapUV) //�X�t�B�A��Z
		+ spa.Sample(smp, sphereMapUV) //�X�t�B�A���Z
		+ float4(specularB * specular.rgb, 1) //�X�؃L����
		, float4(texColor * ambient, 1)); //�A���r�G���g
		*/
		
}
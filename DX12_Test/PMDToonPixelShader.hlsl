#include"PMDToonShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ
Texture2D<float4> sph:register(t1);//1番スロットに設定されたテクスチャ
Texture2D<float4> spa:register(t2);//2番スロットに設定されたテクスチャ(加算)
Texture2D<float4> toon:register(t3);//2番スロットに設定されたテクスチャ(加算)
SamplerState smp:register(s0);//0番スロットに設定されたサンプラ
SamplerState smpToon:register(s1);//0番スロットに設定されたサンプラ

float4 PSMain(BasicType input) : SV_TARGET{
	float3 light = normalize(float3(1,-1,1)); //光

	float3 lightColor = float3(1, 1, 1); //ライト色

	//ディフューズ
	float diffuseB = dot(-light, input.normal);

	//Toon
	float4 toonDiff = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//反射ベクトル
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	//スフィアマップ用UV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	//テクスチャカラー
	float4 texColor = tex.Sample(smp, input.uv);

	float4 diffuseColor = /*toonDiff * diffuse */ texColor; //diffuse
	float4 sphColor = sph.Sample(smp, sphereMapUV); //スフィア乗算
	float4 spaColor = spa.Sample(smp, sphereMapUV); //スフィア加算
	float4 specularColor = float4(min(0, specularB * specular.rgb) * specularity, texColor.a); //スぺキュラ
	float4 ambientColor = float4((texColor * ambient).rgb, texColor.a);

	return diffuseColor * sphColor + spaColor + specularColor + ambientColor;
}
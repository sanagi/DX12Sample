#include"PMXToonShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ
Texture2D<float4> toon:register(t1);//1番スロットに設定されたテクスチャ(トゥーン)
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

	//テクスチャカラー
	float4 texColor = tex.Sample(smp, input.uv);

	float4 diffuseColor = toonDiff * diffuse * texColor; //diffuse
	float4 specularColor = float4(min(0, specularB * specular.rgb) * specularity, texColor.a); //スぺキュラ
	float4 ambientColor = float4((texColor * ambient).rgb, texColor.a);

	return diffuseColor + specularColor + ambientColor;
	//+ float4(specularB * specular.rgb, texColor.a); //スぺキュラ
//, float4(texColor * ambient, texColor.a)); //アンビエント

/*return max(toonDiff * diffuse * texColor //diffuse
	* sph.Sample(smp, sphereMapUV) //スフィア乗算
	+ spa.Sample(smp, sphereMapUV) //スフィア加算
	+ float4(specularB * specular.rgb, texColor.a) //スぺキュラ
	, float4(texColor * ambient.rgb, texColor.a)); //アンビエント
*/
}
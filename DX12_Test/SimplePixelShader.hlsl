#include"SimpleShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ
Texture2D<float4> sph:register(t1);//1番スロットに設定されたテクスチャ
Texture2D<float4> spa:register(t2);//2番スロットに設定されたテクスチャ(加算)
SamplerState smp:register(s0);//0番スロットに設定されたサンプラ


float4 PSMain(BasicType input) : SV_TARGET{

	return float4(1,0,0,1);
	//return float4(.Sample(smp,input.uv));
	//return float4(input.normal.xyz,1);
	/*float3 light = normalize(float3(1,-1,1)); //光

	float3 lightColor = float3(1, 1, 1); //ライト色

	//ディフューズ
	float diffuseB = dot(-light, input.normal);

	//反射ベクトル
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	//スフィアマップ用UV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	//テクスチャカラー
	float4 texColor = tex.Sample(smp, input.uv);

	return max(diffuseB * diffuse * texColor //diffuse
		* sph.Sample(smp, sphereMapUV) //スフィア乗算
		+ spa.Sample(smp, sphereMapUV) //スフィア加算
		+ float4(specularB * specular.rgb, 1) //スぺキュラ
		, float4(texColor * ambient, 1)); //アンビエント
		*/
		
}
//頂点シェーダ→ピクセルシェーダへのやり取りに使用する
//構造体
struct BasicType {
	float4 svpos:SV_POSITION;//システム用頂点座標
	float4 pos:POSITION; //頂点座標
	float4 normal:NORMAL;//法線ベクトル
	float4 vnormal:NORMAL1;//ビュー変換後の法線ベクトル
	float2 uv:TEXCOORD;//UV値
	float3 ray:VECTOR;//視点からのレイ
};

cbuffer cbuff0 : register(b0) {
	matrix world;//ワールド変換行列
	matrix view;//ビュー行列
	matrix proj;//ビュープロジェクション行列
	float3 eye;//視点
};

cbuffer Material : register(b1) {
	float4 diffuse;
	float alpha;
	float4 specular;
	float specularity;
	float3 ambient;
};

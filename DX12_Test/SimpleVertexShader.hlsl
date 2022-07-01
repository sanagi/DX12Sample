#include"SimpleShaderHeader.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ
SamplerState smp:register(s0);//0番スロットに設定されたサンプラ

BasicType VSMain(float4 pos : POSITION, float2 uv : TEXCOORD) {
	BasicType output;//ピクセルシェーダへ渡す値
	output.svpos = pos;
	output.uv = uv;
	return output;
}
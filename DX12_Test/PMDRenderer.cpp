#include "PMDRenderer.h"
#include "Texture.h"

using namespace std;

namespace {
	void PrintErrorBlob(ID3DBlob* blob) {
		assert(blob);
		string err;
		err.resize(blob->GetBufferSize());
		copy_n((char*)blob->GetBufferPointer(), err.size(), err.begin());
	}
}

#pragma region コンストラクタ系

#pragma endregion


PMDRenderer::PMDRenderer(ComPtr<ID3D12Device> device, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName)
{
	assert(SUCCEEDED(CreateRootSignature(device)));
	assert(SUCCEEDED(CreateGraphicsPipelineForPMD(device, vertexShaderName, pixelShaderName)));
	WhiteTex = Texture::CreateWhiteTexture(device);
	AlphaTex = Texture::CreateAlphaTexture(device);
	BlackTex = Texture::CreateBlackTexture(device);
	GradTex = Texture::CreateGrayGradationTexture(device);
}

PMDRenderer::~PMDRenderer()
{

}

#pragma region 描画ループ

void PMDRenderer::Update() {

}
void PMDRenderer::Draw() {

}

#pragma endregion

#pragma region レンダー関係

/// <summary>
/// コンパイラシェーダのチェック
/// </summary>
/// <param name="result"></param>
/// <param name="error"></param>
/// <returns></returns>
bool PMDRenderer::CheckShaderCompileResult(HRESULT result, ID3DBlob* error) {
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(error->GetBufferSize());
			std::copy_n((char*)error->GetBufferPointer(), error->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		return false;
	}
	else {
		return true;
	}
}

//パイプラインステートオブジェクト
//重要。後にリファクタするならここを色々分けたい
HRESULT PMDRenderer::CreateGraphicsPipelineForPMD(ComPtr<ID3D12Device> device, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName) {
	HRESULT hr{};

	ComPtr<ID3DBlob> vertexShader{};
	ComPtr<ID3DBlob> pixelShader{};
	ComPtr<ID3DBlob> errorBlob = nullptr;

#if defined(_DEBUG)
	UINT compile_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compile_flag = 0;
#endif

	//頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(
		vertexShaderName, //シェーダー名
		nullptr, //マクロオブジェクト
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //includeのオブジェクト
		"VSMain", //シェーダーのメイン関数
		"vs_5_0", //シェーダーの種類
		compile_flag, //コンパイル設定
		0, //受け取り用のポインタアドレス(シェーダーは0)
		vertexShader.GetAddressOf(), //vsシェーダーのアドレス
		errorBlob.GetAddressOf() //エラー用アドレス
	);
	if (!CheckShaderCompileResult(hr, errorBlob.Get())) {
		assert(0);
		return hr;
	}

	//ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(
		pixelShaderName,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain",
		"ps_5_0",
		compile_flag,
		0,
		pixelShader.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (!CheckShaderCompileResult(hr, errorBlob.Get())) {
		assert(0);
		return hr;
	}

	//パイプラインステート
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	//ルートシグネチャ
	gpipeline.pRootSignature = _rootSignature.Get();

	//シェーダー
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());

	//サンプラ
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

	//ブレンドの指定
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpipeline.BlendState.AlphaToCoverageEnable = TRUE;
	gpipeline.BlendState.IndependentBlendEnable = FALSE;
	for (int i = 0; i < _countof(gpipeline.BlendState.RenderTarget); ++i)
	{
		//見やすくするため変数化
		auto rt = gpipeline.BlendState.RenderTarget[i];
		rt.BlendEnable = TRUE;
		rt.LogicOpEnable = FALSE;
		rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.LogicOp = D3D12_LOGIC_OP_NOOP;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	//ラスタライザ指定
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングしない
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;//裏面のカリング

	//デプスステンシルの指定
	gpipeline.DepthStencilState.DepthEnable = true; //デプステストあり
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //小さい方を採用
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //全て書き込み
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.StencilEnable = false; //ステンシルテスト無し

	//インプットレイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = _inputLayout; //レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(_inputLayout); //レイアウト配列数
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

	//レンダーターゲットの設定
	gpipeline.NumRenderTargets = 1;//今は１つのみ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0～1に正規化されたRGBA

	//サンプル設定
	gpipeline.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
	gpipeline.SampleDesc.Quality = 0;//クオリティは最低

	//パイプライン作成
	hr = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipeline.ReleaseAndGetAddressOf()));

	return hr;
}

//ルートシグネチャ(ディスクリプターテーブルを管理するもの)を作る
HRESULT PMDRenderer::CreateRootSignature(ComPtr<ID3D12Device> device) {
	HRESULT hr{};

	ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //頂点情報の列挙がある事を伝える

	//ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descTblRange[4] = {}; //テクスチャと定数、ボーン、マテリアル

	descTblRange[0].NumDescriptors = 1;//定数ひとつ
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//種別は定数
	descTblRange[0].BaseShaderRegister = 0;//0番スロットから
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//マテリアル用
	descTblRange[1].NumDescriptors = 1;//テクスチャひとつ
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//種別は定数
	descTblRange[1].BaseShaderRegister = 1;//1番スロットから
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//テクスチャ4
	descTblRange[2].NumDescriptors = TOON_MATERIAL_DESC_SIZE - 1;
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//種別はテクスチャ
	descTblRange[2].BaseShaderRegister = 0;
	descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ボーン
	descTblRange[3].NumDescriptors = 1;
	descTblRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//種別は定数
	descTblRange[3].BaseShaderRegister = 2; //2番スロットへ
	descTblRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメーター
	D3D12_ROOT_PARAMETER rootparam[3] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];//デスクリプタレンジのアドレス
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//デスクリプタレンジ数
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//全シェーダから見える

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];//デスクリプタレンジのアドレス
	rootparam[1].DescriptorTable.NumDescriptorRanges = 2;//デスクリプタレンジ数
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダから見える

	rootparam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[2].DescriptorTable.pDescriptorRanges = &descTblRange[3];//デスクリプタレンジのアドレス
	rootparam[2].DescriptorTable.NumDescriptorRanges = 1;//デスクリプタレンジ数
	rootparam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//バーテックスシェーダから見える

	rootSignatureDesc.pParameters = rootparam;//ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = 3;//ルートパラメータ数

	//サンプラの指定
	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	samplerDesc[0].MinLOD = 0.0f;//ミップマップ最小値
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

	samplerDesc[1] = samplerDesc[0];
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ShaderRegister = 1;

	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	ID3DBlob* rootSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
	rootSigBlob->Release();

	return hr;
}

#pragma endregion

#pragma region ゲッター

ID3D12PipelineState* PMDRenderer::GetPipelineState() {
	return _pipeline.Get();
}

ID3D12RootSignature* PMDRenderer::GetRootSignature() {
	return _rootSignature.Get();
}

#pragma endregion
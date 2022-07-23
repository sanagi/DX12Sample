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
	BlackTex = Texture::CreateBlackTexture(device);
	//_gradTex = CreateGrayGradationTexture();
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

	//ラスタライザ指定
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングしない

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

	//ディスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descTblRanges[4] = {}; //テクスチャ2つと定数の3つ
	descTblRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//定数[b0](ビュープロジェクション用)
	descTblRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);//定数[b1](ワールド、ボーン用)
	descTblRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);//定数[b2](マテリアル用)
	descTblRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//テクスチャ４つ(基本とsphとspa、トゥーン)

	//ルートパラメーター
	D3D12_ROOT_PARAMETER rootparam[2] = {};
	CD3DX12_ROOT_PARAMETER rootParams[3] = {};
	rootParams[0].InitAsDescriptorTable(1, &descTblRanges[0]);//ビュープロジェクション変換
	rootParams[1].InitAsDescriptorTable(1, &descTblRanges[1]);//ワールド・ボーン変換
	rootParams[2].InitAsDescriptorTable(2, &descTblRanges[2]);//マテリアル周り

	//サンプラ系の指定
	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
	samplerDescs[0].Init(0);
	samplerDescs[1].Init(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	//ルートシグネチャ
	CD3DX12_ROOT_SIGNATURE_DESC  rootSignatureDesc = {};
	rootSignatureDesc.Init(3, rootParams, 2, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (FAILED(hr)) {
		assert(SUCCEEDED(hr));
		return hr;
	}
	hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(_rootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		assert(SUCCEEDED(hr));
		return hr;
	}
	//rootSigBlob->Release();

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
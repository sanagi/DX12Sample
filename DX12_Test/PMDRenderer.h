#pragma once
#include "BaseInclude.h"

class PMDRenderer
{
private:
	D3D12_INPUT_ELEMENT_DESC _inputLayout[5] = {
	{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	ComPtr<ID3D12PipelineState> _pipeline = nullptr;//PMD用パイプライン
	ComPtr<ID3D12RootSignature> _rootSignature = nullptr;//PMD用ルートシグネチャ

	//パイプライン初期化
	HRESULT CreateGraphicsPipelineForPMD(ComPtr<ID3D12Device> device, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	//ルートシグネチャ初期化
	HRESULT CreateRootSignature(ComPtr<ID3D12Device> device);

	bool CheckShaderCompileResult(HRESULT result, ID3DBlob* error = nullptr);

public:
	static const int TOON_MATERIAL_DESC_SIZE = 5; //マテリアル、基本テクスチャ、スフィア2種

	//PMD用共通テクスチャ(白、黒、グレイスケールグラデーション)
	ComPtr<ID3D12Resource> WhiteTex = nullptr;
	ComPtr<ID3D12Resource> AlphaTex = nullptr;
	ComPtr<ID3D12Resource> BlackTex = nullptr;
	ComPtr<ID3D12Resource> GradTex = nullptr;

	PMDRenderer(ComPtr<ID3D12Device> device, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	~PMDRenderer();
	void Update();
	void Draw();

	ID3D12PipelineState* GetPipelineState();
	ID3D12RootSignature* GetRootSignature();
};


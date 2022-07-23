#include "PMDActor.h"

const int MATERIAL_DESC_SIZE = 4; //マテリアル、基本テクスチャ、スフィア2種

#pragma region コンストラクタ系

PMDActor::PMDActor(ComPtr<ID3D12Device> device, const char* filepath, PMDRenderer renderer) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/巡音ルカ.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//モデル生成
	//_model = new Model(fp, device, filepath);

	//マテリアル作成
	_material = new Material(device, fp, filepath, MATERIAL_DESC_SIZE, renderer);

	//トランスフォーム生成
	//_transformMatrix = new Matrix();
	//_transformMatrix->CreateTransformView(device);
	//Matrix::CreateTransformView(device, _mappedTransform, _transformHeap);
	//_transformMatrix->GetTransformData()->world = XMMatrixIdentity();

	//_transform.world = XMMatrixIdentity();
	//CreateTransformView(device);

	fclose(fp);
}


PMDActor::~PMDActor()
{
}

#pragma endregion

/*HRESULT PMDActor::CreateTransformView(ComPtr<ID3D12Device> device) {
	//GPUバッファ作成
	auto buffSize = sizeof(Matrix::Transform);
	buffSize = (buffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_transformBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	//マップとコピー
	result = _transformBuff->Map(0, nullptr, (void**)&_mappedTransform);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}
	*_mappedTransform = _transform;

	//ビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;//とりあえずワールドひとつ
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = device->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = buffSize;
	device->CreateConstantBufferView(&cbvDesc, _transformHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}*/

#pragma region 描画ループ

void PMDActor::Update() {
	//_angle += 0.03f;
	//_transformMatrix->GetTransformData()->world = XMMatrixRotationY(_angle);
	//_angle += 0.03f;
	//_mappedTransform->world = XMMatrixRotationY(_angle);
}

void PMDActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//モデル描画
	//_model->SetRenderBuffer(command_list);

	//トランスフォーム用のヒープ指定
	//ID3D12DescriptorHeap* transheaps[] = {_transformMatrix->GetTransformHeap().Get()};
	//command_list->SetDescriptorHeaps(1, transheaps);
	//command_list->SetGraphicsRootDescriptorTable(1, _transformMatrix->GetTransformHeap()->GetGPUDescriptorHandleForHeapStart());
	
	//ID3D12DescriptorHeap* transheaps[] = { _transformHeap.Get() };
	//command_list->SetDescriptorHeaps(1, transheaps);
	//command_list->SetGraphicsRootDescriptorTable(1, _transformHeap->GetGPUDescriptorHandleForHeapStart());

	//マテリアル描画
	_material->Draw(device, command_list, MATERIAL_DESC_SIZE);
	
}

#pragma endregion

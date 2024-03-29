#include "PMDBone.h"

#pragma region コンストラクタ

PMDBone::PMDBone(ComPtr<ID3D12Device> device, FILE* fp) {
	LoadBone(device, fp);
	CreateResource(device);
}

PMDBone::~PMDBone() {

}

#pragma endregion

#pragma region 読み込み

void PMDBone::LoadBone(ComPtr<ID3D12Device> device, FILE* fp) {
	unsigned short boneNum = 0;
	fread(&boneNum, sizeof(boneNum), 1, fp);

	vector<Bone> pmdBones(boneNum);
	fread(pmdBones.data(), sizeof(Bone), boneNum, fp);
	fclose(fp);

	//インデックスと名前の対応関係構築のために後で使う
	vector<string> boneNames(pmdBones.size());
	//ボーンノードマップを作る
	for (int idx = 0; idx < pmdBones.size(); ++idx) {
		auto& pb = pmdBones[idx];
		boneNames[idx] = pb.boneName;
		auto& node = BoneNodeTable[pb.boneName];
		node.boneIdx = idx;
		node.startPos = pb.pos;
	}
	//親子関係を構築する
	for (auto& pb : pmdBones) {
		//親インデックスをチェック(あり得ない番号なら飛ばす)
		if (pb.parentNo >= pmdBones.size()) {
			continue;
		}
		auto parentName = boneNames[pb.parentNo];
		BoneNodeTable[parentName].children.emplace_back(&BoneNodeTable[pb.boneName]);
	}
	BoneMatrices.resize(pmdBones.size());
}

HRESULT PMDBone::CreateResource(ComPtr<ID3D12Device> device) {
	HRESULT hr{};

	//アップロード用のヒープとリソース用ディスクリプタ
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) * BoneMatrices.size() + 0xff) & ~0xff);
	//定数バッファ作成
	hr = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_boneBuffer)
	);

	//マップとコピー
	hr = _boneBuffer->Map(0, nullptr, (void**)&BoneMappedMatrix);
	if (FAILED(hr)) {
		assert(SUCCEEDED(hr));
		return hr;
	}
	std::copy(BoneMatrices.begin(), BoneMatrices.end(), BoneMappedMatrix);

	//リソース用のヒープ
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_boneMatrixDescHeap.ReleaseAndGetAddressOf()));//生成

	//バッファビュー用ディスクリプタ
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _boneBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_boneBuffer->GetDesc().Width);

	D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle = _boneMatrixDescHeap->GetCPUDescriptorHandleForHeapStart();

	//定数バッファビューの作成
	device->CreateConstantBufferView(&cbvDesc, resourceHeapHandle);

	return hr;
}

#pragma endregion 

#pragma region ボーン設定

void PMDBone::InitializeBone() {
	//ボーンをすべて初期化する。
	std::fill(BoneMatrices.begin(), BoneMatrices.end(), XMMatrixIdentity());

	auto armNode = BoneNodeTable["左腕"];
	auto& armPos = armNode.startPos;
	auto armMat = XMMatrixTranslation(-armPos.x, -armPos.y, -armPos.z) * XMMatrixRotationZ(XM_PIDIV2) * XMMatrixTranslation(armPos.x, armPos.y, armPos.z);
	BoneMatrices[armNode.boneIdx] = armMat;

	auto elbowNode = BoneNodeTable["左ひじ"];
	auto& elbowPos = elbowNode.startPos;
	auto elbowMat = XMMatrixTranslation(-elbowPos.x, -elbowPos.y, -elbowPos.z) * XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixTranslation(elbowPos.x, elbowPos.y, elbowPos.z);
	BoneMatrices[elbowNode.boneIdx] = elbowMat;

	RecursiveMatrixMultiply(&BoneNodeTable["センター"], XMMatrixIdentity());

	std::copy(BoneMatrices.begin(), BoneMatrices.end(), BoneMappedMatrix);
}

/// <summary>
/// ボーンの回転をGUPに伝える
/// </summary>
/// <param name="command_list"></param>
void PMDBone::SettingBone(ComPtr<ID3D12GraphicsCommandList> command_list) {
	ID3D12DescriptorHeap* sceneheaps[] = { _boneMatrixDescHeap.Get() };
	command_list->SetDescriptorHeaps(1, sceneheaps);
	command_list->SetGraphicsRootDescriptorTable(2, _boneMatrixDescHeap->GetGPUDescriptorHandleForHeapStart());
}

/// <summary>
/// 再帰的にボーンの回転
/// </summary>
/// <param name="node"></param>
/// <param name="mat"></param>
void PMDBone::RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat) {
	BoneMatrices[node->boneIdx] *= mat;
	for (auto& cnode : node->children) {
		RecursiveMatrixMultiply(cnode, BoneMatrices[node->boneIdx]);
	}
}

#pragma endregion
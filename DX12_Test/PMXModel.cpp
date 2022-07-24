#include "PMXModel.h"
#include <array>
#include <fstream>

#pragma region コンストラクタ系

PMXModel::PMXModel(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName) {

	//ファイル読み込み
	Open(fp, device, modelName, "rb");
	//頂点情報とインデックス元にリソース作成
	CreateResource(device, _vertices, _indices);
}

PMXModel::~PMXModel() {
}

#pragma endregion

#pragma region ファイルを開く,リソース生成

void PMXModel::Open(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName, const char* mode) {
	HRESULT hr{};

	auto pmxFile = std::ifstream{ modelName, (std::ios::binary | std::ios::in) };
#pragma region ヘッダーを読む

	// ヘッダー -------------------------------
	std::array<byte, 4> pmxHeader{};
	constexpr std::array<byte, 4> PMX_MAGIC_NUMBER{ 0x50, 0x4d, 0x58, 0x20 };
	enum HeaderDataIndex
	{
		ENCODING_FORMAT,
		NUMBER_OF_ADD_UV,
		VERTEX_INDEX_SIZE,
		TEXTURE_INDEX_SIZE,
		MATERIAL_INDEX_SIZE,
		BONE_INDEX_SIZE,
		RIGID_BODY_INDEX_SIZE
	};

	for (int i = 0; i < 4; i++)
	{
		pmxHeader[i] = pmxFile.get();
	}
	if (pmxHeader != PMX_MAGIC_NUMBER)
	{
		pmxFile.close();
	}

	// ver2.0以外は非対応
	float version{};
	pmxFile.read(reinterpret_cast<char*>(&version), 4);
	if (!XMScalarNearEqual(version, 2.0f, g_XMEpsilon.f[0]))
	{
		pmxFile.close();
	}

	byte hederDataLength = pmxFile.get();
	if (hederDataLength != 8)
	{
		pmxFile.close();
	}
	std::array<byte, 8> hederData{};
	for (int i = 0; i < hederDataLength; i++)
	{
		hederData[i] = pmxFile.get();
	}
	//UTF-8は非対応
	if (hederData[0] != 0)
	{
		pmxFile.close();
	}

	unsigned arrayLength{};
	for (int i = 0; i < 4; i++)
	{
		pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
		for (unsigned j = 0; j < arrayLength; j++)
		{
			pmxFile.get();
		}
	}

#pragma endregion

#pragma region 頂点
	int numberOfVertex{};
	pmxFile.read(reinterpret_cast<char*>(&numberOfVertex), 4);
	_vertices = std::vector<PMXVertex>(numberOfVertex);

	for (auto i = 0; i < numberOfVertex; i++)
	{
		pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].position), 12);
		pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].normal), 12);
		pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].uv), 8);
		if (hederData[NUMBER_OF_ADD_UV] != 0)
		{
			for (int j = 0; j < hederData[NUMBER_OF_ADD_UV]; ++j)
			{
				pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].additionalUV[j]), 16);
			}
		}

		const byte weightMethod = pmxFile.get();
		switch (weightMethod)
		{
		case PMXVertex::Weight::BDEF1:
			_vertices[i].weight.type = PMXVertex::Weight::BDEF1;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			_vertices[i].weight.born2 = NO_DATA_FLAG;
			_vertices[i].weight.born3 = NO_DATA_FLAG;
			_vertices[i].weight.born4 = NO_DATA_FLAG;
			_vertices[i].weight.weight1 = 1.0f;
			break;

		case PMXVertex::Weight::BDEF2:
			_vertices[i].weight.type = PMXVertex::Weight::BDEF2;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			_vertices[i].weight.born3 = NO_DATA_FLAG;
			_vertices[i].weight.born4 = NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.weight1), 4);
			_vertices[i].weight.weight2 = 1.0f - _vertices[i].weight.weight1;
			break;

		case PMXVertex::Weight::BDEF4:
			_vertices[i].weight.type = PMXVertex::Weight::BDEF4;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born3), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born4), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.weight1), 4);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.weight2), 4);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.weight3), 4);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.weight4), 4);
			break;

		case PMXVertex::Weight::SDEF:
			_vertices[i].weight.type = PMXVertex::Weight::SDEF;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			_vertices[i].weight.born3 = NO_DATA_FLAG;
			_vertices[i].weight.born4 = NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.weight1), 4);
			_vertices[i].weight.weight2 = 1.0f - _vertices[i].weight.weight1;
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.c), 12);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.r0), 12);
			pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].weight.r1), 12);
			break;

		default:
			pmxFile.close();
		}

		pmxFile.read(reinterpret_cast<char*>(&this->_vertices[i].edgeMagnif), 4);
	}
#pragma endregion
	
	//インデックス数読み込み
	fread(&_indicesNum, sizeof(_indicesNum), 1, fp);

	//インデックス読み込み
	_indices = std::vector<unsigned short>(_indicesNum);
	fread(_indices.data(), _indices.size() * sizeof(_indices[0]), 1, fp);
}

/// <summary>
/// リソース作成
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void PMXModel::CreateResource(ComPtr<ID3D12Device> device, std::vector<PMXVertex> vertices, std::vector<unsigned short> indices) {
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PMXVertex));
	//UPLOAD(確保は可能)
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	//頂点のコピー
	PMXVertex* vertMap = nullptr;
	result = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//バッファビュー
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress(); //バッファの仮想アドレス
	_vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(PMXVertex));//全バイト数
	_vbView.StrideInBytes = sizeof(PMXVertex);//1頂点あたりのバイト数


	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));

	//設定は、バッファのサイズ以外頂点バッファの設定を使いまわして
	//OKだと思います。
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer));

	//作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

}


#pragma endregion

void PMXModel::SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//頂点情報のセット
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //トポロジ指定
	//バッファビューの指定
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//インデックスバッファビューの指定
	command_list->IASetIndexBuffer(&_indexBufferView);
}
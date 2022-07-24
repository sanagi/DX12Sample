#include "PMXModel.h"
#include <array>
#include <fstream>

#pragma region �R���X�g���N�^�n

PMXModel::PMXModel(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName) {

	//�t�@�C���ǂݍ���
	Open(fp, device, modelName, "rb");
	//���_���ƃC���f�b�N�X���Ƀ��\�[�X�쐬
	CreateResource(device, _vertices, _indices);
}

PMXModel::~PMXModel() {
}

#pragma endregion

#pragma region �t�@�C�����J��,���\�[�X����

void PMXModel::Open(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName, const char* mode) {
	HRESULT hr{};

	auto pmxFile = std::ifstream{ modelName, (std::ios::binary | std::ios::in) };
#pragma region �w�b�_�[��ǂ�

	// �w�b�_�[ -------------------------------
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

	// ver2.0�ȊO�͔�Ή�
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
	//UTF-8�͔�Ή�
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

#pragma region ���_
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
	
	//�C���f�b�N�X���ǂݍ���
	fread(&_indicesNum, sizeof(_indicesNum), 1, fp);

	//�C���f�b�N�X�ǂݍ���
	_indices = std::vector<unsigned short>(_indicesNum);
	fread(_indices.data(), _indices.size() * sizeof(_indices[0]), 1, fp);
}

/// <summary>
/// ���\�[�X�쐬
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void PMXModel::CreateResource(ComPtr<ID3D12Device> device, std::vector<PMXVertex> vertices, std::vector<unsigned short> indices) {
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PMXVertex));
	//UPLOAD(�m�ۂ͉\)
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	//���_�̃R�s�[
	PMXVertex* vertMap = nullptr;
	result = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//�o�b�t�@�r���[
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress(); //�o�b�t�@�̉��z�A�h���X
	_vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(PMXVertex));//�S�o�C�g��
	_vbView.StrideInBytes = sizeof(PMXVertex);//1���_������̃o�C�g��


	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));

	//�ݒ�́A�o�b�t�@�̃T�C�Y�ȊO���_�o�b�t�@�̐ݒ���g���܂킵��
	//OK���Ǝv���܂��B
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer));

	//������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

}


#pragma endregion

void PMXModel::SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//���_���̃Z�b�g
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //�g�|���W�w��
	//�o�b�t�@�r���[�̎w��
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//�C���f�b�N�X�o�b�t�@�r���[�̎w��
	command_list->IASetIndexBuffer(&_indexBufferView);
}
#pragma once
#include "BaseInclude.h"

class PMDModel
{
public:

	struct PMDHeader {
		float version;
		char model_name[20];
		char comment[256];
	};

	struct PMDVertex {
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		uint16_t bone_no[2];
		uint8_t  weight;
		uint8_t  EdgeFlag;
		uint16_t dummy;
	};

	PMDModel(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName);
	~PMDModel();
	void SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list);

private:

	unsigned int _indicesNum; //インデックス数

	ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vbView;

	ComPtr<ID3D12Resource> _indexBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	std::vector<PMDVertex> _vertices;//バッファ確保
	std::vector<unsigned short> _indices;

	void Open(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName, const char* mode);
	void CreateResource(ComPtr<ID3D12Device> device, std::vector<PMDVertex> vertices, std::vector<unsigned short> indices);
};
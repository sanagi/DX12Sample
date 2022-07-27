#pragma once
#include "BaseInclude.h"

class PMXModel
{
public:
	struct PMXVertex {
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		std::vector<XMFLOAT4> additionalUV;
		
		// ボーンウェイト
		struct Weight
		{
			enum Type
			{
				BDEF1,
				BDEF2,
				BDEF4,
				SDEF,
			};

			Type type;
			int born1;
			int	born2;
			int	born3;
			int	born4;
			float weight1;
			float weight2;
			float weight3;
			float weight4;
			XMFLOAT3 c;
			XMFLOAT3 r0;
			XMFLOAT3 r1;
		} weight;

		uint8_t  edgeMagnif;
	};

	std::vector<PMXVertex> Vertices;//バッファ確保
	std::vector<unsigned short> Indices;

	PMXModel();
	~PMXModel();
	void CreateResource(ComPtr<ID3D12Device> device);
	void SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list);

private:

	unsigned int _indicesNum; //インデックス数

	ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vbView;

	ComPtr<ID3D12Resource> _indexBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
};
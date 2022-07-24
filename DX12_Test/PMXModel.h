#pragma once
#include "BaseInclude.h"

class PMXModel
{
public:

	struct PMXHeader {
		int pmxMagicNumber[4];
		float version;

		int encodingFormat;
		int numberOfAddUV;
		int vertexIndexSize;
		int textureIndexSize;
		int materialIndexSize;
		int boneIndexSize;
		int rigidBodyIndexSize;
	};

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

	PMXModel(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName);
	~PMXModel();
	void SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list);

private:

	static constexpr int NO_DATA_FLAG = -1;

	unsigned int _indicesNum; //インデックス数

	ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vbView;

	ComPtr<ID3D12Resource> _indexBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	std::vector<PMXVertex> _vertices;//バッファ確保
	std::vector<unsigned short> _indices;

	void Open(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName, const char* mode);
	void CreateResource(ComPtr<ID3D12Device> device, std::vector<PMXVertex> vertices, std::vector<unsigned short> indices);
};
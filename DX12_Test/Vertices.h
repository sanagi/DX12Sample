#pragma once
#include "BaseInclude.h"

class Vertices
{
public:
	struct VertexInfo {
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	Vertices(ComPtr<ID3D12Device> device);
	~Vertices() {}
	HRESULT Initialize(ComPtr<ID3D12Device> device);
	void Draw(ComPtr<ID3D12GraphicsCommandList> command_list);

private:
	ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vbView;

	ComPtr<ID3D12Resource> _indexBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
};


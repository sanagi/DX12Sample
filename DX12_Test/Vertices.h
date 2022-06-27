#pragma once
#include "BaseInclude.h"

class Vertices
{
	Vertices();
	~Vertices() {}
	HRESULT Initialize(ID3D12Device* device);
	HRESULT Draw(ID3D12GraphicsCommandList* command_list);

private:
	ComPtr<ID3D12Resource> _vertexBuffer;
	ComPtr<ID3D12Resource> _indexBuffer;
};


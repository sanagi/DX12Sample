#pragma once
#include "BaseInclude.h"

class Texture
{

public:
	struct TexRGBA
	{
		unsigned char R, G, B, A;
	};

	std::vector<TexRGBA> _texData;
	TexMetadata _metaData;
	ComPtr<ID3D12Resource> TexBuffer;

	D3D12_TEXTURE_COPY_LOCATION* src;
	D3D12_TEXTURE_COPY_LOCATION* dst;


	Texture(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);

private:
	void Initialize(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);
};


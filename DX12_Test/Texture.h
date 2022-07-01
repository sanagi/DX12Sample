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

	//テクスチャ用ヒープ
	ID3D12DescriptorHeap* TexHeaps = nullptr;

	D3D12_TEXTURE_COPY_LOCATION* src;
	D3D12_TEXTURE_COPY_LOCATION* dst;


	Texture(ComPtr<ID3D12Device> device);

	void CreateResource(ComPtr<ID3D12Device> device);

private:
	void Initialize(ComPtr<ID3D12Device> device);

};


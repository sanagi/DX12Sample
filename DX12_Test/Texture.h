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
	ComPtr<ID3D12Resource> TexBuffer;

	D3D12_TEXTURE_COPY_LOCATION* src;
	D3D12_TEXTURE_COPY_LOCATION* dst;


	Texture(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);

	static std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath);
	static ID3D12Resource* LoadTextureFromFile(ComPtr<ID3D12Device> device, std::string& texPath);
	static ID3D12Resource* CreateWhiteTexture(ComPtr<ID3D12Device> device);

private:
	//void Initialize(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);

	static std::wstring GetWideStringFromString(const std::string& str);
};


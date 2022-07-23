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


	static std::string GetExtension(const std::string& path);
	static std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*');
	static std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath);
	static ID3D12Resource* LoadTextureFromFile(ComPtr<ID3D12Device> device, std::string& texPath);
	static ID3D12Resource* CreateWhiteTexture(ComPtr<ID3D12Device> device);//白テクスチャの生成
	static ID3D12Resource* CreateBlackTexture(ComPtr<ID3D12Device> device);//黒テクスチャの生成
	static ID3D12Resource* CreateGrayGradationTexture(ComPtr<ID3D12Device> device);//グレーテクスチャの生成
	
private:
	static std::wstring GetWideStringFromString(const std::string& str);
	static ID3D12Resource* CreateDefaultTexture(ComPtr<ID3D12Device> device, size_t width, size_t height);

	//非公開クラス
	Texture() {};
	~Texture() {};
};


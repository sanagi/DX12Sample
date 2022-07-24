#pragma once
#include "Texture.h"
#include "PMXRenderer.h"

class PMXMaterial
{
public:
	PMXMaterial(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath, int sizeNum, PMXRenderer renderer);
	~PMXMaterial();

#pragma pack(1)//ここから1バイトパッキング…アライメントは発生しない
	struct PMXMaterialData
	{
		XMFLOAT3 diffuse; // ディフューズ
		XMFLOAT3 specular; //スぺキュラ色
		float specularity; //スぺキュラの強さ
		XMFLOAT3 ambient; //アンビエント色

		int colorMapTextureIndex;
		int toonTextureIndex; //トゥーン番号

		//2バイトのパディング

		unsigned int indicesNum; //このマテリアルが割り当てられるインデックス数
		char texFilePath[20]; //テクスチャファイルパス
	
		//unsigned char edgeFlag; //マテリアルの輪郭線フラグ
	};
#pragma pack()//1バイトパッキング解除

	struct MaterialForhlsl
	{
		XMFLOAT3 diffuse;
		float alpha;
		XMFLOAT3 specular;
		float specularity;
		XMFLOAT3 ambient;
	};

	struct AdditionarlMaterial
	{
		std::string texPath;
		int toonIdx;
		bool edgeFlag;
	};

	struct MaterialData {
		unsigned int indicesNum; //インデックス数
		MaterialForhlsl material;
		AdditionarlMaterial additionarl;
	};

	std::vector<PMXMaterialData> _pmxMaterialVector;
	std::vector<MaterialData> _materialVector;
	std::vector<ComPtr<ID3D12Resource>> _textureVector;
	std::vector<ComPtr<ID3D12Resource>> _sphTexVector;
	std::vector<ComPtr<ID3D12Resource>> _spaTexVector;
	std::vector<ComPtr<ID3D12Resource>> _toonTexVector;

	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, int sizeNum);

private:
	PMXRenderer _renderer;

	unsigned int _materialNum; //マテリアル数
	ID3D12DescriptorHeap* _descHeap = nullptr;

	void Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath);
	void CreateResource(ComPtr<ID3D12Device> device, int sizeNum);

	//ファイル名パスとリソースのマップテーブル
	map<string, ID3D12Resource*> _resourceTable;
};


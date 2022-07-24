#pragma once
#include "Texture.h"
#include "PMDRenderer.h"

class Material
{
public:
	Material(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath, int sizeNum, PMDRenderer renderer);
	~Material();

#pragma pack(1)//ここから1バイトパッキング…アライメントは発生しない
	struct PMDMaterial
	{
		XMFLOAT3 diffuse; // ディフューズ
		float alpha; //ディフューズα
		float specularity; //スぺキュラの強さ
		XMFLOAT3 specular; //スぺキュラ色
		XMFLOAT3 ambient; //アンビエント色
		unsigned char toonIdx; //トゥーン番号
		unsigned char edgeFlag; //マテリアルの輪郭線フラグ

		//2バイトのパディング

		unsigned int indicesNum; //このマテリアルが割り当てられるインデックス数
		char texFilePath[20]; //テクスチャファイルパス
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

	std::vector<PMDMaterial> _pmdMaterialVector;
	std::vector<MaterialData> _materialVector;
	std::vector<ComPtr<ID3D12Resource>> _textureVector;
	std::vector<ComPtr<ID3D12Resource>> _sphTexVector;
	std::vector<ComPtr<ID3D12Resource>> _spaTexVector;
	std::vector<ComPtr<ID3D12Resource>> _toonTexVector;

	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, int sizeNum);

private:
	PMDRenderer _renderer;

	unsigned int _materialNum; //マテリアル数
	ID3D12DescriptorHeap* _descHeap = nullptr;

	void Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath);
	void CreateResource(ComPtr<ID3D12Device> device, int sizeNum);

	//ファイル名パスとリソースのマップテーブル
	map<string, ID3D12Resource*> _resourceTable;
};


#pragma once
#include "Texture.h"
#include "PMXRenderer.h"

class PMXMaterial
{
public:
	PMXMaterial(PMXRenderer renderer);
	~PMXMaterial();

//#pragma pack(1)//ここから1バイトパッキング…アライメントは発生しない
	struct PMXMaterialData
	{
		XMFLOAT3 diffuse; // ディフューズ
		XMFLOAT3 specular; //スぺキュラ色
		float specularity; //スぺキュラの強さ
		XMFLOAT3 ambient; //アンビエント色

		int colorMapTextureIndex; //テクスチャテーブルの参照インデックス
		int sphereMapTextureIndex; //スフィアマップの参照インデックス
		int toonMapTextureIndex; //テクスチャテーブルの参照インデックス

		unsigned char sphereMode; //スフィアモード 0:無効 1:乗算(sph) 2:加算(spa) 3:サブテクスチャ(追加UV1のx,yをUV参照して通常テクスチャ描画を行う)

		int toonCommonTextureIndex; //トゥーン番号

		unsigned int indicesNum; //このマテリアルが割り当てられるインデックス数
	};
//#pragma pack()//1バイトパッキング解除

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

	std::vector<std::string> TexturePathVector;

	unsigned int MaterialNum; //マテリアル数

	std::vector<PMXMaterialData> PmxMaterialVector;
	std::vector<MaterialData> MaterialVector;
	std::vector<ComPtr<ID3D12Resource>> _textureVector;
	std::vector<ComPtr<ID3D12Resource>> _sphTexVector;
	std::vector<ComPtr<ID3D12Resource>> _spaTexVector;
	std::vector<ComPtr<ID3D12Resource>> _toonTexVector;

	void CreateMaterialDataForGPU(ComPtr<ID3D12Device> device, std::string modelPath);
	void CreateResource(ComPtr<ID3D12Device> device, int sizeNum);
	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, int sizeNum);

private:
	PMXRenderer _renderer;

	ID3D12DescriptorHeap* _descHeap = nullptr;

	//ファイル名パスとリソースのマップテーブル
	map<string, ID3D12Resource*> _resourceTable;
};


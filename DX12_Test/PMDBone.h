#pragma once
#include "BaseInclude.h"
#include "Matrix.h"

class PMDBone
{
public:
#pragma pack(1)
	//読み込み用ボーン構造体
	struct Bone {
		char boneName[20];//ボーン名
		unsigned short parentNo;//親ボーン番号
		unsigned short nextNo;//先端のボーン番号
		unsigned char type;//ボーン種別
		unsigned short ikBoneNo;//IKボーン番号
		XMFLOAT3 pos;//ボーンの基準点座標
	};
#pragma pack()

	struct MatricesData
	{
		XMMATRIX world; //モデルの回転行列
		XMMATRIX view; // ビュー行列
		XMMATRIX proj; //プロジェクション行列
		XMFLOAT3 eye; //視点
	};

	struct BoneNode {
		int boneIdx;//ボーンインデックス
		XMFLOAT3 startPos;//ボーン基準点(回転中心)
		std::vector<BoneNode*> children;//子ノード
	};

	PMDBone(ComPtr<ID3D12Device> device, FILE* fp);
	~PMDBone();

	//ボーン関連
	std::vector<XMMATRIX> BoneMatrices;
	std::map<std::string, BoneNode> BoneNodeTable;
	XMMATRIX* BoneMappedMatrix;//マップ先を示すポインタ

	void SettingBone(ComPtr<ID3D12GraphicsCommandList> command_list);
	void RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat);

private:
	ComPtr<ID3D12DescriptorHeap> _boneMatrixDescHeap;

	ComPtr<ID3D12Resource> _boneBuffer;

	void LoadBone(ComPtr<ID3D12Device> device, FILE* fp);
	HRESULT CreateResource(ComPtr<ID3D12Device> device);
	void InitializeBone();
};


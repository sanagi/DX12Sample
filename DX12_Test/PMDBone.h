#pragma once
#include "BaseInclude.h"
#include "VMDMotion.h"
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

	PMDBone(ComPtr<ID3D12Device> device, FILE* fp);
	~PMDBone();
	void SettingBone(ComPtr<ID3D12GraphicsCommandList> command_list);
	void SetQuaternion(std::unordered_map<std::string, std::vector<VMDMotion::KeyFrame>> motionMap);

private:
	//ボーン関連
	std::vector<XMMATRIX> _boneMatrices;

	struct BoneNode {
		int boneIdx;//ボーンインデックス
		XMFLOAT3 startPos;//ボーン基準点(回転中心)
		std::vector<BoneNode*> children;//子ノード
	};

	std::map<std::string, BoneNode> _boneNodeTable;

	XMMATRIX* _boneMappedMatrix;//マップ先を示すポインタ
	ComPtr<ID3D12DescriptorHeap> _boneMatrixDescHeap;

	ComPtr<ID3D12Resource> _boneBuffer;

	void LoadBone(ComPtr<ID3D12Device> device, FILE* fp);
	HRESULT CreateResource(ComPtr<ID3D12Device> device);
	void InitializeBone();

	void RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat);
};


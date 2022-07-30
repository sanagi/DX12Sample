#pragma once
#include "BaseInclude.h"
#include "Matrix.h"

class PMDBone
{
public:
#pragma pack(1)
	//�ǂݍ��ݗp�{�[���\����
	struct Bone {
		char boneName[20];//�{�[����
		unsigned short parentNo;//�e�{�[���ԍ�
		unsigned short nextNo;//��[�̃{�[���ԍ�
		unsigned char type;//�{�[�����
		unsigned short ikBoneNo;//IK�{�[���ԍ�
		XMFLOAT3 pos;//�{�[���̊�_���W
	};
#pragma pack()

	struct MatricesData
	{
		XMMATRIX world; //���f���̉�]�s��
		XMMATRIX view; // �r���[�s��
		XMMATRIX proj; //�v���W�F�N�V�����s��
		XMFLOAT3 eye; //���_
	};

	struct BoneNode {
		int boneIdx;//�{�[���C���f�b�N�X
		XMFLOAT3 startPos;//�{�[����_(��]���S)
		std::vector<BoneNode*> children;//�q�m�[�h
	};

	PMDBone(ComPtr<ID3D12Device> device, FILE* fp);
	~PMDBone();

	//�{�[���֘A
	std::vector<XMMATRIX> BoneMatrices;
	std::map<std::string, BoneNode> BoneNodeTable;
	XMMATRIX* BoneMappedMatrix;//�}�b�v��������|�C���^

	void SettingBone(ComPtr<ID3D12GraphicsCommandList> command_list);
	void RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat);

private:
	ComPtr<ID3D12DescriptorHeap> _boneMatrixDescHeap;

	ComPtr<ID3D12Resource> _boneBuffer;

	void LoadBone(ComPtr<ID3D12Device> device, FILE* fp);
	HRESULT CreateResource(ComPtr<ID3D12Device> device);
	void InitializeBone();
};


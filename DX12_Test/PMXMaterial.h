#pragma once
#include "Texture.h"
#include "PMXRenderer.h"

class PMXMaterial
{
public:
	PMXMaterial(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath, int sizeNum, PMXRenderer renderer);
	~PMXMaterial();

#pragma pack(1)//��������1�o�C�g�p�b�L���O�c�A���C�����g�͔������Ȃ�
	struct PMXMaterialData
	{
		XMFLOAT3 diffuse; // �f�B�t���[�Y
		XMFLOAT3 specular; //�X�؃L�����F
		float specularity; //�X�؃L�����̋���
		XMFLOAT3 ambient; //�A���r�G���g�F

		int colorMapTextureIndex;
		int toonTextureIndex; //�g�D�[���ԍ�

		//2�o�C�g�̃p�f�B���O

		unsigned int indicesNum; //���̃}�e���A�������蓖�Ă���C���f�b�N�X��
		char texFilePath[20]; //�e�N�X�`���t�@�C���p�X
	
		//unsigned char edgeFlag; //�}�e���A���̗֊s���t���O
	};
#pragma pack()//1�o�C�g�p�b�L���O����

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
		unsigned int indicesNum; //�C���f�b�N�X��
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

	unsigned int _materialNum; //�}�e���A����
	ID3D12DescriptorHeap* _descHeap = nullptr;

	void Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath);
	void CreateResource(ComPtr<ID3D12Device> device, int sizeNum);

	//�t�@�C�����p�X�ƃ��\�[�X�̃}�b�v�e�[�u��
	map<string, ID3D12Resource*> _resourceTable;
};


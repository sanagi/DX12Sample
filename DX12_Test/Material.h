#pragma once
#include "Texture.h"
#include "PMDRenderer.h"

class Material
{
public:
	Material(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath, int sizeNum, PMDRenderer renderer);
	~Material();

#pragma pack(1)//��������1�o�C�g�p�b�L���O�c�A���C�����g�͔������Ȃ�
	struct PMDMaterial
	{
		XMFLOAT3 diffuse; // �f�B�t���[�Y
		float alpha; //�f�B�t���[�Y��
		float specularity; //�X�؃L�����̋���
		XMFLOAT3 specular; //�X�؃L�����F
		XMFLOAT3 ambient; //�A���r�G���g�F
		unsigned char toonIdx; //�g�D�[���ԍ�
		unsigned char edgeFlag; //�}�e���A���̗֊s���t���O

		//2�o�C�g�̃p�f�B���O

		unsigned int indicesNum; //���̃}�e���A�������蓖�Ă���C���f�b�N�X��
		char texFilePath[20]; //�e�N�X�`���t�@�C���p�X
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

	std::vector<PMDMaterial> _pmdMaterialVector;
	std::vector<MaterialData> _materialVector;
	std::vector<ComPtr<ID3D12Resource>> _textureVector;
	std::vector<ComPtr<ID3D12Resource>> _sphTexVector;
	std::vector<ComPtr<ID3D12Resource>> _spaTexVector;
	std::vector<ComPtr<ID3D12Resource>> _toonTexVector;

	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, int sizeNum);

private:
	PMDRenderer _renderer;

	unsigned int _materialNum; //�}�e���A����
	ID3D12DescriptorHeap* _descHeap = nullptr;

	void Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath);
	void CreateResource(ComPtr<ID3D12Device> device, int sizeNum);

	//�t�@�C�����p�X�ƃ��\�[�X�̃}�b�v�e�[�u��
	map<string, ID3D12Resource*> _resourceTable;
};


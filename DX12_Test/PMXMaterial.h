#pragma once
#include "Texture.h"
#include "PMXRenderer.h"

class PMXMaterial
{
public:
	PMXMaterial(PMXRenderer renderer);
	~PMXMaterial();

//#pragma pack(1)//��������1�o�C�g�p�b�L���O�c�A���C�����g�͔������Ȃ�
	struct PMXMaterialData
	{
		XMFLOAT3 diffuse; // �f�B�t���[�Y
		XMFLOAT3 specular; //�X�؃L�����F
		float specularity; //�X�؃L�����̋���
		XMFLOAT3 ambient; //�A���r�G���g�F

		int colorMapTextureIndex; //�e�N�X�`���e�[�u���̎Q�ƃC���f�b�N�X
		int sphereMapTextureIndex; //�X�t�B�A�}�b�v�̎Q�ƃC���f�b�N�X
		int toonMapTextureIndex; //�e�N�X�`���e�[�u���̎Q�ƃC���f�b�N�X

		unsigned char sphereMode; //�X�t�B�A���[�h 0:���� 1:��Z(sph) 2:���Z(spa) 3:�T�u�e�N�X�`��(�ǉ�UV1��x,y��UV�Q�Ƃ��Ēʏ�e�N�X�`���`����s��)

		int toonCommonTextureIndex; //�g�D�[���ԍ�

		unsigned int indicesNum; //���̃}�e���A�������蓖�Ă���C���f�b�N�X��
	};
//#pragma pack()//1�o�C�g�p�b�L���O����

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

	std::vector<std::string> TexturePathVector;

	unsigned int MaterialNum; //�}�e���A����

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

	//�t�@�C�����p�X�ƃ��\�[�X�̃}�b�v�e�[�u��
	map<string, ID3D12Resource*> _resourceTable;
};


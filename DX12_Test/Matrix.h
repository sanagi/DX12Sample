/*#pragma once
#include "BaseInclude.h"

class Matrix
{
public:

	struct SceneData {
		DirectX::XMMATRIX view;//�r���[�s��
		DirectX::XMMATRIX proj;//�v���W�F�N�V�����s��
		DirectX::XMFLOAT3 eye;//���_���W
	};

	struct Transform {
		//�����Ɏ����Ă�XMMATRIX�����o��16�o�C�g�A���C�����g�ł��邽��
		//Transform��new����ۂɂ�16�o�C�g���E�Ɋm�ۂ���
		void* operator new(size_t size);
		XMMATRIX world;
	};

	Matrix();
	~Matrix();

	HRESULT CreateSceneView(ComPtr<ID3D12Device> device, int width, int height);
	HRESULT CreateTransformView(ComPtr<ID3D12Device> device);

	void Rotate(float angle);

	SceneData* GetMappedSceneData();
	ComPtr<ID3D12DescriptorHeap> GetSceneDescHeap();

	Transform* GetTransformData();
	ComPtr<ID3D12DescriptorHeap> GetTransformHeap();

private:
	SceneData* _mappedSceneData;
	ComPtr<ID3D12DescriptorHeap> _sceneDescHeap;

	Transform* _mappedTransformData;
	ComPtr<ID3D12DescriptorHeap> _transformDescHeap;
};
*/

#pragma once
#include "BaseInclude.h"

class Matrix
{
public:
	Matrix(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);
	void Rotate();

	struct MatricesData
	{
		XMMATRIX world; //���f���̉�]�s��
		XMMATRIX view; // �r���[�s��
		XMMATRIX proj; //�v���W�F�N�V�����s��
		XMFLOAT3 eye; //���_
	};

private:
	XMMATRIX _worldMat; //���[���h�s��
	XMMATRIX _viewMat; //�r���[�s��
	XMMATRIX _projMat; //�v���W�F�N�V�����s��

	float _angle = 0.0; //�p�x

	MatricesData* _mapMatrix;//�}�b�v��������|�C���^

	int _width;
	int _height;

	ComPtr<ID3D12Resource> _constBuffer;

	void Initialize(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);
};
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
#pragma once
#include "BaseInclude.h"

class Matrix
{
public:
	Matrix(ComPtr<ID3D12Device> device, int width, int height);
	void Rotate(float angle);
	ComPtr<ID3D12DescriptorHeap> GetDescHeap();

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

	MatricesData* _mapMatrix;//�}�b�v��������|�C���^
	ComPtr<ID3D12DescriptorHeap> _matrixDescHeap;

	int _width;
	int _height;

	ComPtr<ID3D12Resource> _constBuffer;

	void Initialize(ComPtr<ID3D12Device> device, int width, int height);
};
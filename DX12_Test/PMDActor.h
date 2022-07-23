#pragma once
#include "Matrix.h"
#include "Material.h"
#include "Model.h"

class PMDActor
{
public:
	PMDActor(ComPtr<ID3D12Device> device, const char* filepath, PMDRenderer renderer);
	~PMDActor();

	void Update();
	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list);

private:
	//���W�ϊ��n
	//Matrix* _transformMatrix;
	//ComPtr<ID3D12DescriptorHeap> _transformHeap = nullptr;//���W�ϊ��q�[�v
	//Matrix::Transform* _mappedTransform = nullptr;

	//Matrix::Transform _transform;
	//Matrix::Transform* _mappedTransform = nullptr;
	//ComPtr<ID3D12Resource> _transformBuff = nullptr;

	float _angle = 0; //���f���̊p�x
	//HRESULT CreateTransformView(ComPtr<ID3D12Device> device);

	//���f��
	Model* _model;

	//�}�e���A���֘A
	Material* _material;
};
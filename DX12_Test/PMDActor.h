#pragma once
#include "Matrix.h"
#include "PMDMaterial.h"
#include "PMDModel.h"
#include "PMDBone.h"

class PMDActor
{
public:
	PMDActor(ComPtr<ID3D12Device> device, const char* filepath, PMDRenderer renderer);
	~PMDActor();

	void Update(ComPtr<ID3D12GraphicsCommandList> command_list, std::shared_ptr<Matrix> matrix);
	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list);

private:
	float _angle = 0; //���f���̊p�x

	//���f��
	PMDModel* _model;

	//�}�e���A���֘A
	PMDMaterial* _material;

	//�{�[���֘A
	PMDBone* _bone;
};
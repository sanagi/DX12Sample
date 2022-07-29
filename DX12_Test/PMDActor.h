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
	float _angle = 0; //モデルの角度

	//モデル
	PMDModel* _model;

	//マテリアル関連
	PMDMaterial* _material;

	//ボーン関連
	PMDBone* _bone;
};
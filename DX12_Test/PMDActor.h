#pragma once
#include "Matrix.h"
#include "Material.h"
#include "Model.h"

class PMDActor
{
public:
	PMDActor(ComPtr<ID3D12Device> device, const char* filepath, PMDRenderer renderer);
	~PMDActor();

	void Update(std::shared_ptr<Matrix> matrix);
	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list);

private:
	float _angle = 0; //モデルの角度

	//モデル
	Model* _model;

	//マテリアル関連
	Material* _material;
};
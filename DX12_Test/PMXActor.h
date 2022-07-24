#pragma once
#include "Matrix.h"
#include "PMXMaterial.h"
#include "PMXModel.h"

class PMXActor
{
public:
	PMXActor(ComPtr<ID3D12Device> device, const char* filepath, PMXRenderer renderer);
	~PMXActor();

	void Update(std::shared_ptr<Matrix> matrix);
	void Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list);

private:
	float _angle = 0; //モデルの角度

	//モデル
	PMXModel* _model;

	//マテリアル関連
	PMXMaterial* _material;
};
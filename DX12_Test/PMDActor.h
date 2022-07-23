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
	//座標変換系
	//Matrix* _transformMatrix;
	//ComPtr<ID3D12DescriptorHeap> _transformHeap = nullptr;//座標変換ヒープ
	//Matrix::Transform* _mappedTransform = nullptr;

	//Matrix::Transform _transform;
	//Matrix::Transform* _mappedTransform = nullptr;
	//ComPtr<ID3D12Resource> _transformBuff = nullptr;

	float _angle = 0; //モデルの角度
	//HRESULT CreateTransformView(ComPtr<ID3D12Device> device);

	//モデル
	Model* _model;

	//マテリアル関連
	Material* _material;
};
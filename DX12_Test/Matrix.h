#pragma once
#include "BaseInclude.h"

class Matrix
{
public:
	Matrix(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);
	void Rotate();

	struct MatricesData
	{
		XMMATRIX world; //モデルの回転行列
		XMMATRIX view; // ビュー行列
		XMMATRIX proj; //プロジェクション行列
		XMFLOAT3 eye; //視点
	};

private:
	XMMATRIX _worldMat; //ワールド行列
	XMMATRIX _viewMat; //ビュー行列
	XMMATRIX _projMat; //プロジェクション行列

	float _angle = 0.0; //角度

	MatricesData* _mapMatrix;//マップ先を示すポインタ

	int _width;
	int _height;

	ComPtr<ID3D12Resource> _constBuffer;

	void Initialize(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);
};
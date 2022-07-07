#pragma once
#include "Vertices.h"

class Matrix
{
public:
	Matrix(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle);
	void Rotate();

	struct MatricesData
	{
		XMMATRIX world; //モデルの回転行列
		XMMATRIX viewproj; //ビューとプロジェクションの合成行列
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


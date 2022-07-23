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
		XMMATRIX world; //モデルの回転行列
		XMMATRIX view; // ビュー行列
		XMMATRIX proj; //プロジェクション行列
		XMFLOAT3 eye; //視点
	};

private:
	XMMATRIX _worldMat; //ワールド行列
	XMMATRIX _viewMat; //ビュー行列
	XMMATRIX _projMat; //プロジェクション行列

	MatricesData* _mapMatrix;//マップ先を示すポインタ
	ComPtr<ID3D12DescriptorHeap> _matrixDescHeap;

	int _width;
	int _height;

	ComPtr<ID3D12Resource> _constBuffer;

	void Initialize(ComPtr<ID3D12Device> device, int width, int height);
};
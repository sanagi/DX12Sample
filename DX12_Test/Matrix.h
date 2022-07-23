#pragma once
#include "BaseInclude.h"

class Matrix
{
public:

	struct SceneData {
		DirectX::XMMATRIX view;//ビュー行列
		DirectX::XMMATRIX proj;//プロジェクション行列
		DirectX::XMFLOAT3 eye;//視点座標
	};

	struct Transform {
		//内部に持ってるXMMATRIXメンバが16バイトアライメントであるため
		//Transformをnewする際には16バイト境界に確保する
		void* operator new(size_t size);
		XMMATRIX world;
	};

	Matrix();
	~Matrix();

	HRESULT CreateSceneView(ComPtr<ID3D12Device> device, int width, int height);
	HRESULT CreateTransformView(ComPtr<ID3D12Device> device);

	void Rotate(float angle);

	SceneData* GetMappedSceneData();
	ComPtr<ID3D12DescriptorHeap> GetSceneDescHeap();

	Transform* GetTransformData();
	ComPtr<ID3D12DescriptorHeap> GetTransformHeap();

private:
	SceneData* _mappedSceneData;
	ComPtr<ID3D12DescriptorHeap> _sceneDescHeap;

	Transform* _mappedTransformData;
	ComPtr<ID3D12DescriptorHeap> _transformDescHeap;
};


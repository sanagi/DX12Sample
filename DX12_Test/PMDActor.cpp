#include "PMDActor.h"

const int MATERIAL_DESC_SIZE = 4; //マテリアル、基本テクスチャ、スフィア2種

#pragma region コンストラクタ系

PMDActor::PMDActor(ComPtr<ID3D12Device> device, const char* filepath, PMDRenderer renderer) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/巡音ルカ.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//モデル生成
	_model = new Model(fp, device, "rb");

	//マテリアル作成
	_material = new Material(device, fp, filepath, MATERIAL_DESC_SIZE, renderer);

	fclose(fp);
}


PMDActor::~PMDActor()
{
}

#pragma endregion

#pragma region 描画ループ

void PMDActor::Update(std::shared_ptr<Matrix> matrix) {
	_angle += 0.01f;
	matrix->Rotate(_angle);
}

void PMDActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//モデル描画
	_model->SetRenderBuffer(command_list);

	//マテリアル描画
	_material->Draw(device, command_list, MATERIAL_DESC_SIZE);
	
}

#pragma endregion

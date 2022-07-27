#include "PMXActor.h"
#include <array>
#include <fstream>

#pragma region コンストラクタ系

PMXActor::PMXActor(ComPtr<ID3D12Device> device, const char* filepath, PMXRenderer renderer) : _angle(0.0f)
{
	//モデル生成
	_model = new PMXModel();
	//マテリアル作成
	_material = new PMXMaterial(renderer);

	//ファイルを読んでモデルとマテリアルの中身を初期化
	Open(device, filepath);
	_model->CreateResource(device);
	_material->CreateMaterialDataForGPU(device, filepath);
	_material->CreateResource(device, PMXRenderer::TOON_MATERIAL_DESC_SIZE);
}


PMXActor::~PMXActor()
{
}

#pragma endregion

#pragma region ファイルを開く,リソース生成

void PMXActor::Open(ComPtr<ID3D12Device> device, const char* modelName) {
	HRESULT hr{};

	auto pmxFile = std::ifstream{ modelName, (std::ios::binary | std::ios::in) };
#pragma region ヘッダーを読む

	// ヘッダー -------------------------------
	std::array<byte, 4> pmxHeader{};
	constexpr std::array<byte, 4> PMX_MAGIC_NUMBER{ 0x50, 0x4d, 0x58, 0x20 };
	enum HeaderDataIndex
	{
		ENCODING_FORMAT,
		NUMBER_OF_ADD_UV,
		VERTEX_INDEX_SIZE,
		TEXTURE_INDEX_SIZE,
		MATERIAL_INDEX_SIZE,
		BONE_INDEX_SIZE,
		RIGID_BODY_INDEX_SIZE
	};

	for (int i = 0; i < 4; i++)
	{
		pmxHeader[i] = pmxFile.get();
	}
	if (pmxHeader != PMX_MAGIC_NUMBER)
	{
		pmxFile.close();
	}

	// ver2.0以外は非対応
	float version{};
	pmxFile.read(reinterpret_cast<char*>(&version), 4);
	if (!XMScalarNearEqual(version, 2.0f, g_XMEpsilon.f[0]))
	{
		pmxFile.close();
	}

	byte hederDataLength = pmxFile.get();
	if (hederDataLength != 8)
	{
		pmxFile.close();
	}
	std::array<byte, 8> hederData{};
	for (int i = 0; i < hederDataLength; i++)
	{
		hederData[i] = pmxFile.get();
	}
	//UTF-8は非対応
	if (hederData[0] != 0)
	{
		pmxFile.close();
	}

	unsigned arrayLength{};
	for (int i = 0; i < 4; i++)
	{
		pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
		for (unsigned j = 0; j < arrayLength; j++)
		{
			pmxFile.get();
		}
	}

#pragma endregion

#pragma region 頂点
	int numberOfVertex{};
	pmxFile.read(reinterpret_cast<char*>(&numberOfVertex), 4);
	_model->Vertices = std::vector<PMXModel::PMXVertex>(numberOfVertex);

	for (auto i = 0; i < numberOfVertex; i++)
	{
		pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].position), 12);
		pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].normal), 12);
		pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].uv), 8);
		if (hederData[NUMBER_OF_ADD_UV] != 0)
		{
			for (int j = 0; j < hederData[NUMBER_OF_ADD_UV]; ++j)
			{
				pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].additionalUV[j]), 16);
			}
		}

		const byte weightMethod = pmxFile.get();
		switch (weightMethod)
		{
		case PMXModel::PMXVertex::Weight::BDEF1:
			_model->Vertices[i].weight.type = PMXModel::PMXVertex::Weight::BDEF1;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			_model->Vertices[i].weight.born2 = NO_DATA_FLAG;
			_model->Vertices[i].weight.born3 = NO_DATA_FLAG;
			_model->Vertices[i].weight.born4 = NO_DATA_FLAG;
			_model->Vertices[i].weight.weight1 = 1.0f;
			break;

		case PMXModel::PMXVertex::Weight::BDEF2:
			_model->Vertices[i].weight.type = PMXModel::PMXVertex::Weight::BDEF2;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			_model->Vertices[i].weight.born3 = NO_DATA_FLAG;
			_model->Vertices[i].weight.born4 = NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.weight1), 4);
			_model->Vertices[i].weight.weight2 = 1.0f - _model->Vertices[i].weight.weight1;
			break;

		case PMXModel::PMXVertex::Weight::BDEF4:
			_model->Vertices[i].weight.type = PMXModel::PMXVertex::Weight::BDEF4;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born3), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born4), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.weight1), 4);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.weight2), 4);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.weight3), 4);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.weight4), 4);
			break;

		case PMXModel::PMXVertex::Weight::SDEF:
			_model->Vertices[i].weight.type = PMXModel::PMXVertex::Weight::SDEF;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			_model->Vertices[i].weight.born3 = NO_DATA_FLAG;
			_model->Vertices[i].weight.born4 = NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.weight1), 4);
			_model->Vertices[i].weight.weight2 = 1.0f - _model->Vertices[i].weight.weight1;
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.c), 12);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.r0), 12);
			pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].weight.r1), 12);
			break;

		default:
			pmxFile.close();
		}

		pmxFile.read(reinterpret_cast<char*>(&_model->Vertices[i].edgeMagnif), 4);
	}
#pragma endregion

#pragma region インデックス
	int numOfIndex{};
	pmxFile.read(reinterpret_cast<char*>(&numOfIndex), 4);
	_model->Indices = std::vector<unsigned short> (numOfIndex);

	for (int i = 0; i < numOfIndex; i++)
	{
		pmxFile.read(reinterpret_cast<char*>(&_model->Indices[i]), hederData[VERTEX_INDEX_SIZE]);

		if (_model->Indices[i] == NO_DATA_FLAG)
		{
			pmxFile.close();
		}
	}
#pragma endregion

#pragma region テクスチャ
	int numOfTexture{};
	pmxFile.read(reinterpret_cast<char*>(&numOfTexture), 4);
	_material->TexturePathVector = std::vector<std::string>(numOfTexture);

	for (int i = 0; i < numOfTexture; i++)
	{
		std::array<wchar_t, 512> wBuffer{};
		int textSize;
		pmxFile.read(reinterpret_cast<char*>(&textSize), 4);
		pmxFile.read(reinterpret_cast<char*>(&wBuffer), textSize);

		_material->TexturePathVector[i] = std::string(&wBuffer[0], &wBuffer[0] + textSize / 2);
	}
#pragma endregion

#pragma region
	//マテリアル数
	pmxFile.read(reinterpret_cast<char*>(&_material->MaterialNum), 4);

	_material->MaterialVector = std::vector <PMXMaterial::MaterialData > (_material->MaterialNum);
	_material->PmxMaterialVector = std::vector<PMXMaterial::PMXMaterialData>(_material->MaterialNum);
	_material->_textureVector = std::vector<ComPtr<ID3D12Resource>>(_material->MaterialNum);
	_material->_sphTexVector = std::vector<ComPtr<ID3D12Resource>>(_material->MaterialNum);
	_material->_spaTexVector = std::vector<ComPtr<ID3D12Resource>>(_material->MaterialNum);
	_material->_toonTexVector = std::vector<ComPtr<ID3D12Resource>>(_material->MaterialNum);

	for (int i = 0; i < _material->MaterialNum; i++)
	{
		//材質名なので飛ばす
		for (int j = 0; j < 2; ++j)
		{
			pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
			for (unsigned i = 0; i < arrayLength; i++)
			{
				pmxFile.get();
			}
		}

		//データ読む
		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].diffuse), 16);
		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].specular), 12);
		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].specularity), 4);
		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].ambient), 12);

		//描画フラグ
		pmxFile.get();

		//エッジ色
		for (int i = 0; i < 16; i++)
		{
			pmxFile.get();
		}

		//エッジサイズ
		for (int i = 0; i < 4; i++)
		{
			pmxFile.get();
		}

		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].colorMapTextureIndex), hederData[TEXTURE_INDEX_SIZE]); //通常テクスチャ, テクスチャテーブルの参照Index
		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].sphereMapTextureIndex), hederData[TEXTURE_INDEX_SIZE]); //スフィアテクスチャ, テクスチャテーブルの参照Index
		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].sphereMode), 1); //スフィアモード 0:無効 1:乗算(sph) 2:加算(spa) 3:サブテクスチャ(追加UV1のx,yをUV参照して通常テクスチャ描画を行う)

		const byte shareToonFlag = pmxFile.get();//共有フラグ
		if (shareToonFlag)
		{
			pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].toonCommonTextureIndex), 1); //共有ToonTextureのインデックス
		}
		else
		{
			pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].toonMapTextureIndex), hederData[TEXTURE_INDEX_SIZE]); //個別ToonTexture
		}

		//メモ欄、飛ばす
		pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
		for (unsigned i = 0; i < arrayLength; i++)
		{
			pmxFile.get();
		}

		pmxFile.read(reinterpret_cast<char*>(&_material->PmxMaterialVector[i].indicesNum), 4);
	}

#pragma endregion

#pragma region ボーン

#pragma endregion
}

/*
void PMXMaterial::Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath) {
	fread(&_materialNum, sizeof(_materialNum), 1, fp); //マテリアル数を読む

	MaterialVector = std::vector<MaterialData>(_materialNum);
	PmxMaterialVector = std::vector<PMXMaterialData>(_materialNum);
	_textureVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	_toonTexVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);

	auto tmp = PmxMaterialVector.size() * sizeof(PMXMaterialData);

	fread(PmxMaterialVector.data(), PmxMaterialVector.size() * sizeof(PMXMaterialData), 1, fp); //全部読む

	//中身をGPU用にコピー
	for (int i = 0; i < PmxMaterialVector.size(); ++i) {
		MaterialVector[i].indicesNum = PmxMaterialVector[i].indicesNum;
		MaterialVector[i].material.diffuse = PmxMaterialVector[i].diffuse;
		MaterialVector[i].material.specular = PmxMaterialVector[i].specular;
		MaterialVector[i].material.specularity = PmxMaterialVector[i].specularity;
		MaterialVector[i].material.ambient = PmxMaterialVector[i].ambient;
		MaterialVector[i].additionarl.toonIdx = PmxMaterialVector[i].toonCommonTextureIndex;
	}

	//テクスチャを読み込んでテクスチャリソースバッファの配列を得る
	for (int i = 0; i < PmxMaterialVector.size(); i++) {

		//トゥーンリソースの読み込み
		char toonFilePath[32];
		sprintf_s(toonFilePath, "toon%02d.bmp", PmxMaterialVector[i].toonCommonTextureIndex + 1);
		auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, toonFilePath);
		_toonTexVector[i] = Texture::LoadTextureFromFile(device, texFilePath, _resourceTable);

		if (strlen(PmxMaterialVector[i].texFilePath) == 0) {
			_textureVector[i] = nullptr;
		}
		else
		{
			string texFileName = PmxMaterialVector[i].texFilePath;
			string sphFileName = "";
			string spaFileName = "";
			auto namepair = Texture::SplitFileName(texFileName);
			if (std::count(texFileName.begin(), texFileName.end(), '*') > 0) {
				auto namepair = Texture::SplitFileName(texFileName);
				if (Texture::GetExtension(namepair.first) == "sph") {
					texFileName = namepair.second;
					sphFileName = namepair.first;
				}
				else if (Texture::GetExtension(namepair.first) == "spa") {
					texFileName = namepair.second;
					spaFileName = namepair.first;
				}
				else {
					texFileName = namepair.first;
					if (Texture::GetExtension(namepair.second) == "sph") {
						sphFileName = namepair.second;
					}
					else if (Texture::GetExtension(namepair.first) == "spa") {
						spaFileName = namepair.second;
					}
				}
			}
			else {
				if (Texture::GetExtension(PmxMaterialVector[i].texFilePath) == "sph") {
					sphFileName = PmxMaterialVector[i].texFilePath;
					texFileName = "";
				}
				else if (Texture::GetExtension(PmxMaterialVector[i].texFilePath) == "spa") {
					spaFileName = PmxMaterialVector[i].texFilePath;
					texFileName = "";
				}
				else {
					texFileName = PmxMaterialVector[i].texFilePath;
				}
			}
			if (texFileName != "") {
				auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, texFileName.c_str());
				_textureVector[i] = Texture::LoadTextureFromFile(device, texFilePath, _resourceTable);
			}
			if (sphFileName != "") {
				auto sphFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, sphFileName.c_str());
				_sphTexVector[i] = Texture::LoadTextureFromFile(device, sphFilePath, _resourceTable);
			}
			if (spaFileName != "") {
				auto spaFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, spaFileName.c_str());
				_spaTexVector[i] = Texture::LoadTextureFromFile(device, spaFilePath, _resourceTable);
			}
		}

	}
}*/

#pragma region 描画ループ

void PMXActor::Update(std::shared_ptr<Matrix> matrix) {
	_angle += 0.01f;
	//matrix->Rotate(_angle);
}

void PMXActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//モデル描画
	_model->SetRenderBuffer(command_list);
	//マテリアル描画
	_material->Draw(device, command_list, PMXRenderer::TOON_MATERIAL_DESC_SIZE);
}

#pragma endregion

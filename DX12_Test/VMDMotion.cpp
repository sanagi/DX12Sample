#include "VMDMotion.h"
#include <algorithm>

#pragma region コンストラクタ

VMDMotion::VMDMotion(const char* filepath) {
	LoadVMDFile(filepath);
}

VMDMotion::~VMDMotion() {

}

#pragma endregion

#pragma region ロード

void VMDMotion::LoadVMDFile(const char* filepath) {
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}

	fseek(fp, 50, SEEK_SET);//最初の50バイトは飛ばしてOK

	unsigned int keyframeNum = 0;
	fread(&keyframeNum, sizeof(keyframeNum), 1, fp); //モーションデータ数を読む

	struct VMDKeyFrame
	{
		char boneName[15]; //ボーン名
		unsigned int frameNo; //フレーム番号()
		XMFLOAT3 location; //位置
		XMFLOAT4 quaternion; //回転
		unsigned char bezier[64]; //ベジェ補完パラメータ
	};
	vector<VMDKeyFrame> keyFrameData(keyframeNum);

	//読み込み開始
	for (auto& keyframe : keyFrameData) {
		fread(keyframe.boneName, sizeof(keyframe.boneName), 1, fp);//ボーン名
		fread(&keyframe.frameNo, sizeof(keyframe.frameNo) +//フレーム番号
			sizeof(keyframe.location) +//位置(IKのときに使用予定)
			sizeof(keyframe.quaternion) +//クオータニオン
			sizeof(keyframe.bezier), 1, fp);//補間ベジェデータ
		_duration = std::max<unsigned int>(_duration, keyframe.frameNo);
	}

	//VMDキーフレームから使用するテーブルへ変換
	for (auto& f : keyFrameData) {
		_motiondata[f.boneName].emplace_back(
			KeyFrame(
				f.frameNo,
				XMLoadFloat4(&f.quaternion),
				XMFLOAT2((float)f.bezier[3] / 127.0f, (float)f.bezier[7] / 127.0f),
				XMFLOAT2((float)f.bezier[11] / 127.0f, (float)f.bezier[15] / 127.0f)
			)
		);
	}

	//モーションデータのソート
	for (auto& motion : _motiondata) {
		sort(motion.second.begin(), motion.second.end(),
			[](const KeyFrame& lval, const KeyFrame& rval) {
			return lval.frameNo <= rval.frameNo;
		});
	}
}

#pragma endregion

#pragma region アニメーション系

float VMDMotion::GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n) {
	if (a.x == a.y && b.x == b.y)return x;//計算不要
	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x;//t^3の係数
	const float k1 = 3 * b.x - 6 * a.x;//t^2の係数
	const float k2 = 3 * a.x;//tの係数

	//誤差の範囲内かどうかに使用する定数
	constexpr float epsilon = 0.0005f;

	for (int i = 0; i < n; ++i) {
		//f(t)求めまーす
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;
		//もし結果が0に近い(誤差の範囲内)なら打ち切り
		if (ft <= epsilon && ft >= -epsilon)break;

		t -= ft / 2;
	}
	//既に求めたいtは求めているのでyを計算する
	auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}

void VMDMotion::SetPMDBone(PMDBone*& pmdbone) {
	_pmdBone = pmdbone;
}

/// <summary>
/// クオータニオン回転
/// </summary>
void VMDMotion::SetQuaternionForPMDBone()
{
	for (auto& bonemotion : _motiondata) {
		auto node = _pmdBone->BoneNodeTable[bonemotion.first]; //最初のノード取得
		auto& pos = node.startPos;

		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z) *
			XMMatrixRotationQuaternion(bonemotion.second[0].quaternion) *
			XMMatrixTranslation(pos.x, pos.y, pos.z);
		_pmdBone->BoneMatrices[node.boneIdx] = mat;
	}
}

/// <summary>
/// アニメーション開始
/// </summary>
void VMDMotion::PlayAnimation() {
	_startTime = timeGetTime();
}

/// <summary>
/// モーション更新
/// </summary>
void VMDMotion::UpdateMotion() {
	auto elapsedTime = timeGetTime() - _startTime;//経過時間を測る
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	if (frameNo > _duration) {
		_startTime = timeGetTime();
		frameNo = 0;
	}

	//行列情報クリア(してないと前フレームのポーズが重ね掛けされてモデルが壊れる)
	std::fill(_pmdBone->BoneMatrices.begin(), _pmdBone->BoneMatrices.end(), XMMatrixIdentity());

	//モーションデータ更新
	for (auto& bonemotion : _motiondata) {
		auto itBoneNode = _pmdBone->BoneNodeTable.find(bonemotion.first);
		if (itBoneNode == _pmdBone->BoneNodeTable.end()) {
			continue;
		}
		auto node = _pmdBone->BoneNodeTable[bonemotion.first];
		//合致するものを探す
		auto keyframes = bonemotion.second;

		auto rit = find_if(keyframes.rbegin(), keyframes.rend(), [frameNo](const KeyFrame& keyframe) {
			return keyframe.frameNo <= frameNo;
		});
		if (rit == keyframes.rend())continue;//合致するものがなければ飛ばす
		XMMATRIX rotation;
		auto it = rit.base();
		if (it != keyframes.end()) {
			auto t = static_cast<float>(frameNo - rit->frameNo) /
				static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->p1, it->p2, 12);

			rotation = XMMatrixRotationQuaternion(
				XMQuaternionSlerp(rit->quaternion, it->quaternion, t)
			);
		}
		else {
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}

		//ボーンの回転を適用
		auto& pos = node.startPos;
		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z) * //原点に戻し
			rotation * //回転
			XMMatrixTranslation(pos.x, pos.y, pos.z);//元の座標に戻す
		_pmdBone->BoneMatrices[node.boneIdx] = mat;
	}

	_pmdBone->RecursiveMatrixMultiply(&_pmdBone->BoneNodeTable["センター"], XMMatrixIdentity());
	copy(_pmdBone->BoneMatrices.begin(), _pmdBone->BoneMatrices.end(), _pmdBone->BoneMappedMatrix);
}

#pragma endregion
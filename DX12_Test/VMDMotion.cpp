#include "VMDMotion.h"
#include <algorithm>

#pragma region �R���X�g���N�^

VMDMotion::VMDMotion(const char* filepath) {
	LoadVMDFile(filepath);
}

VMDMotion::~VMDMotion() {

}

#pragma endregion

#pragma region ���[�h

void VMDMotion::LoadVMDFile(const char* filepath) {
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}

	fseek(fp, 50, SEEK_SET);//�ŏ���50�o�C�g�͔�΂���OK

	unsigned int keyframeNum = 0;
	fread(&keyframeNum, sizeof(keyframeNum), 1, fp); //���[�V�����f�[�^����ǂ�

	struct VMDKeyFrame
	{
		char boneName[15]; //�{�[����
		unsigned int frameNo; //�t���[���ԍ�()
		XMFLOAT3 location; //�ʒu
		XMFLOAT4 quaternion; //��]
		unsigned char bezier[64]; //�x�W�F�⊮�p�����[�^
	};
	vector<VMDKeyFrame> keyFrameData(keyframeNum);

	//�ǂݍ��݊J�n
	for (auto& keyframe : keyFrameData) {
		fread(keyframe.boneName, sizeof(keyframe.boneName), 1, fp);//�{�[����
		fread(&keyframe.frameNo, sizeof(keyframe.frameNo) +//�t���[���ԍ�
			sizeof(keyframe.location) +//�ʒu(IK�̂Ƃ��Ɏg�p�\��)
			sizeof(keyframe.quaternion) +//�N�I�[�^�j�I��
			sizeof(keyframe.bezier), 1, fp);//��ԃx�W�F�f�[�^
		_duration = std::max<unsigned int>(_duration, keyframe.frameNo);
	}

	//VMD�L�[�t���[������g�p����e�[�u���֕ϊ�
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

	//���[�V�����f�[�^�̃\�[�g
	for (auto& motion : _motiondata) {
		sort(motion.second.begin(), motion.second.end(),
			[](const KeyFrame& lval, const KeyFrame& rval) {
			return lval.frameNo <= rval.frameNo;
		});
	}
}

#pragma endregion

#pragma region �A�j���[�V�����n

float VMDMotion::GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n) {
	if (a.x == a.y && b.x == b.y)return x;//�v�Z�s�v
	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x;//t^3�̌W��
	const float k1 = 3 * b.x - 6 * a.x;//t^2�̌W��
	const float k2 = 3 * a.x;//t�̌W��

	//�덷�͈͓̔����ǂ����Ɏg�p����萔
	constexpr float epsilon = 0.0005f;

	for (int i = 0; i < n; ++i) {
		//f(t)���߂܁[��
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;
		//�������ʂ�0�ɋ߂�(�덷�͈͓̔�)�Ȃ�ł��؂�
		if (ft <= epsilon && ft >= -epsilon)break;

		t -= ft / 2;
	}
	//���ɋ��߂���t�͋��߂Ă���̂�y���v�Z����
	auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}

void VMDMotion::SetPMDBone(PMDBone*& pmdbone) {
	_pmdBone = pmdbone;
}

/// <summary>
/// �N�I�[�^�j�I����]
/// </summary>
void VMDMotion::SetQuaternionForPMDBone()
{
	for (auto& bonemotion : _motiondata) {
		auto node = _pmdBone->BoneNodeTable[bonemotion.first]; //�ŏ��̃m�[�h�擾
		auto& pos = node.startPos;

		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z) *
			XMMatrixRotationQuaternion(bonemotion.second[0].quaternion) *
			XMMatrixTranslation(pos.x, pos.y, pos.z);
		_pmdBone->BoneMatrices[node.boneIdx] = mat;
	}
}

/// <summary>
/// �A�j���[�V�����J�n
/// </summary>
void VMDMotion::PlayAnimation() {
	_startTime = timeGetTime();
}

/// <summary>
/// ���[�V�����X�V
/// </summary>
void VMDMotion::UpdateMotion() {
	auto elapsedTime = timeGetTime() - _startTime;//�o�ߎ��Ԃ𑪂�
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	if (frameNo > _duration) {
		_startTime = timeGetTime();
		frameNo = 0;
	}

	//�s����N���A(���ĂȂ��ƑO�t���[���̃|�[�Y���d�ˊ|������ă��f��������)
	std::fill(_pmdBone->BoneMatrices.begin(), _pmdBone->BoneMatrices.end(), XMMatrixIdentity());

	//���[�V�����f�[�^�X�V
	for (auto& bonemotion : _motiondata) {
		auto itBoneNode = _pmdBone->BoneNodeTable.find(bonemotion.first);
		if (itBoneNode == _pmdBone->BoneNodeTable.end()) {
			continue;
		}
		auto node = _pmdBone->BoneNodeTable[bonemotion.first];
		//���v������̂�T��
		auto keyframes = bonemotion.second;

		auto rit = find_if(keyframes.rbegin(), keyframes.rend(), [frameNo](const KeyFrame& keyframe) {
			return keyframe.frameNo <= frameNo;
		});
		if (rit == keyframes.rend())continue;//���v������̂��Ȃ���Δ�΂�
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

		//�{�[���̉�]��K�p
		auto& pos = node.startPos;
		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z) * //���_�ɖ߂�
			rotation * //��]
			XMMatrixTranslation(pos.x, pos.y, pos.z);//���̍��W�ɖ߂�
		_pmdBone->BoneMatrices[node.boneIdx] = mat;
	}

	_pmdBone->RecursiveMatrixMultiply(&_pmdBone->BoneNodeTable["�Z���^�["], XMMatrixIdentity());
	copy(_pmdBone->BoneMatrices.begin(), _pmdBone->BoneMatrices.end(), _pmdBone->BoneMappedMatrix);
}

#pragma endregion
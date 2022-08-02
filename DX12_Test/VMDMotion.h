#pragma once
#include "BaseInclude.h"
#include "PMDBone.h"
#include <unordered_map>
#include <timeapi.h>

class VMDMotion
{
private:
	PMDBone* _pmdBone;
	DWORD _startTime;
	int _duration;
	float GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n);
public:
	VMDMotion(const char* filepath);
	~VMDMotion();
	
	struct KeyFrame {
		unsigned int frameNo; //�t���[����
		XMVECTOR quaternion; //�N�I�[�^�j�I��
		XMFLOAT2 p1, p2; //�x�W�F�̃R���g���[���|�C���g
		KeyFrame(
			unsigned int fno,
			const DirectX::XMVECTOR& q,
			const DirectX::XMFLOAT2& ip1,
			const DirectX::XMFLOAT2& ip2) :
			frameNo(fno),
			quaternion(q),
			p1(ip1),
			p2(ip2) {}
	};

	std::unordered_map<std::string, std::vector<KeyFrame>> _motiondata;
	
	void LoadVMDFile(const char* filePath);
	void SetPMDBone(PMDBone*& pmdBone);
	void SetQuaternionForPMDBone();
	void UpdateMotion();
	void PlayAnimation();
};


#include "D3D12Manager.h"
#include "Matrix.h"
#include "PMDRenderer.h"
#include "PMDActor.h"
#include "PMXRenderer.h"
#include "PMXActor.h"

#pragma once
class Application
{
private:

	//�E�B���h�E����
	WNDCLASSEX _windowClass;
	HWND _hwnd;
	std::shared_ptr<D3D12Manager> _dx12;
	std::shared_ptr<Matrix> _matrix;

	std::shared_ptr<PMDRenderer> _pmdRenderer;
	std::shared_ptr<PMDActor> _pmdActor;

	std::shared_ptr<PMXRenderer> _pmxRenderer;
	std::shared_ptr<PMXActor> _pmxActor;

	//�Q�[���p�E�B���h�E�̐���
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	//���V���O���g���̂��߂ɃR���X�g���N�^��private��
	//����ɃR�s�[�Ƒ�����֎~��
	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
public:
	///Application�̃V���O���g���C���X�^���X�𓾂�
	static Application& Instance();

	///������
	bool Init();

	///���[�v�N��
	void Run();

	///�㏈��
	void Terminate();
	SIZE GetWindowSize()const;
	~Application();
};


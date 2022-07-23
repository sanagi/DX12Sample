#include "Application.h"

#pragma region Method

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	auto& app = Application::Instance();
	if (!app.Init()) {
		return -1;
	}
	app.Run();
	app.Terminate();
	return 0;

	/*WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //�R�[���o�b�N�w��
	w.lpszClassName = _T("DXSample");	//�A�v���P�[�V�����N���X��
	w.hInstance = GetModuleHandle(nullptr);	//�n���h���̎擾

	RegisterClassEx(&w);	//�E�B���h�E�N���X�̎w���os�ɓ`����
	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };	//�E�B���h�E�̃T�C�Y
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);	//�T�C�Y�␳

	//�E�B���h�E����
	HWND hwnd = CreateWindow(
		w.lpszClassName,	//�N���X��
		_T("DX12Test"),	//�^�C�g����
		WS_OVERLAPPEDWINDOW,	//���E������
		CW_USEDEFAULT,	//�\��x��os�C��
		CW_USEDEFAULT,	//�\��y��os�C��
		wrc.right - wrc.left,	//��
		wrc.bottom - wrc.top,	//�邳
		nullptr,	//�e�E�B���h�E�n���h��
		nullptr,	//���j���[�n���h��
		w.hInstance,	//�Ăяo���A�v���n���h��
		nullptr	//�ǉ��p�����[�^
		);

	//�}�l�[�W���[������
	D3D12Manager direct_3d(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT, L"SimpleVertexShader.hlsl", L"SimplePixelShader.hlsl");

	//�\��
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			break;
		}

		direct_3d.Render();
	}

	UnregisterClass(w.lpszClassName, w.hInstance);
	//DebugOutputFormatString("Show window test.");
	getchar();
	return 0;*/
}

#pragma endregion
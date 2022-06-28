#include "D3D12Manager.h"
#include "Vertices.h"

#pragma region Constant

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

#pragma endregion

#pragma region Member



#pragma endregion

#pragma region Method

/*void DebugOutputFormatString(const char* format)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}*/

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//�E�B���h�E�̔j���ŌĂ΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	WNDCLASSEX w = {};
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

	//���_�N���X������
	/*Vertices vert(direct_3d.Dev);

	//����������`
	std::function<void(ComPtr<ID3D12GraphicsCommandList>)> DrawCallBack = {
		vert.Draw(direct_3d.CommandList.Get()) 
	};*/

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
	return 0;
}

#pragma endregion

/*
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASSEX	wc{};
	HWND hwnd{};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("D3D12");
	wc.hIcon = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (RegisterClassEx(&wc) == 0) {
		return -1;
	}

	//�E�C���h�E�쐬
	hwnd = CreateWindowEx(
		WS_EX_COMPOSITED,
		TEXT("D3D12"),
		TEXT("D3D12"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		NULL,
		NULL,
		hInst,
		NULL);

	{
		int screen_width = GetSystemMetrics(SM_CXSCREEN);
		int screen_height = GetSystemMetrics(SM_CYSCREEN);
		RECT rect{};
		GetClientRect(hwnd, &rect);
		MoveWindow(
			hwnd,
			(screen_width / 2) - ((WINDOW_WIDTH + (WINDOW_WIDTH - rect.right)) / 2), //�E�C���h�E�����
			(screen_height / 2) - ((WINDOW_HEIGHT + (WINDOW_HEIGHT - rect.bottom)) / 2),//�����Ɏ����Ă���
			WINDOW_WIDTH + (WINDOW_WIDTH - rect.right),
			WINDOW_HEIGHT + (WINDOW_HEIGHT - rect.bottom),
			TRUE);
	}

	//�E�C���h�E���J���ăA�b�v�f�[�g
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//���b�Z�[�W���[�v
	while (TRUE) {
		MSG msg{};
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) { break; }
		}

	}

	UnregisterClass(TEXT("D3D12"), NULL); hwnd = NULL;

	return 0;
}
*/
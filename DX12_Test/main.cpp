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
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック指定
	w.lpszClassName = _T("DXSample");	//アプリケーションクラス名
	w.hInstance = GetModuleHandle(nullptr);	//ハンドルの取得

	RegisterClassEx(&w);	//ウィンドウクラスの指定をosに伝える
	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };	//ウィンドウのサイズ
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);	//サイズ補正

	//ウィンドウ生成
	HWND hwnd = CreateWindow(
		w.lpszClassName,	//クラス名
		_T("DX12Test"),	//タイトル名
		WS_OVERLAPPEDWINDOW,	//境界線あり
		CW_USEDEFAULT,	//表示xはos任せ
		CW_USEDEFAULT,	//表示yはos任せ
		wrc.right - wrc.left,	//幅
		wrc.bottom - wrc.top,	//鷹さ
		nullptr,	//親ウィンドウハンドル
		nullptr,	//メニューハンドル
		w.hInstance,	//呼び出しアプリハンドル
		nullptr	//追加パラメータ
		);

	//マネージャー初期化
	D3D12Manager direct_3d(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT, L"SimpleVertexShader.hlsl", L"SimplePixelShader.hlsl");

	//表示
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
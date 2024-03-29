#include "Application.h"

//ウィンドウ定数
const unsigned int window_width = 1280;
const unsigned int window_height = 720;

//面倒だけど書かなあかんやつ
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {//ウィンドウが破棄されたら呼ばれます
		PostQuitMessage(0);//OSに対して「もうこのアプリは終わるんや」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//規定の処理を行う
}

void Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass) {
	HINSTANCE hInst = GetModuleHandle(nullptr);
	//ウィンドウクラス生成＆登録
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
	windowClass.lpszClassName = _T("DirectXTest");//アプリケーションクラス名(適当でいいです)
	windowClass.hInstance = GetModuleHandle(0);//ハンドルの取得
	RegisterClassEx(&windowClass);//アプリケーションクラス(こういうの作るからよろしくってOSに予告する)

	RECT wrc = { 0,0, window_width, window_height };//ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//ウィンドウのサイズはちょっと面倒なので関数を使って補正する
	//ウィンドウオブジェクトの生成
	hwnd = CreateWindow(windowClass.lpszClassName,//クラス名指定
		_T("DX12"),//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウです
		CW_USEDEFAULT,//表示X座標はOSにお任せします
		CW_USEDEFAULT,//表示Y座標はOSにお任せします
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,//親ウィンドウハンドル
		nullptr,//メニューハンドル
		windowClass.hInstance,//呼び出しアプリケーションハンドル
		nullptr);//追加パラメータ
}

SIZE Application::GetWindowSize()const {
	SIZE ret;
	ret.cx = window_width;
	ret.cy = window_height;
	return ret;
}

void Application::Run() {
	ShowWindow(_hwnd, SW_SHOW);//ウィンドウ表示
	float angle = 0.0f;
	MSG msg = {};
	unsigned int frame = 0;
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//もうアプリケーションが終わるって時にmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}

		if (_dx12 != nullptr) {
			//全体の描画準備
			_dx12->BeginDraw();

			//PMD用の描画パイプラインに合わせる
			_dx12->SetRenderer(_pmdRenderer->GetPipelineState(), _pmdRenderer->GetRootSignature());

			//PMX用の描画パイプラインに合わせる
			//_dx12->SetRenderer(_pmxRenderer->GetPipelineState(), _pmxRenderer->GetRootSignature());

			_dx12->SetScene(_matrix);

			_pmdActor->Update(_dx12->GetCommandList(), _matrix);
			_pmdActor->Draw(_dx12->GetDevice(), _dx12->GetCommandList());

			//_pmxActor->Update(_matrix);
			//_pmxActor->Draw(_dx12->GetDevice(), _dx12->GetCommandList());

			_dx12->EndDraw();
		}
	}
}

bool Application::Init() {
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);
	CreateGameWindow(_hwnd, _windowClass);

	//DirectX12ラッパー生成＆初期化
	_dx12.reset(new D3D12Manager(_hwnd));
	_matrix.reset(new Matrix(_dx12->GetDevice(), window_width, window_height));
	_pmdRenderer.reset(new PMDRenderer(_dx12->GetDevice(), L"PMDToonVertexShader.hlsl", L"PMDToonPixelShader.hlsl"));
	_pmdActor.reset(new PMDActor(_dx12->GetDevice(), "Model/Lat式ミクVer2.31_Normal.pmd", "motion/DeepBlueTown_he_Oideyo_dance.vmd", *_pmdRenderer, false));
	//_pmdActor.reset(new PMDActor(_dx12->GetDevice(), "Model/初音ミク.pmd", "motion/swing.vmd", *_pmdRenderer, true));
	
	//_pmxRenderer.reset(new PMXRenderer(_dx12->GetDevice(), L"PMXToonVertexShader.hlsl", L"PMXToonPixelShader.hlsl"));
	//_pmxActor.reset(new PMXActor(_dx12->GetDevice(), "Model/Appearance Miku/Appearance Miku.pmx", *_pmxRenderer));
	
	return true;
}

void Application::Terminate() {
	//もうクラス使わんから登録解除してや
	UnregisterClass(_windowClass.lpszClassName, _windowClass.hInstance);
}

Application& Application::Instance() {
	static Application instance;
	return instance;
}

Application::Application()
{
}


Application::~Application()
{
}
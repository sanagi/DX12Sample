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

	//ウィンドウ周り
	WNDCLASSEX _windowClass;
	HWND _hwnd;
	std::shared_ptr<D3D12Manager> _dx12;
	std::shared_ptr<Matrix> _matrix;

	std::shared_ptr<PMDRenderer> _pmdRenderer;
	std::shared_ptr<PMDActor> _pmdActor;

	std::shared_ptr<PMXRenderer> _pmxRenderer;
	std::shared_ptr<PMXActor> _pmxActor;

	//ゲーム用ウィンドウの生成
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	//↓シングルトンのためにコンストラクタをprivateに
	//さらにコピーと代入を禁止に
	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
public:
	///Applicationのシングルトンインスタンスを得る
	static Application& Instance();

	///初期化
	bool Init();

	///ループ起動
	void Run();

	///後処理
	void Terminate();
	SIZE GetWindowSize()const;
	~Application();
};


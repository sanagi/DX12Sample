#pragma once

#include <Windows.h>
#include <stdlib.h>
#include<tchar.h>
#include <stdio.h>
#include <d3d12.h>
#include <DirectXTex.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <d3d12shader.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include <vector>
#include <wrl/client.h>
#include <string>
#include <map>

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")	// winmm.libを使用する

using namespace std;
using namespace DirectX;
//ComPtr(DirectX版のスマートポインタ)
using Microsoft::WRL::ComPtr;

#ifdef _DEBUG
#include <iostream>
#endif
#include "BaseInclude.h"
const int RTV_NUM = 2; //�����_�[�^�[�Q�b�g�̖���
const FLOAT CLEAR_COLOR[4] = { 1.0f, 0.0f, 0.20f, 1.0f }; // �N���A����F

class D3D12Manager
{
public:
	
	D3D12Manager(HWND hwnd, int width, int height, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	~D3D12Manager();

	HRESULT Render(std::function<void(ID3D12GraphicsCommandList*)> draw);

private:

	//�T�C�Y
	int _width;
	int _height;

	//�E�B���h�E�̃n���h��
	HWND _windowHandle;

	ComPtr<ID3D12Device> _dev = nullptr;
	ComPtr<IDXGIFactory4> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;

	//�R�}���h���X�g
	ComPtr<ID3D12GraphicsCommandList> _commandList;
	ComPtr<ID3D12CommandAllocator>	_commandAllocator;
	//�R�}���h�L���[
	ComPtr<ID3D12CommandQueue>	_commandQueue;

	//RTV�p�q�[�v
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	//RTV�p���\�[�X
	ComPtr<ID3D12Resource> _rtvBuffers[RTV_NUM];

	//Depth�X�e���V���p�q�[�v
	ComPtr<ID3D12DescriptorHeap> _dsvHeaps = nullptr;

	//Descriptor�̃C���N�������g�T�C�Y
	UINT _descHandleIncSize;

	//�t�F���X
	ComPtr<ID3D12Fence> _queueFence;

	//���[�g�V�O�l�`��
	ComPtr<ID3D12RootSignature>	_rootSignature;

	//�p�C�v���C���X�e�[�g
	ComPtr<ID3D12PipelineState> _pipelineState;

	//�֐�

	//����
	HRESULT CreateDevice();
	HRESULT CreateFactory();
	HRESULT CreateCommandList();
	HRESULT CreateCommandQueue();
	HRESULT CreateFence();
	HRESULT CreateSwapChain();
	HRESULT CreateRenderTargetView();
	HRESULT CreateDepthStencilBuffer();
	HRESULT CreateRootSignature();
	HRESULT CreatePipelineStateObject(LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);

	//�`��
	HRESULT StackDrawCommandList(std::function<void(ID3D12GraphicsCommandList*)> draw);
	HRESULT WaitForPreviousFrame();

	//void DrawCallback(std::function<void(ID3D12GraphicsCommandList*)> draw);
};


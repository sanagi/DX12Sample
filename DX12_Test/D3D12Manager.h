#include "BaseInclude.h"
#include "Vertices.h"
#include "Texture.h"
#include "Matrix.h"
#include "Model.h"

const int RTV_NUM = 2; //�����_�[�^�[�Q�b�g�̖���
const FLOAT CLEAR_COLOR[4] = { 1.0f, 1.0f, 1.00f, 1.0f }; // �N���A����F

class D3D12Manager
{
public:

	D3D12Manager(HWND hwnd, int width, int height, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	~D3D12Manager();

	HRESULT Render();

	ComPtr<ID3D12Device> Dev = nullptr;
	//�R�}���h���X�g
	ComPtr<ID3D12GraphicsCommandList> CommandList;

private:

	//�T�C�Y
	int _width;
	int _height;

	//�E�B���h�E�̃n���h��
	HWND _windowHandle;

	ComPtr<IDXGIFactory4> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;

	ComPtr<ID3D12CommandAllocator>	_commandAllocator;

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
	UINT64 _fenceValue = 0;

	//���[�g�V�O�l�`��
	ComPtr<ID3D12RootSignature>	_rootSignature;

	//�p�C�v���C���X�e�[�g
	ComPtr<ID3D12PipelineState> _pipelineState;

	//�R�}���h�L���[
	ComPtr<ID3D12CommandQueue>	_commandQueue;

	//���\�[�X�p�q�[�v
	ComPtr<ID3D12DescriptorHeap> _resourceHeaps = nullptr;

	//�V�U�[��`
	D3D12_RECT _scissorRect;
	//�r���[�|�[�g
	D3D12_VIEWPORT _viewPort;

	//�֐�

	//����
	HRESULT CreateDevice();
	HRESULT CreateFactory();
	HRESULT CreateCommandList();
	HRESULT CreateCommandQueue();
	HRESULT CreateFence();
	HRESULT CreateSwapChain();
	HRESULT CreateRenderTargetView();
	HRESULT CreateResourceHeap();
	HRESULT CreateDepthStencilBuffer();
	HRESULT CreateRootSignature();
	HRESULT CreatePipelineStateObject(LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	void CreateViewPortScissorRect();

	//GPU�ւ̓]��(�e�N�X�`��)
	void CopyTexture();

	//�`��
	HRESULT StackDrawCommandList();
	//�R�}���h�҂�
	HRESULT WaitForPreviousFrame();
	//���Z�b�g
	HRESULT ResetCommand(ComPtr<ID3D12PipelineState> pipelineState);

	//�`�悷��I�u�W�F�N�g�B
	//�o�����main���ŃR�[���o�b�N�Ƃ��ł�肽��
	Model* _model;
	Texture* _tex;
	Matrix* _matrix;

	//void DrawCallback(std::function<void(ID3D12GraphicsCommandList*)> draw);
};
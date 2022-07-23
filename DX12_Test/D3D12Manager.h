#pragma once
#include "Matrix.h"

const int RTV_NUM = 2; //�����_�[�^�[�Q�b�g�̖���
const FLOAT CLEAR_COLOR[4] = { 1.0f, 1.0f, 1.00f, 1.0f }; // �N���A����F

class D3D12Manager
{
public:

	D3D12Manager(HWND hwnd);
	~D3D12Manager();

	ComPtr<ID3D12Device> GetDevice();
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();

	//�`��
	void BeginDraw();
	void SetRenderer(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature);
	void SetScene();
	void EndDraw();

private:

	//�T�C�Y
	SIZE _winSize;

	//�E�B���h�E�̃n���h��
	HWND _windowHandle;

	ComPtr<IDXGIFactory4> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;

	//�f�o�C�X
	ComPtr<ID3D12Device> _device = nullptr;
	//�A���P�[�^
	ComPtr<ID3D12CommandAllocator>	_commandAllocator;
	//�R�}���h�L���[
	ComPtr<ID3D12CommandQueue>	_commandQueue;
	//�R�}���h���X�g
	ComPtr<ID3D12GraphicsCommandList> _commandList;

	//RTV�p�q�[�v
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	//Depth�X�e���V���p�q�[�v
	ComPtr<ID3D12DescriptorHeap> _dsvHeaps = nullptr;
	//RTV�p���\�[�X
	std::vector<ID3D12Resource*> _backBuffers;

	//�V�U�[��`
	std::unique_ptr<D3D12_RECT> _scissorRect;
	//�r���[�|�[�g
	std::unique_ptr<D3D12_VIEWPORT> _viewPort;

	//�V�[�����\������s��
	Matrix* _sceneMatrix;
	//Matrix::SceneData* _mappedSceneData;
	//ComPtr<ID3D12DescriptorHeap> _sceneDescHeap = nullptr;

	//�V�[�����\������o�b�t�@�܂��
	ComPtr<ID3D12Resource> _sceneConstBuff = nullptr;

	struct SceneData {
		DirectX::XMMATRIX view;//�r���[�s��
		DirectX::XMMATRIX proj;//�v���W�F�N�V�����s��
		DirectX::XMFLOAT3 eye;//���_���W
	};
	SceneData* _mappedSceneData;
	ComPtr<ID3D12DescriptorHeap> _sceneDescHeap = nullptr;

	//�t�F���X
	ComPtr<ID3D12Fence> _queueFence;
	UINT64 _fenceValue = 0;

	//�֐�
	//����������
	HRESULT CreateDevice(); //�f�o�C�X����
	HRESULT CreateFactory(); //Factory����

	//�R�}���h�n
	HRESULT CreateCommandList(); //�R�}���h���X�g�쐬
	HRESULT CreateCommandQueue();//�R�}���h�L���[�쐬
	
	//�t�F���X�A�X���b�v�`�F�C��
	HRESULT CreateFence(); //�t�F���X
	HRESULT CreateSwapChain(); //�X���b�v�`�F�C��

	//�V�[���쐬
	void CreateScene(); //�V�[�����ƍ쐬
	HRESULT CreateSceneView();

	//�����_�[�^�[�Q�b�g�n
	HRESULT CreateFinalRenderTargetView();
	HRESULT CreateDepthStencilBuffer();

	//�R�}���h�҂�
	HRESULT WaitForPreviousFrame();
	//���Z�b�g
	HRESULT ResetCommand();
};


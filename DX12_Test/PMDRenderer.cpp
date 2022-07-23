#include "PMDRenderer.h"
#include "Texture.h"

using namespace std;

namespace {
	void PrintErrorBlob(ID3DBlob* blob) {
		assert(blob);
		string err;
		err.resize(blob->GetBufferSize());
		copy_n((char*)blob->GetBufferPointer(), err.size(), err.begin());
	}
}

#pragma region �R���X�g���N�^�n

#pragma endregion


PMDRenderer::PMDRenderer(ComPtr<ID3D12Device> device, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName)
{
	assert(SUCCEEDED(CreateRootSignature(device)));
	assert(SUCCEEDED(CreateGraphicsPipelineForPMD(device, vertexShaderName, pixelShaderName)));
	WhiteTex = Texture::CreateWhiteTexture(device);
	BlackTex = Texture::CreateBlackTexture(device);
	//_gradTex = CreateGrayGradationTexture();
}

PMDRenderer::~PMDRenderer()
{

}

#pragma region �`�惋�[�v

void PMDRenderer::Update() {

}
void PMDRenderer::Draw() {

}

#pragma endregion

#pragma region �����_�[�֌W

/// <summary>
/// �R���p�C���V�F�[�_�̃`�F�b�N
/// </summary>
/// <param name="result"></param>
/// <param name="error"></param>
/// <returns></returns>
bool PMDRenderer::CheckShaderCompileResult(HRESULT result, ID3DBlob* error) {
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(error->GetBufferSize());
			std::copy_n((char*)error->GetBufferPointer(), error->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		return false;
	}
	else {
		return true;
	}
}

//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
//�d�v�B��Ƀ��t�@�N�^����Ȃ炱����F�X��������
HRESULT PMDRenderer::CreateGraphicsPipelineForPMD(ComPtr<ID3D12Device> device, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName) {
	HRESULT hr{};

	ComPtr<ID3DBlob> vertexShader{};
	ComPtr<ID3DBlob> pixelShader{};
	ComPtr<ID3DBlob> errorBlob = nullptr;

#if defined(_DEBUG)
	UINT compile_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compile_flag = 0;
#endif

	//���_�V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(
		vertexShaderName, //�V�F�[�_�[��
		nullptr, //�}�N���I�u�W�F�N�g
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //include�̃I�u�W�F�N�g
		"VSMain", //�V�F�[�_�[�̃��C���֐�
		"vs_5_0", //�V�F�[�_�[�̎��
		compile_flag, //�R���p�C���ݒ�
		0, //�󂯎��p�̃|�C���^�A�h���X(�V�F�[�_�[��0)
		vertexShader.GetAddressOf(), //vs�V�F�[�_�[�̃A�h���X
		errorBlob.GetAddressOf() //�G���[�p�A�h���X
	);
	if (!CheckShaderCompileResult(hr, errorBlob.Get())) {
		assert(0);
		return hr;
	}

	//�s�N�Z���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(
		pixelShaderName,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain",
		"ps_5_0",
		compile_flag,
		0,
		pixelShader.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (!CheckShaderCompileResult(hr, errorBlob.Get())) {
		assert(0);
		return hr;
	}

	//�p�C�v���C���X�e�[�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	//���[�g�V�O�l�`��
	gpipeline.pRootSignature = _rootSignature.Get();

	//�V�F�[�_�[
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());

	//�T���v��
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff

	//�u�����h�̎w��
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	//���X�^���C�U�w��
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�

	//�f�v�X�X�e���V���̎w��
	gpipeline.DepthStencilState.DepthEnable = true; //�f�v�X�e�X�g����
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //�����������̗p
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //�S�ď�������
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.StencilEnable = false; //�X�e���V���e�X�g����

	//�C���v�b�g���C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = _inputLayout; //���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(_inputLayout); //���C�A�E�g�z��
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

	//�����_�[�^�[�Q�b�g�̐ݒ�
	gpipeline.NumRenderTargets = 1;//���͂P�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA

	//�T���v���ݒ�
	gpipeline.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

	//�p�C�v���C���쐬
	hr = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipeline.ReleaseAndGetAddressOf()));

	return hr;
}

//���[�g�V�O�l�`��(�f�B�X�N���v�^�[�e�[�u�����Ǘ��������)�����
HRESULT PMDRenderer::CreateRootSignature(ComPtr<ID3D12Device> device) {
	HRESULT hr{};

	//�f�B�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descTblRanges[4] = {}; //�e�N�X�`��2�ƒ萔��3��
	descTblRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//�萔[b0](�r���[�v���W�F�N�V�����p)
	descTblRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);//�萔[b1](���[���h�A�{�[���p)
	descTblRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);//�萔[b2](�}�e���A���p)
	descTblRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//�e�N�X�`���S��(��{��sph��spa�A�g�D�[��)

	//���[�g�p�����[�^�[
	D3D12_ROOT_PARAMETER rootparam[2] = {};
	CD3DX12_ROOT_PARAMETER rootParams[3] = {};
	rootParams[0].InitAsDescriptorTable(1, &descTblRanges[0]);//�r���[�v���W�F�N�V�����ϊ�
	rootParams[1].InitAsDescriptorTable(1, &descTblRanges[1]);//���[���h�E�{�[���ϊ�
	rootParams[2].InitAsDescriptorTable(2, &descTblRanges[2]);//�}�e���A������

	//�T���v���n�̎w��
	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
	samplerDescs[0].Init(0);
	samplerDescs[1].Init(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	//���[�g�V�O�l�`��
	CD3DX12_ROOT_SIGNATURE_DESC  rootSignatureDesc = {};
	rootSignatureDesc.Init(3, rootParams, 2, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (FAILED(hr)) {
		assert(SUCCEEDED(hr));
		return hr;
	}
	hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(_rootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		assert(SUCCEEDED(hr));
		return hr;
	}
	//rootSigBlob->Release();

	return hr;
}

#pragma endregion

#pragma region �Q�b�^�[

ID3D12PipelineState* PMDRenderer::GetPipelineState() {
	return _pipeline.Get();
}

ID3D12RootSignature* PMDRenderer::GetRootSignature() {
	return _rootSignature.Get();
}

#pragma endregion
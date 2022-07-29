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
	AlphaTex = Texture::CreateAlphaTexture(device);
	BlackTex = Texture::CreateBlackTexture(device);
	GradTex = Texture::CreateGrayGradationTexture(device);
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
	gpipeline.BlendState.AlphaToCoverageEnable = TRUE;
	gpipeline.BlendState.IndependentBlendEnable = FALSE;
	for (int i = 0; i < _countof(gpipeline.BlendState.RenderTarget); ++i)
	{
		//���₷�����邽�ߕϐ���
		auto rt = gpipeline.BlendState.RenderTarget[i];
		rt.BlendEnable = TRUE;
		rt.LogicOpEnable = FALSE;
		rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.LogicOp = D3D12_LOGIC_OP_NOOP;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	//���X�^���C�U�w��
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;//���ʂ̃J�����O

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

	ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //���_���̗񋓂����鎖��`����

	//�f�B�X�N���v�^�����W
	D3D12_DESCRIPTOR_RANGE descTblRange[4] = {}; //�e�N�X�`���ƒ萔�A�{�[���A�}�e���A��

	descTblRange[0].NumDescriptors = 1;//�萔�ЂƂ�
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//��ʂ͒萔
	descTblRange[0].BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�}�e���A���p
	descTblRange[1].NumDescriptors = 1;//�e�N�X�`���ЂƂ�
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//��ʂ͒萔
	descTblRange[1].BaseShaderRegister = 1;//1�ԃX���b�g����
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�e�N�X�`��4
	descTblRange[2].NumDescriptors = TOON_MATERIAL_DESC_SIZE - 1;
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//��ʂ̓e�N�X�`��
	descTblRange[2].BaseShaderRegister = 0;
	descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�{�[��
	descTblRange[3].NumDescriptors = 1;
	descTblRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//��ʂ͒萔
	descTblRange[3].BaseShaderRegister = 2; //2�ԃX���b�g��
	descTblRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//���[�g�p�����[�^�[
	D3D12_ROOT_PARAMETER rootparam[3] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];//�f�X�N���v�^�����W�̃A�h���X
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//�f�X�N���v�^�����W��
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�S�V�F�[�_���猩����

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];//�f�X�N���v�^�����W�̃A�h���X
	rootparam[1].DescriptorTable.NumDescriptorRanges = 2;//�f�X�N���v�^�����W��
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_���猩����

	rootparam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[2].DescriptorTable.pDescriptorRanges = &descTblRange[3];//�f�X�N���v�^�����W�̃A�h���X
	rootparam[2].DescriptorTable.NumDescriptorRanges = 1;//�f�X�N���v�^�����W��
	rootparam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//�o�[�e�b�N�X�V�F�[�_���猩����

	rootSignatureDesc.pParameters = rootparam;//���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = 3;//���[�g�p�����[�^��

	//�T���v���̎w��
	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���J��Ԃ�
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�J��Ԃ�
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s�J��Ԃ�
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
	samplerDesc[0].MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�

	samplerDesc[1] = samplerDesc[0];
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ShaderRegister = 1;

	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	ID3DBlob* rootSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
	rootSigBlob->Release();

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
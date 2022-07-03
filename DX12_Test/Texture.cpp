#include "Texture.h"

///�A���C�����g�ɑ������T�C�Y��Ԃ�
///@param size ���̃T�C�Y
///@param alignment �A���C�����g�T�C�Y
///@return �A���C�����g�����낦���T�C�Y
size_t
AlignmentedSize(size_t size, size_t alignment) {
	return size + alignment - size % alignment;
}

Texture::Texture(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle){
	Initialize(device, resourceHeapHandle);
}

void Texture::Initialize(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle) {
	//WIC�e�N�X�`���̃��[�h
	CoInitializeEx(0, COINIT_MULTITHREADED);
	ScratchImage scratchImg = {};
	HRESULT hr = LoadFromWICFile(L"img/textest.png", WIC_FLAGS_NONE, &_metaData, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);//���f�[�^���o

	//���ԃo�b�t�@�Ƃ���Upload�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;//�P�Ȃ�o�b�t�@�Ƃ���
	auto pixelsize = scratchImg.GetPixelsSize();
	resDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;//�f�[�^�T�C�Y
	resDesc.Height = 1;//
	resDesc.DepthOrArraySize = 1;//
	resDesc.MipLevels = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//�A�������f�[�^�ł���
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�
	resDesc.SampleDesc.Count = 1;//�ʏ�e�N�X�`���Ȃ̂ŃA���`�F�����Ȃ�
	resDesc.SampleDesc.Quality = 0;//

	//���ԃo�b�t�@�쐬
	ID3D12Resource* uploadbuff = nullptr;
	hr = device->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//CPU���珑�����݉\
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	//�e�N�X�`���̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	//�e�N�X�`���𐧌䂷��f�B�X�N���v�^�q�[�v
	resDesc.Format = _metaData.format;
	resDesc.Width = static_cast<UINT>(_metaData.width);//��
	resDesc.Height = static_cast<UINT>(_metaData.height);//����
	resDesc.DepthOrArraySize = static_cast<UINT16>(_metaData.arraySize);//2D�Ŕz��ł��Ȃ��̂łP
	resDesc.MipLevels = static_cast<UINT16>(_metaData.mipLevels);//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(_metaData.dimension);//2D�e�N�X�`���p
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�

	//���\�[�X�쐬
	hr = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,//�R�s�[�p
		nullptr,
		IID_PPV_ARGS(&TexBuffer)
	);
	if (FAILED(hr)) {
		return;
	}

	uint8_t* mapforImg = nullptr;//�Ɠ����^�ɂ���
	hr = uploadbuff->Map(0, nullptr, (void**)&mapforImg);//�}�b�v
	auto srcAddress = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < img->height; ++y) {
		std::copy_n(srcAddress,
			rowPitch,
			mapforImg);//�R�s�[
		//1�s���Ƃ̒�������킹�Ă��
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}
	uploadbuff->Unmap(0, nullptr);//�A���}�b�v

	src = new D3D12_TEXTURE_COPY_LOCATION();
	dst = new D3D12_TEXTURE_COPY_LOCATION();

	dst->pResource = TexBuffer.Get();
	dst->Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst->SubresourceIndex = 0;

	src->pResource = uploadbuff;//���ԃo�b�t�@
	src->Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;//�t�b�g�v�����g(�������̐�L�̈�)�w��
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	UINT nrow;
	UINT64 rowsize, size;
	auto desc = TexBuffer->GetDesc();
	device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
	src->PlacedFootprint = footprint;
	src->PlacedFootprint.Offset = 0;
	src->PlacedFootprint.Footprint.Width = static_cast<UINT>(_metaData.width);
	src->PlacedFootprint.Footprint.Height = static_cast<UINT>(_metaData.height);
	src->PlacedFootprint.Footprint.Depth = static_cast<UINT>(_metaData.depth);
	src->PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	src->PlacedFootprint.Footprint.Format = img->format;

	//�e�N�X�`���r���[
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _metaData.format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f�`1.0f�ɐ��K��)
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	device->CreateShaderResourceView(TexBuffer.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
		resourceHeapHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
	);
}
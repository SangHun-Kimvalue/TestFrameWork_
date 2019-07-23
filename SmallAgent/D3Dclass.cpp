#include "D3Dclass.h"


D3DClass::D3DClass() {
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	pDWriteFactory_ = NULL;
	pTextFormat_ = NULL;
	pBlackBrush_ = NULL;
	wszText_ = NULL;
	cTextLength_ = NULL;
	queuesize = 0;
	//log = m_log;
}

D3DClass::~D3DClass() {
}

void D3DClass::Shutdown() {
	// ����ü���� �����Ҷ��� ��������� �ٲ����...
	m_swapChain->SetFullscreenState(false, NULL);
	if (m_renderTargetView)m_renderTargetView->Release();		m_renderTargetView = 0;
	if (m_deviceContext)m_deviceContext->Release();				m_deviceContext = 0;
	if (m_device)m_device->Release();							m_device = 0;
	if (m_swapChain)m_swapChain->Release();						m_swapChain = 0;
	if (m_layout)  m_layout->Release();							m_layout = 0;
	if (m_pixelShader)  m_pixelShader->Release();				m_pixelShader = 0;
	if (m_vertexShader) m_vertexShader->Release();				m_vertexShader = 0;
	if (m_indexBuffer) 	m_indexBuffer->Release();				m_indexBuffer = 0;
	if (m_vertexBuffer) m_vertexBuffer->Release();				m_vertexBuffer = 0;
	if (m_renderTargetView) m_renderTargetView->Release();		m_renderTargetView = 0;
	if (m_2DTex) m_2DTex->Release();							m_2DTex = 0;
	if (m_ShaderResourceView) m_ShaderResourceView->Release();	m_ShaderResourceView = 0;
	if (pBlackBrush_) pBlackBrush_->Release();					pBlackBrush_ = 0;
	if (m_D2DDevicctx)m_D2DDevicctx->Release();					m_D2DDevicctx = 0;
	if (m_D2DDevice) m_D2DDevice->Release();					m_D2DDevice = 0;
	if (pD2DFactory_) pD2DFactory_->Release();					pD2DFactory_ = 0;
	if (pdxgiDevice) pdxgiDevice->Release();					pdxgiDevice = 0;
	if (BackBuffer) BackBuffer->Release();						BackBuffer = 0;
	if (BitmapTarget) BitmapTarget->Release();					BitmapTarget = 0;
	
	return;
}

bool D3DClass::InitializeBuffers3D(HWND hwnd, const int selectdecode)
{
	
	WCHAR* vsFilename = const_cast<WCHAR*>(L"../DirectX64/texture.vs");
	WCHAR* psFilename = 0;
	if (selectdecode == 0)
		psFilename = const_cast<WCHAR*>(L"../DirectX64/textureyuv.ps");
	else
		psFilename = const_cast<WCHAR*>(L"../DirectX64/textureNV12.ps");
	//WCHAR* psFilename = const_cast<WCHAR*>(L"../DirectX64/texture.ps");

	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	DXGI_MODE_DESC* displayModeList;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	m_vsync_enabled = false;		//��ũ �Ű����� ���� �޾ƿ� ����
									//��ũ�� �������� ����� fps�� ���ⲫ�� �ִ��� �����Ҳ��� ���ߴ��۾�
									//vsync�� true�� �⺻������ 60hz�� ����������
									//false�� �ִ��� �����׸�

	//���丮��� �׷����������̽� ����(DXDI���� ����ϴ� ����͸� ������)
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory);
	if (FAILED(result))		return false;

	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))		return false;

	//����Ϳ� ù��° ����͸� ��������
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))		return false;

	//����ʹ� ��´�� ���÷��� ���(����)�� ������(DXGI_FORMAT_R8G8B8A8_UNORM�� �´� ���÷���)
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))		return false;

	//���ڸ�ŭ �Ҵ��� ���ְ�
	displayModeList = new DXGI_MODE_DESC[numModes];

	//������ ���÷��� ����� ����ü�� ä����
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))		return false;

	//���÷��� ����� ��ȯ�ϸ� �ػ󵵰� �´°��� ã��, fps(������� ���ΰ�ħ ������ ���ϴ�)�� �и�� ���ڸ� ä���.
	for (i = 0; i < numModes; i++) {
		if (displayModeList[i].Width == (unsigned int)width) {
			if (displayModeList[i].Height == (unsigned int)height) {
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}//���ΰ�ħ ������ ����ü���� ����۸� ����̳� ����Ʈ ���ۿ� ��ü�� �׸��� �Ұ������� ���� ����

	delete[] displayModeList;	displayModeList = 0;
	adapterOutput->Release();	adapterOutput = 0;
	adapter->Release();	adapter = 0;
	factory->Release();	factory = 0;

	// ����ü�ι��۸� �Ҵ�
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	// ����۸� ��𿡾������� ������
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// ����۰� ����� �����츦 �ڵ�� �޾ƿ� ����
	swapChainDesc.OutputWindow = hwnd;			//������ ������ ���� ���⼭ �������� �ѷ��ټ��ְڴ�

	//������� ���� ���̸� �ػ󵵿� �°� �����ϰ� ���۹��� ���� �ϳ��� ����
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	// ����۰� �׸��� ����(����)�� ������
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//������� ���ΰ�ħ ������ ������� ����(�ֻ���)�� ���߰ų�, �ִ��� �������ϰų�
	if (m_vsync_enabled) {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	//���÷� ������ ����������.
	swapChainDesc.SampleDesc.Count = 1;
	// ��Ƽ ���ø� ����(��Ƽ����̰̽� ���� ���ø��̶� ����� ����ϱ����ؼ� ��Ƽ���ø��� �ʿ�)�� �Ⱦ�
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = true;

	// ��ĵ���� ���İ� ��ĵ���̴��� �Ⱦ��ϱ� ����
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//��� ���� ���� ���� ���                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;		//���ɻ� �Ⱦ��°� ����
	//�߰��ɼ� xxx
	swapChainDesc.Flags = 0;
	// ���̷�Ʈ��� ���� 
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};  

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
		// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);
	if (FAILED(result))		return false;

	// ����� �����͸� ����
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))		return false;

	// ����� �����ͷ� ���� Ÿ�ٺ並 ����(���۹��� �� ���� ������Ŵ)
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))		return false;

	backBufferPtr->Release();
	backBufferPtr = 0;

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, NULL);

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &vp);

	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set the number of vertices in the vertex array.
	m_vertexCount = 6;

	// Set the number of indices in the index array.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)		return false;

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)		return false;

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// Load the index array with data.
	indices[0] = 0;	indices[1] = 1;	indices[2] = 2;
	indices[3] = 2;	indices[4] = 1;	indices[5] = 3;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))		return false;

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = m_device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))		return false;

	vertices[0].position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);  // �� ��
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

	vertices[1].position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);  // �� ��
	vertices[1].texture = D3DXVECTOR2(1.0f, 0.0f);

	vertices[2].position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);  // �� ��
	vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);

	vertices[3].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);  // �� ��
	vertices[3].texture = D3DXVECTOR2(1.0f, 1.0f);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	// Lock the vertex buffer so it can be written to.
	result = m_deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// Unlock the vertex buffer.
	m_deviceContext->Unmap(m_vertexBuffer, 0);

	delete[] vertices;
	delete[] indices;
	indices = 0;
	vertices = 0;



	// Initialize the vertex and pixel shaders.
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	unsigned int numElements;							

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.			D3D10_SHADER_ENABLE_STRICTNESS������ ���(�����ϰ� ������ �˻���)
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result)) {
		OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		log->LogFile(log->ConvertString(vsFilename, "Missing Shader File"));
		return false;
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result)) {
		OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		log->LogFile(log->ConvertString(psFilename, "Missing Shader File"));
		return false;
	}

	// Create the vertex shader from the buffer.
	result = m_device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))		return false;

	// Create the pixel shader from the buffer.
	result = m_device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))		return false;

	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState *m_sampleState;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;		//���� Ÿ�� - ���� ������. (ó������ �������� ����� ���� ����, ���, Ȯ�� . �� ���� ���ø� �ÿ� ��������)
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;			//��ǥ�� ������ 0~1.0f ���̿� �ֵ��� ���ִ� ����(���� ��Ż�� ��������) - ���� �Ѿ�� �ݺ� �ǵ���
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; 
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; 
	samplerDesc.MipLODBias = 0.0f;								//�Ӹ� ���������� ������ �� ���� �Ӹʷ����� ���Ͽ� ���ø� �ϵ�����.
	samplerDesc.MaxAnisotropy = 1; 
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;		
	samplerDesc.BorderColor[0] = 0;								//ADDRESS_WARP�� �ƴ� ADDRESS_BORDER�� �����ǰ� �ִ� ��� �� ���� 0~1
	samplerDesc.BorderColor[1] = 0; 
	samplerDesc.BorderColor[2] = 0; 
	samplerDesc.BorderColor[3] = 0; 
	samplerDesc.MinLOD = 0;										
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_device->CreateSamplerState(&samplerDesc, &m_sampleState); 
	if (FAILED(result)) { return false; }

	m_deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];			//���̴��� �����ϸ� ���� �ٲ������

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;											//�ִ� 16���� ���۱��� �ѹ��� ����� ����.(��Ʈ�� �ε���)
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;		//���������� �ڵ����� �˷��ֵ�����(�޸𸮻���)
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;			
	polygonLayout[1].InstanceDataStepRate = 0;								//�ν��ֽ��� ����ϱ� ����(���� �޽ø� �ѹ��� ���� �ҷ����� ����)

	polygonLayout[2].SemanticName = "TEXCOORD";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.		//���̾ƿ� �� �� 
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = m_device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&m_layout);
	if (FAILED(result))		return false;

	m_deviceContext->IASetInputLayout(m_layout);

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);				//����(����)
	offset = 0;									//���������������?

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	// Set the index buffer to active in the input assembler so it can be rendered.
	m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	result = Text_Set();
	if (!result)
		return false;

	cpuusage = 0;
	m_startTime = timeGetTime();

	return true;
}

bool D3DClass::Debug_position(bool Show_Debug) {
	
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	VertexType* vertices;
	HRESULT result;
	float rect_debug = 0;
	if (Show_Debug == true)
		rect_debug = 0.9f;
	else
		rect_debug = 1.0f;

	vertices = new VertexType[m_vertexCount];
	if (!vertices) {
		log->LogFile("vertexset Error\n");
		delete[] vertices;
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	vertices[0].position = D3DXVECTOR3(-1.0f, rect_debug, 1.0f);  // �� ��
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

	vertices[1].position = D3DXVECTOR3(1.0f, rect_debug, 1.0f);  // �� ��
	vertices[1].texture = D3DXVECTOR2(1.0f, 0.0f);

	vertices[2].position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);  // �� ��
	vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);

	vertices[3].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);  // �� ��
	vertices[3].texture = D3DXVECTOR2(1.0f, 1.0f);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	// Lock the vertex buffer so it can be written to.
	result = m_deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		log->LogFile("Debug_Mapping Error\n");
		m_deviceContext->Unmap(m_vertexBuffer, 0);
		delete[] vertices;
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// Unlock the vertex buffer.
	m_deviceContext->Unmap(m_vertexBuffer, 0);

	delete[] vertices;
	if (Show_Debug == true)
		return true;
	else
		return false;
}

bool D3DClass::Text_Set() {
	
	HRESULT result = 0;
	D2D1_FACTORY_OPTIONS options;

//#ifndef NDEBUG
//	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
//#else
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
//#endif
	
	result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &pD2DFactory_);	//d2���丮����
	if (FAILED(result))
		return log->LogFile("2D Factory error");

	m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);		//d3����̽��� dxgi ����̽��� ����
	
	result = pD2DFactory_->CreateDevice(pdxgiDevice, &m_D2DDevice);	//dxgi�� d3�� ����� d2����̽� ����
	if (FAILED(result))
		return log->LogFile("2DDevice error");

	result = m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDevicctx);	//d2 ����̽� ���ؽ�Ʈ ����
	if (FAILED(result))
		return log->LogFile("2DDevicectx error");

	result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory_));//dwrite ���丮 ����
	if (FAILED(result))
		return log->LogFile("2DDWrite Factory error");

	wszText_ = L"Hello World using  DirectWrite!";
	cTextLength_ = (UINT32)wcslen(wszText_);

	result = pDWriteFactory_->CreateTextFormat(
		L"����",                // ��Ʈ �̸�
		NULL,                   // ��Ʈ�� �����ִ� �׷� (NULL sets it to use the system font collection).
		DWRITE_FONT_WEIGHT_BLACK,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		50.0f,
		L"ko_KR",
		&pTextFormat_
	);
	if (FAILED(result))
		return log->LogFile("Text Format error");
	
	result = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	if (FAILED(result))		//��� ��ġ
		return log->LogFile("SetTextAlignment error");
	
	result = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);		
	if (FAILED(result))		//��� ��ġ
		return log->LogFile("SetParagraphAlignment error");

	/////////////////////////////////////////////////////////////////////////////////////
	result = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));			//d3�� ����ü���� �˻��� ������
	if (FAILED(result))
		return log->LogFile("Get Backbuffer error");

	D2D1_RENDER_TARGET_PROPERTIES RTproperties;
	RTproperties = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
		96, 96);

	pD2DFactory_->CreateDxgiSurfaceRenderTarget(BackBuffer, &RTproperties, &D2_RenderTarget);
	
	BackBuffer->Release();
	BackBuffer = NULL;

	result = D2_RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&pBlackBrush_);		//�۾� ��
	if (FAILED(result))
		return log->LogFile("CreateSolidColorBrush error");

	RECT desktop;								//pixel per inch		//�ػ� == �ȼ� �е� ����
	SetProcessDPIAware();						//Dot per inch
	d3d_hwnd = GetDesktopWindow();				//dpi������ ����� ������ �ػ󵵸� �����´�
	GetWindowRect(d3d_hwnd, &desktop);

	layoutRect = { 50.0f, 0.0f, (float)desktop.right, 200.0f };

	return true;
}

void D3DClass::Get_Render_FPS() {
	static int m_count;

	m_count++;
	if (timeGetTime() >= (m_startTime + 500)) {
		RenderFPS = m_count * 2;
		m_count = 0;
		m_startTime = timeGetTime();
	}
	
}

void D3DClass::Draw_Text() {
	HRESULT result;
	string text_temp = "";	wstring temp = L"";	
	string FPStemp = to_string(VideoFPS).substr(0,5);
	string Sleeptemp = to_string(sleeptime).substr(0,5);
	log->Frame(cpuusage);
	string CPUtemp = to_string(cpuusage);

	//text_temp = "Video FPS : " + FPStemp + "  RenderCycle :" + to_string((int)RenderCycle) +
	text_temp = "Cpu_usage :" + CPUtemp + "\tRenderCycle :" + to_string((int)RenderCycle) +
		"  RenderFPS : " + to_string(RenderFPS) + "  Rendersleep : " + Sleeptemp + "  Q_size : " + to_string(queuesize) + 
		"  Mbps : " + to_string(Megabitrate);

	temp = temp.assign(text_temp.begin(), text_temp.end());
	wszText_ = temp.c_str();
	cTextLength_ = (UINT32)wcslen(wszText_);

	D2_RenderTarget->BeginDraw();

	D2_RenderTarget->DrawText(
		wszText_,        // ������ �� ���ڿ�
		cTextLength_,    // ���ڿ� ����
		pTextFormat_,    // �ؽ�Ʈ ����
		layoutRect,       // �ؽ�Ʈ�� ������ �� ���̾ƿ�
		pBlackBrush_     // ����� �ؽ�Ʈ�� �귯��
	);
	
	result = D2_RenderTarget->EndDraw();
	if (FAILED(result))
		log->LogFile("Text Draw Failed");

	return ;
}

int D3DClass::Render(bool Show_Debug)
{
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Get_Render_FPS();

	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	m_deviceContext->DrawIndexed(m_indexCount, 0, 0);					//�׸��� �Լ�(�������� ����� present�� ��)

	if (Show_Debug == true)
		Draw_Text();
	m_swapChain->Present(0, 0);
	
	return RenderFPS;
}

bool D3DClass::CreateResource_RGB() {
	HRESULT result;
	D3D11_TEXTURE2D_DESC TextureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc;
	if (m_2DTex) { m_2DTex->Release(); m_2DTex = 0; }
	if (m_ShaderResourceView){ m_ShaderResourceView->Release(); m_ShaderResourceView = 0;}

	ZeroMemory(&TextureDesc, sizeof(TextureDesc));

	TextureDesc.Height = m_imageHeight;
	TextureDesc.Width = m_imageWidth;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//size 32bit
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Usage = D3D11_USAGE_DYNAMIC;
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	TextureDesc.MiscFlags = 0;

	DWORD*	pInitImage = new DWORD[m_imageWidth*m_imageHeight*3];
	memset(pInitImage, 0, sizeof(DWORD)*m_imageWidth*m_imageHeight);
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pInitImage;
	InitData.SysMemPitch = m_imageWidth * sizeof(DWORD);
	InitData.SysMemSlicePitch = 0;

	result = m_device->CreateTexture2D(&TextureDesc, &InitData, &m_2DTex);
	if (FAILED(result))		return false;

	ShaderResourceViewDesc.Format = TextureDesc.Format;
	ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	ShaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = m_device->CreateShaderResourceView(m_2DTex, &ShaderResourceViewDesc, &m_ShaderResourceView);
	if (FAILED(result))		return false;

	m_deviceContext->PSSetShaderResources(0, 1, &m_ShaderResourceView);

	return true;
}

bool D3DClass::CreateResource_YUV420P(enum Texture_Format resource_format) {
	HRESULT result;
	D3D11_TEXTURE2D_DESC TextureDescY;
	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc;
	if (m_Ybuffer) { m_Ybuffer->Release(); m_Ybuffer = 0; }
	if (m_Ubuffer) { m_Ubuffer->Release(); m_Ubuffer = 0; }
	if (m_Vbuffer) { m_Vbuffer->Release(); m_Vbuffer = 0; }

	if (YUV_SRView[0]){YUV_SRView[0]->Release(); YUV_SRView[0] = 0;}
	if (YUV_SRView[1]){YUV_SRView[1]->Release(); YUV_SRView[1] = 0;}
	if (YUV_SRView[2]){YUV_SRView[2]->Release(); YUV_SRView[2] = 0;}

	ZeroMemory(&TextureDescY, sizeof(TextureDescY));

	TextureDescY.Height = m_imageHeight;
	TextureDescY.Width = m_imageWidth;
	TextureDescY.MipLevels = 1;
	TextureDescY.ArraySize = 1;
	TextureDescY.Format = DXGI_FORMAT_R8_UNORM;
	TextureDescY.SampleDesc.Count = 1;
	TextureDescY.SampleDesc.Quality = 0;
	TextureDescY.Usage = D3D11_USAGE_DEFAULT;
	TextureDescY.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	TextureDescY.CPUAccessFlags = 0;
	TextureDescY.MiscFlags = 0;

	DWORD*	YInitImage = new DWORD[m_imageWidth*m_imageHeight];
	memset(YInitImage, 0, sizeof(DWORD)*m_imageWidth*m_imageHeight);
	D3D11_SUBRESOURCE_DATA InitYData;

	InitYData.pSysMem = YInitImage;
	InitYData.SysMemPitch = m_imageWidth * sizeof(DWORD);
	InitYData.SysMemSlicePitch = 0;

	result = m_device->CreateTexture2D(&TextureDescY, &InitYData, &m_Ybuffer);

	if (resource_format == YUV420P) {

		D3D11_TEXTURE2D_DESC TextureDescU;
		ZeroMemory(&TextureDescU, sizeof(TextureDescU));

		int UVimageHei = m_imageHeight / 2;
		int UVimageWid = m_imageWidth / 2;

		TextureDescU.Height = UVimageHei;
		TextureDescU.Width = UVimageWid;
		TextureDescU.MipLevels = 1;
		TextureDescU.ArraySize = 1;
		TextureDescU.Format = DXGI_FORMAT_R8_UNORM;
		TextureDescU.SampleDesc.Count = 1;
		TextureDescU.SampleDesc.Quality = 0;
		TextureDescU.Usage = D3D11_USAGE_DEFAULT;
		TextureDescU.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDescU.CPUAccessFlags = 0;
		TextureDescU.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitUData;

		DWORD*	UInitImage = new DWORD[UVimageWid*UVimageHei];
		memset(UInitImage, 0, sizeof(DWORD)*UVimageWid*UVimageHei);
		InitUData.pSysMem = UInitImage;
		InitUData.SysMemPitch = UVimageWid * sizeof(DWORD);
		InitUData.SysMemSlicePitch = 0;

		D3D11_TEXTURE2D_DESC TextureDescV;
		ZeroMemory(&TextureDescV, sizeof(TextureDescV));
		TextureDescV.Height = UVimageHei;
		TextureDescV.Width = UVimageWid;
		TextureDescV.MipLevels = 1;
		TextureDescV.ArraySize = 1;
		TextureDescV.Format = DXGI_FORMAT_R8_UNORM;
		TextureDescV.SampleDesc.Count = 1;
		TextureDescV.SampleDesc.Quality = 0;
		TextureDescV.Usage = D3D11_USAGE_DEFAULT;
		TextureDescV.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDescV.CPUAccessFlags = 0;
		TextureDescV.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitVData;

		DWORD*	VInitImage = new DWORD[UVimageWid*UVimageHei];
		memset(VInitImage, 0, sizeof(DWORD)*UVimageWid*UVimageHei);
		InitVData.pSysMem = VInitImage;
		InitVData.SysMemPitch = UVimageWid * sizeof(DWORD);
		InitVData.SysMemSlicePitch = 0;

		result = m_device->CreateTexture2D(&TextureDescU, &InitUData, &m_Ubuffer);
		result = m_device->CreateTexture2D(&TextureDescV, &InitVData, &m_Vbuffer);

		if (FAILED(result)) {
			if (m_Ybuffer) { m_Ybuffer->Release(); m_Ybuffer = 0; }
			if (m_Ubuffer) { m_Ubuffer->Release(); m_Ubuffer = 0; }
			if (m_Vbuffer) { m_Vbuffer->Release(); m_Vbuffer = 0; }
			return false;
		}

		ShaderResourceViewDesc.Format = TextureDescY.Format;
		ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2D.MipLevels = 1;

		result = m_device->CreateShaderResourceView(m_Ybuffer, &ShaderResourceViewDesc, &YUV_SRView[0]);
		result = m_device->CreateShaderResourceView(m_Ubuffer, &ShaderResourceViewDesc, &YUV_SRView[1]);
		result = m_device->CreateShaderResourceView(m_Vbuffer, &ShaderResourceViewDesc, &YUV_SRView[2]);

		if (FAILED(result)) {
			if (YUV_SRView[0])YUV_SRView[0]->Release(); YUV_SRView[0] = 0;
			if (YUV_SRView[1])YUV_SRView[1]->Release(); YUV_SRView[1] = 0;
			if (YUV_SRView[2])YUV_SRView[2]->Release(); YUV_SRView[2] = 0;
			return false;
		}

		m_deviceContext->PSSetShaderResources(0, 1, &YUV_SRView[0]);
		m_deviceContext->PSSetShaderResources(1, 1, &YUV_SRView[1]);
		m_deviceContext->PSSetShaderResources(2, 1, &YUV_SRView[2]);

		return true;
	}
	else if (resource_format == NV12) {

		D3D11_TEXTURE2D_DESC TextureDescU;
		ZeroMemory(&TextureDescU, sizeof(TextureDescU));

		int UVimageHei = m_imageHeight / 2;
		int UVimageWid = m_imageWidth/ 2;

		TextureDescU.Height = UVimageHei;
		TextureDescU.Width = UVimageWid;
		TextureDescU.MipLevels = 1;
		TextureDescU.ArraySize = 1;
		TextureDescU.Format = DXGI_FORMAT_R8G8_UNORM;
		TextureDescU.SampleDesc.Count = 1;
		TextureDescU.SampleDesc.Quality = 0;
		TextureDescU.Usage = D3D11_USAGE_DEFAULT;
		TextureDescU.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDescU.CPUAccessFlags = 0;
		TextureDescU.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitUData;

		DWORD*	UInitImage = new DWORD[UVimageWid*UVimageHei];
		memset(UInitImage, 0, sizeof(DWORD)*UVimageWid*UVimageHei);
		InitUData.pSysMem = UInitImage;
		InitUData.SysMemPitch = UVimageWid * sizeof(DWORD);
		InitUData.SysMemSlicePitch = 0;

		result = m_device->CreateTexture2D(&TextureDescU, &InitUData, &m_Ubuffer);
		if (FAILED(result)) {
			if (m_Ybuffer) { m_Ybuffer->Release(); m_Ybuffer = 0; }
			if (m_Ubuffer) { m_Ubuffer->Release(); m_Ubuffer = 0; }
			return false;
		}

		ShaderResourceViewDesc.Format = TextureDescY.Format;
		ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2D.MipLevels = 1;

		result = m_device->CreateShaderResourceView(m_Ybuffer, &ShaderResourceViewDesc, &YUV_SRView[0]);
		if (FAILED(result)) {
			if (YUV_SRView[0])YUV_SRView[0]->Release(); YUV_SRView[0] = 0;
			if (YUV_SRView[1])YUV_SRView[1]->Release(); YUV_SRView[1] = 0;
			return false;
		}

		ShaderResourceViewDesc.Format = TextureDescU.Format;
		ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2D.MipLevels = 1;
		result = m_device->CreateShaderResourceView(m_Ubuffer, &ShaderResourceViewDesc, &YUV_SRView[1]);
		if (FAILED(result)) {
			if (YUV_SRView[0])YUV_SRView[0]->Release(); YUV_SRView[0] = 0;
			if (YUV_SRView[1])YUV_SRView[1]->Release(); YUV_SRView[1] = 0;
			return false;
		}
		m_deviceContext->PSSetShaderResources(0, 1, &YUV_SRView[0]);
		m_deviceContext->PSSetShaderResources(1, 1, &YUV_SRView[1]);

		return true;
	}

	else {
		return false;
	}
}

bool D3DClass::Update_Tex_YUV420P(AVFrame *VFrame) {

	bool res;			//HRESULT result;
	BYTE * pFrameData1;	//BYTE* mappedData;
	BYTE * pFrameData2; BYTE * pFrameData3;

	if (VFrame->data[0] != NULL || VFrame->linesize[0] != 0) {

		if (VFrame->linesize[0] != Stride || VFrame->height != m_imageHeight || VFrame->width != m_imageWidth) {
			//�ؽ��� ���۸� ���� ��������.
			m_imageHeight = VFrame->height;		m_imageWidth = VFrame->width;
			res = CreateResource_YUV420P(YUV420P);
			if (!res)
				return false;
		}
		pFrameData1 = VFrame->data[0];	pFrameData2 = VFrame->data[1];	pFrameData3 = VFrame->data[2];
		Stride = VFrame->linesize[0];

		m_deviceContext->UpdateSubresource(m_Ybuffer, 0, NULL, pFrameData1, Stride, 0);
		m_deviceContext->UpdateSubresource(m_Ubuffer, 0, NULL, pFrameData2, Stride/2, 0);
		m_deviceContext->UpdateSubresource(m_Vbuffer, 0, NULL, pFrameData3, Stride/2, 0);

		/*result = m_deviceContext->Map(m_Ybuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &S_mappedResource_tt);
		if (FAILED(result))	return false;
		memcpy(mappedData, VFrame->data[0], Stride);
		mappedData += S_mappedResource_tt.RowPitch;
		pFrameData1 += Stride;
		memcpy(mappedData, VFrame->data[1], Stride / 2);
		mappedData += S_mappedResource_tt.RowPitch;
		pFrameData2 += Stride / 2;
		memcpy(mappedData, VFrame->data[2], Stride / 2);
		mappedData += S_mappedResource_tt.RowPitch;
		pFrameData3 += Stride / 2;
		m_deviceContext->Unmap(m_Vbuffer, 0);*/
	}
	return true;
}

bool D3DClass::Update_Tex_NV12(AVFrame *VFrame) {
	
	bool res;	BYTE * pFrameData1;	BYTE * pFrameData2;

	if (VFrame->data[0] != NULL || VFrame->linesize[0] != 0) {

		if (VFrame->linesize[0] != Stride || VFrame->height != m_imageHeight || VFrame->width != m_imageWidth) {
			//�ؽ��� ���۸� ���� ��������.
			m_imageHeight = VFrame->height;		m_imageWidth = VFrame->width;
			res = CreateResource_YUV420P(NV12);
			if (!res)
				return false;
		}
		pFrameData1 = VFrame->data[0];	pFrameData2 = VFrame->data[1];	Stride = VFrame->linesize[0];

		m_deviceContext->UpdateSubresource(m_Ybuffer, 0, NULL, pFrameData1, Stride, 0);
		m_deviceContext->UpdateSubresource(m_Ubuffer, 0, NULL, pFrameData2, Stride, 0);

	}
	return true;
}

bool D3DClass::Update_Tex_RGB(AVFrame *VFrame) {
	HRESULT result;		bool res;
	BYTE * pFrameData;	BYTE* mappedData;

	if (VFrame->data[0] != NULL || VFrame->linesize[0] != 0) {

		if (VFrame->linesize[0] != Stride || VFrame->height != m_imageHeight || VFrame->width != m_imageWidth) {
			//�ؽ��� ���۸� ���� ��������.
			m_imageHeight = VFrame->height;		m_imageWidth = VFrame->width;
			if (m_2DTex != NULL) { m_2DTex->Release();					m_2DTex = NULL; }
			if (m_ShaderResourceView != NULL) { m_ShaderResourceView->Release();	m_ShaderResourceView = NULL; }
			res = CreateResource_RGB();
			if (!res)
				return false;
		}

		ZeroMemory(&S_mappedResource_tt, sizeof(S_mappedResource_tt));
		Stride = VFrame->linesize[0];

		result = m_deviceContext->Map(m_2DTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &S_mappedResource_tt);
		if (FAILED(result))	return false;

		pFrameData = VFrame->data[0];
		mappedData = (BYTE *)S_mappedResource_tt.pData;
		for (auto i = 0; i < VFrame->height; i++) {
			memcpy(mappedData, pFrameData, Stride);
			mappedData += S_mappedResource_tt.RowPitch;
			pFrameData += Stride;
		}

		m_deviceContext->Unmap(m_2DTex, 0);
	}

	return true;
}

void D3DClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;					//�� ���� �Լ�
	unsigned long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());
	// Get the length of the message.
	bufferSize = (unsigned long)(errorMessage->GetBufferSize());
	// Open a file to write the error message to.
	fout.open("shader-error.txt");
	// Write out the error message.
	for (i = 0; i < bufferSize; i++) {
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();
	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	log->LogFile("Error compiling shader.  Check shader-error.txt for message.");
	//MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
	return;
}

#include "stdafx.h"
#include "ParticleSystem.h"


ParticleSystem::ParticleSystem()
{
	GameTime = 0.0f;
	DeltaTime = 0.0f;
	EmitterPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	Acceleration = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	CBforParticle = nullptr;

	InitVB = nullptr;
	DrawVB = nullptr;
	StreamOutVB = nullptr;

	InputLayout = nullptr;
	VS = nullptr;
	VSforSO = nullptr;
	GS = nullptr;
	GSforSO = nullptr;
	PS = nullptr;

	SODepthStencilState = nullptr;
	DepthStencilState = nullptr;
	BlendState = nullptr;
	RasterizerState = nullptr;

	RandomTexSRV = nullptr;
	ParticleTexSRV = nullptr;
}


ParticleSystem::~ParticleSystem()
{
}

// Buffer, ConstantBuffer, DepthStencilState, BlendState
void ParticleSystem::CreateShaderVariables(ID3D11Device *pd3dDevice, D3DXVECTOR3 particlePos,
	D3D11_BLEND src_blend, D3D11_BLEND dest_blend, D3D11_BLEND_OP blend_op)
{
	// Buffer
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(Particle);
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	Particle particle;
	ZeroMemory(&particle, sizeof(Particle));
	particle.Age = 0.0f;
	particle.InitialPos = particlePos;
	particle.InitialVel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	particle.Size = D3DXVECTOR2(1.0f, 1.0f);
	particle.Type = 0;
	EmitterPos = particlePos;

	D3D11_SUBRESOURCE_DATA d3dSubResourceData;
	d3dSubResourceData.pSysMem = &particle;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dSubResourceData, &InitVB);
	// ok
	d3dBufferDesc.ByteWidth = sizeof(Particle) * PARTICLES;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &StreamOutVB);
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &DrawVB);
	// ok
	// ConstantBuffer
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof(ParticleInfo);
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &CBforParticle);
	// ok
	// DepthStencilState, BlendState, RasterizerState
	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &SODepthStencilState);
	// ok
	d3dDepthStencilDesc.DepthEnable = true;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	d3dDepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &DepthStencilState);
	// ok
	D3D11_BLEND_DESC d3dBlendStateDesc;
	ZeroMemory(&d3dBlendStateDesc, sizeof(D3D11_BLEND_DESC));
	d3dBlendStateDesc.IndependentBlendEnable = false;
	ZeroMemory(&d3dBlendStateDesc.RenderTarget[0], sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	d3dBlendStateDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendStateDesc.RenderTarget[0].SrcBlend = src_blend;
	d3dBlendStateDesc.RenderTarget[0].DestBlend = dest_blend;
	d3dBlendStateDesc.RenderTarget[0].BlendOp = blend_op;
	d3dBlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;//
	d3dBlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;//
	d3dBlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;//
	d3dBlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	pd3dDevice->CreateBlendState(&d3dBlendStateDesc, &BlendState);

	D3D11_RASTERIZER_DESC d3dRasterDesc;
	ZeroMemory(&d3dRasterDesc, sizeof(D3D11_RASTERIZER_DESC));
	d3dRasterDesc.FillMode = D3D11_FILL_SOLID;
	d3dRasterDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterDesc.FrontCounterClockwise = FALSE;
	d3dRasterDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	d3dRasterDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	d3dRasterDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	d3dRasterDesc.DepthClipEnable = TRUE;
	d3dRasterDesc.ScissorEnable = FALSE;
	d3dRasterDesc.MultisampleEnable = FALSE;
	d3dRasterDesc.AntialiasedLineEnable = FALSE;

	pd3dDevice->CreateRasterizerState(&d3dRasterDesc, &RasterizerState);
}

void ParticleSystem::Reset(ID3D11DeviceContext *pd3dDeviceContext)
{
	InitFlag = true;
	GameTime = 0.0f;
	DeltaTime = 0.0f;

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(CBforParticle, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	ParticleInfo *pParticleInfo = (ParticleInfo *)d3dMappedResource.pData;
	pParticleInfo->GameTime = GameTime;
	pParticleInfo->DeltaTime = DeltaTime;
	pParticleInfo->Acceleration = Acceleration;
	pParticleInfo->EmitterPos = EmitterPos;
	pd3dDeviceContext->Unmap(CBforParticle, 0);
}

void ParticleSystem::UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(CBforParticle, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	ParticleInfo *pParticleInfo = (ParticleInfo *)d3dMappedResource.pData;
	pParticleInfo->GameTime = GameTime;
	pParticleInfo->DeltaTime = DeltaTime;
	pParticleInfo->Acceleration = Acceleration;
	pParticleInfo->EmitterPos = EmitterPos;
	pd3dDeviceContext->Unmap(CBforParticle, 0);

	pd3dDeviceContext->VSSetConstantBuffers(VS_CB_SLOT_PARTICLE, 1, &CBforParticle);
	pd3dDeviceContext->GSSetConstantBuffers(GS_CB_SLOT_PARTICLE, 1, &CBforParticle);
}

void ParticleSystem::Initialize(ID3D11Device * pd3dDevice, ID3D11ShaderResourceView *pd3dsrvParticleTexture,
	ID3D11ShaderResourceView *pd3dsrvRandomTexture, UINT nMaxParticles)
{
	MaxParticles = nMaxParticles;
	InitFlag = true;
	GameTime = 0.0f;
	DeltaTime = 0.0f;
	Acceleration = D3DXVECTOR3(0.0f, 50.0f, 0.0f);
	EmitterPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	RandomTexSRV = pd3dsrvRandomTexture;
	ParticleTexSRV = pd3dsrvParticleTexture;

	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;

	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &ParticleSampler);
}

void ParticleSystem::Update(float deltaTime, float nCount)
{
	GameTime += deltaTime + particleID / nCount;
	DeltaTime = deltaTime;
}

void ParticleSystem::Render(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->DSSetShader(NULL, NULL, 0);
	pd3dDeviceContext->HSSetShader(NULL, NULL, 0);

	pd3dDeviceContext->IASetInputLayout(InputLayout);
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT stride = sizeof(Particle);
	UINT offset = 0;

	pd3dDeviceContext->SOSetTargets(1, &StreamOutVB, &offset);

	pd3dDeviceContext->VSSetShader(VSforSO, NULL, 0);
	pd3dDeviceContext->GSSetShader(GSforSO, NULL, 0);
	pd3dDeviceContext->PSSetShader(NULL, NULL, 0);
	pd3dDeviceContext->OMSetDepthStencilState(SODepthStencilState, 0);

	pd3dDeviceContext->GSSetShaderResources(GS_SLOT_TEXTURE_RANDOM, 1, &RandomTexSRV);
	pd3dDeviceContext->GSSetSamplers(GS_SLOT_SAMPLER_PARTICLE, 1, &ParticleSampler);

	if (InitFlag)
	{
		pd3dDeviceContext->IASetVertexBuffers(0, 1, &InitVB, &stride, &offset);
		pd3dDeviceContext->Draw(1, 0);
		InitFlag = false;
	}
	else
	{
		pd3dDeviceContext->IASetVertexBuffers(0, 1, &DrawVB, &stride, &offset);
		pd3dDeviceContext->DrawAuto();
	}

	ID3D11Buffer *pd3dBuffer = DrawVB;
	DrawVB = StreamOutVB;
	StreamOutVB = pd3dBuffer;

	ID3D11Buffer *pd3dBuffers[1] = { NULL };
	UINT pStreamOffsets[1] = { 0 };

	pd3dDeviceContext->SOSetTargets(1, pd3dBuffers, pStreamOffsets);

	pd3dDeviceContext->VSSetShader(VS, NULL, 0);
	pd3dDeviceContext->GSSetShader(GS, NULL, 0);
	pd3dDeviceContext->PSSetShader(PS, NULL, 0);

	pd3dDeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
	pd3dDeviceContext->OMSetBlendState(BlendState, NULL, 0xffffffff);
	// ok
	pd3dDeviceContext->PSSetShaderResources(PS_SLOT_TEXTURE_ARRAY_FP, 1, &ParticleTexSRV);
	pd3dDeviceContext->PSSetSamplers(PS_SLOT_SAMPLER_PARTICLE, 1, &ParticleSampler);

	pd3dDeviceContext->IASetVertexBuffers(0, 1, &DrawVB, &stride, &offset);

	pd3dDeviceContext->RSSetState(RasterizerState);
	pd3dDeviceContext->DrawAuto();

	pd3dDeviceContext->RSSetState(nullptr);
	pd3dDeviceContext->OMSetDepthStencilState(NULL, 0);
	pd3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
}

BYTE * ParticleSystem::ReadCompiledShaderCode(ID3D11Device *pd3dDevice, WCHAR *pszFileName, int& nSize)
{
	std::ifstream ifsCompiledShader;
	ifsCompiledShader.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	int nFileSize = ifsCompiledShader.tellg();
	ifsCompiledShader.seekg(0);
	BYTE *pByteCode = new BYTE[nFileSize];
	ifsCompiledShader.read((char *)pByteCode, nFileSize);
	ifsCompiledShader.close();

	nSize = nFileSize;
	return pByteCode;
}

void ParticleSystem::CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputElements, UINT nElements, ID3D11InputLayout **ppd3dInputLayout)
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dErrorBlob = NULL;
	if (SUCCEEDED(hResult = D3DX11CompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dVertexShaderBlob, &pd3dErrorBlob, NULL)))
	{
		pd3dDevice->CreateVertexShader(pd3dVertexShaderBlob->GetBufferPointer(), pd3dVertexShaderBlob->GetBufferSize(), NULL, ppd3dVertexShader);
		pd3dDevice->CreateInputLayout(pd3dInputElements, nElements, pd3dVertexShaderBlob->GetBufferPointer(), pd3dVertexShaderBlob->GetBufferSize(), ppd3dInputLayout);
		pd3dVertexShaderBlob->Release();
	}
}

void ParticleSystem::CreateSOGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader)
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	D3D11_SO_DECLARATION_ENTRY pSODecls[] = {
		{ 0, "POSITION", 0, 0, 3, 0 },
		{ 0, "VELOCITY", 0, 0, 3, 0 },
		{ 0, "SIZE", 0, 0, 2, 0 },
		{ 0, "AGE", 0, 0, 1, 0 },
		{ 0, "TYPE", 0, 0, 1, 0 }
	};

	UINT pBufferStrides[1] = { sizeof(Particle) };

	ID3DBlob *pd3dPixelShaderBlob = NULL, *pd3dErrorBlob = NULL;
	if (SUCCEEDED(hResult = D3DX11CompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL,
		&pd3dPixelShaderBlob, &pd3dErrorBlob, NULL)))
	{
		pd3dDevice->CreateGeometryShaderWithStreamOutput(pd3dPixelShaderBlob->GetBufferPointer(),
			pd3dPixelShaderBlob->GetBufferSize(), pSODecls, 5, pBufferStrides, 1, D3D11_SO_NO_RASTERIZED_STREAM, NULL,
			ppd3dGeometryShader);
		pd3dPixelShaderBlob->Release();
	}
}

void ParticleSystem::CreateGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader)
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dPixelShaderBlob = NULL, *pd3dErrorBlob = NULL;
	if (SUCCEEDED(hResult = D3DX11CompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dPixelShaderBlob, &pd3dErrorBlob, NULL)))
	{
		pd3dDevice->CreateGeometryShader(pd3dPixelShaderBlob->GetBufferPointer(), pd3dPixelShaderBlob->GetBufferSize(), NULL, ppd3dGeometryShader);
		pd3dPixelShaderBlob->Release();
	}
}

void ParticleSystem::CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader)
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dPixelShaderBlob = NULL, *pd3dErrorBlob = NULL;
	if (SUCCEEDED(hResult = D3DX11CompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dPixelShaderBlob, &pd3dErrorBlob, NULL)))
	{
		pd3dDevice->CreatePixelShader(pd3dPixelShaderBlob->GetBufferPointer(), pd3dPixelShaderBlob->GetBufferSize(), NULL, ppd3dPixelShader);
		pd3dPixelShaderBlob->Release();
	}
}

// CreateShader
void ParticleSystem::CreateShader(ID3D11Device *pd3dDevice, unsigned int type)
{
	int nSize;

	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	if (PARTICLE_TYPE_FLARE == type) {
		BYTE *pVSforSOByteCode = ReadCompiledShaderCode(pd3dDevice, L"VSParticleStreamOut.cso", nSize);
		pd3dDevice->CreateVertexShader(pVSforSOByteCode, nSize, nullptr, &VSforSO);
		pd3dDevice->CreateInputLayout(d3dInputLayout, 5, pVSforSOByteCode, nSize, &InputLayout);
		delete[] pVSforSOByteCode;

		BYTE *pVSByteCode = ReadCompiledShaderCode(pd3dDevice, L"VSParticleDraw.cso", nSize);
		pd3dDevice->CreateVertexShader(pVSByteCode, nSize, nullptr, &VS);
		pd3dDevice->CreateInputLayout(d3dInputLayout, 5, pVSByteCode, nSize, &InputLayout);
		delete[] pVSByteCode;

		D3D11_SO_DECLARATION_ENTRY pSODecls[] = {
			{ 0, "POSITION", 0, 0, 3, 0 },
			{ 0, "VELOCITY", 0, 0, 3, 0 },
			{ 0, "SIZE", 0, 0, 2, 0 },
			{ 0, "AGE", 0, 0, 1, 0 },
			{ 0, "TYPE", 0, 0, 1, 0 }
		};

		UINT pBufferStrides[1] = { sizeof(Particle) };

		BYTE *pGSforSOByteCode = ReadCompiledShaderCode(pd3dDevice, L"GSParticleStreamOut.cso", nSize);
		pd3dDevice->CreateGeometryShaderWithStreamOutput(pGSforSOByteCode, nSize, pSODecls, 5, pBufferStrides, 1, 
			D3D11_SO_NO_RASTERIZED_STREAM, NULL, &GSforSO);
		delete[] pGSforSOByteCode;

		BYTE *pGSByteCode = ReadCompiledShaderCode(pd3dDevice, L"GSParticleDraw.cso", nSize);
		pd3dDevice->CreateGeometryShader(pGSByteCode, nSize, nullptr, &GS);
		delete[] pGSByteCode;

		BYTE *pPSByteCode = ReadCompiledShaderCode(pd3dDevice, L"PSParticleDraw.cso", nSize);
		pd3dDevice->CreatePixelShader(pPSByteCode, nSize, nullptr, &PS);
		delete[] pPSByteCode;
	}
	else if (PARTICLE_TYPE_SMOKE == type) {
		BYTE *pVSforSOByteCode = ReadCompiledShaderCode(pd3dDevice, L"VSParticleStreamOut_Smoke.cso", nSize);
		pd3dDevice->CreateVertexShader(pVSforSOByteCode, nSize, nullptr, &VSforSO);
		pd3dDevice->CreateInputLayout(d3dInputLayout, 5, pVSforSOByteCode, nSize, &InputLayout);
		delete[] pVSforSOByteCode;

		BYTE *pVSByteCode = ReadCompiledShaderCode(pd3dDevice, L"VSParticleDraw_Smoke.cso", nSize);
		pd3dDevice->CreateVertexShader(pVSByteCode, nSize, nullptr, &VS);
		pd3dDevice->CreateInputLayout(d3dInputLayout, 5, pVSByteCode, nSize, &InputLayout);
		delete[] pVSByteCode;

		D3D11_SO_DECLARATION_ENTRY pSODecls[] = {
			{ 0, "POSITION", 0, 0, 3, 0 },
			{ 0, "VELOCITY", 0, 0, 3, 0 },
			{ 0, "SIZE", 0, 0, 2, 0 },
			{ 0, "AGE", 0, 0, 1, 0 },
			{ 0, "TYPE", 0, 0, 1, 0 }
		};

		UINT pBufferStrides[1] = { sizeof(Particle) };

		BYTE *pGSforSOByteCode = ReadCompiledShaderCode(pd3dDevice, L"GSParticleStreamOut_Smoke.cso", nSize);
		pd3dDevice->CreateGeometryShaderWithStreamOutput(pGSforSOByteCode, nSize, pSODecls, 5, pBufferStrides, 1,
			D3D11_SO_NO_RASTERIZED_STREAM, NULL, &GSforSO);
		delete[] pGSforSOByteCode;

		BYTE *pGSByteCode = ReadCompiledShaderCode(pd3dDevice, L"GSParticleDraw_Smoke.cso", nSize);
		pd3dDevice->CreateGeometryShader(pGSByteCode, nSize, nullptr, &GS);
		delete[] pGSByteCode;

		BYTE *pPSByteCode = ReadCompiledShaderCode(pd3dDevice, L"PSParticleDraw_Smoke.cso", nSize);
		pd3dDevice->CreatePixelShader(pPSByteCode, nSize, nullptr, &PS);
		delete[] pPSByteCode;
	}
	else if (PARTICLE_TYPE_BLOOD == type) {
		BYTE *pVSforSOByteCode = ReadCompiledShaderCode(pd3dDevice, L"VSParticleStreamOut_Blood.cso", nSize);
		pd3dDevice->CreateVertexShader(pVSforSOByteCode, nSize, nullptr, &VSforSO);
		pd3dDevice->CreateInputLayout(d3dInputLayout, 5, pVSforSOByteCode, nSize, &InputLayout);
		delete[] pVSforSOByteCode;

		BYTE *pVSByteCode = ReadCompiledShaderCode(pd3dDevice, L"VSParticleDraw_Blood.cso", nSize);
		pd3dDevice->CreateVertexShader(pVSByteCode, nSize, nullptr, &VS);
		pd3dDevice->CreateInputLayout(d3dInputLayout, 5, pVSByteCode, nSize, &InputLayout);
		delete[] pVSByteCode;

		D3D11_SO_DECLARATION_ENTRY pSODecls[] = {
			{ 0, "POSITION", 0, 0, 3, 0 },
			{ 0, "VELOCITY", 0, 0, 3, 0 },
			{ 0, "SIZE", 0, 0, 2, 0 },
			{ 0, "AGE", 0, 0, 1, 0 },
			{ 0, "TYPE", 0, 0, 1, 0 }
		};

		UINT pBufferStrides[1] = { sizeof(Particle) };

		BYTE *pGSforSOByteCode = ReadCompiledShaderCode(pd3dDevice, L"GSParticleStreamOut_Blood.cso", nSize);
		pd3dDevice->CreateGeometryShaderWithStreamOutput(pGSforSOByteCode, nSize, pSODecls, 5, pBufferStrides, 1,
			D3D11_SO_NO_RASTERIZED_STREAM, NULL, &GSforSO);
		delete[] pGSforSOByteCode;

		BYTE *pGSByteCode = ReadCompiledShaderCode(pd3dDevice, L"GSParticleDraw_Blood.cso", nSize);
		pd3dDevice->CreateGeometryShader(pGSByteCode, nSize, nullptr, &GS);
		delete[] pGSByteCode;

		BYTE *pPSByteCode = ReadCompiledShaderCode(pd3dDevice, L"PSParticleDraw_Blood.cso", nSize);
		pd3dDevice->CreatePixelShader(pPSByteCode, nSize, nullptr, &PS);
		delete[] pPSByteCode;
	}
}

void ParticleSystem::SetPosition(ID3D11Device *pd3dDevice, D3DXVECTOR3 pos)
{
	// Buffer
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(Particle);
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	Particle particle;
	ZeroMemory(&particle, sizeof(Particle));
	particle.Age = 0.0f;
	particle.InitialPos = pos;
	particle.InitialVel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	particle.Size = D3DXVECTOR2(1.0f, 1.0f);
	particle.Type = 0;
	EmitterPos = pos;

	InitVB->Release();
	D3D11_SUBRESOURCE_DATA d3dSubResourceData;
	d3dSubResourceData.pSysMem = &particle;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dSubResourceData, &InitVB);
}
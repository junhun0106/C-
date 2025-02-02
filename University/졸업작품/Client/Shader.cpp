//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "Scene.h"

extern int g_GameState;

CShader::CShader()
{
	m_nReferences = 0;

	m_pd3dVertexShader = NULL;
	m_pd3dVertexLayout = NULL;
	m_pd3dPixelShader = NULL;
	m_pd3dGeometryShader = NULL;

	m_nType = 0x00;
}

CShader::~CShader()
{
	if (m_pd3dVertexShader) m_pd3dVertexShader->Release();
	if (m_pd3dVertexLayout) m_pd3dVertexLayout->Release();
	if (m_pd3dPixelShader) m_pd3dPixelShader->Release();
	if (m_pd3dGeometryShader) m_pd3dGeometryShader->Release();
}

void CShader::CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputElements, UINT nElements, ID3D11InputLayout **ppd3dInputLayout)
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
		hResult = pd3dDevice->CreateInputLayout(pd3dInputElements, nElements, pd3dVertexShaderBlob->GetBufferPointer(), pd3dVertexShaderBlob->GetBufferSize(), ppd3dInputLayout);
		pd3dVertexShaderBlob->Release();
	}
}

void CShader::CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader)
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

void CShader::CreateGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader)
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

void CShader::CreateShader(ID3D11Device *pd3dDevice, UINT nType)
{
	m_nType |= nType;
	GetInputElementDesc(m_nType);
	LPCSTR pszVSShaderName = NULL, pszVSShaderModel = "vs_5_0", pszPSShaderName = NULL, pszPSShaderModel = "ps_5_0";
	GetShaderName(m_nType, &pszVSShaderName, &pszVSShaderModel, &pszPSShaderName, &pszPSShaderModel);
	CreateShader(pd3dDevice, NULL, 0, L"Effect.fx", pszVSShaderName, pszVSShaderModel, pszPSShaderName, pszPSShaderModel);
}

void CShader::CreateShader(ID3D11Device *pd3dDevice, D3D11_INPUT_ELEMENT_DESC *pd3dInputElementDesc, int nInputElements, WCHAR *pszFileName, LPCSTR pszVSShaderName, LPCSTR pszVSShaderModel, LPCSTR pszPSShaderName, LPCSTR pszPSShaderModel)
{
	CreateVertexShaderFromFile(pd3dDevice, pszFileName, pszVSShaderName, pszVSShaderModel, &m_pd3dVertexShader, (pd3dInputElementDesc) ? pd3dInputElementDesc : m_pd3dInputElementDescs, (pd3dInputElementDesc) ? nInputElements : m_nInputElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, pszFileName, pszPSShaderName, pszPSShaderModel, &m_pd3dPixelShader);
}

void CShader::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
}

void CShader::ReleaseShaderVariables()
{
}

void CShader::GetInputElementDesc(UINT nVertexElementType)
{
	m_nInputElements = 0;
	if (nVertexElementType & VERTEX_POSITION_ELEMENT) m_nInputElements++;//
	if (nVertexElementType & VERTEX_COLOR_ELEMENT) m_nInputElements++;
	if (nVertexElementType & VERTEX_NORMAL_ELEMENT) m_nInputElements++;//
	if (nVertexElementType & VERTEX_TEXTURE_ELEMENT_0) m_nInputElements++;//
	if (nVertexElementType & VERTEX_TEXTURE_ELEMENT_1) m_nInputElements++;
	if (nVertexElementType & VERTEX_BONE_ID_ELEMENT) m_nInputElements++;//
	if (nVertexElementType & VERTEX_BONE_WEIGHT_ELEMENT) m_nInputElements++;//
	if (nVertexElementType & VERTEX_INSTANCING_ELEMENT) m_nInputElements += 4;//
	m_pd3dInputElementDescs = new D3D11_INPUT_ELEMENT_DESC[m_nInputElements];
	int nIndex = 0, nSlot = 0;
	if (nVertexElementType & VERTEX_POSITION_ELEMENT) m_pd3dInputElementDescs[nIndex++] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_COLOR_ELEMENT) m_pd3dInputElementDescs[nIndex++] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_NORMAL_ELEMENT) m_pd3dInputElementDescs[nIndex++] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_TEXTURE_ELEMENT_0) m_pd3dInputElementDescs[nIndex++] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_TEXTURE_ELEMENT_1) m_pd3dInputElementDescs[nIndex++] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_BONE_ID_ELEMENT) m_pd3dInputElementDescs[nIndex++] = { "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_SINT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_BONE_WEIGHT_ELEMENT) m_pd3dInputElementDescs[nIndex++] = { "BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, (UINT)nSlot++, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (nVertexElementType & VERTEX_INSTANCING_ELEMENT)
	{
		m_pd3dInputElementDescs[nIndex++] = { "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, (UINT)nSlot, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		m_pd3dInputElementDescs[nIndex++] = { "INSTANCEPOS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, (UINT)nSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		m_pd3dInputElementDescs[nIndex++] = { "INSTANCEPOS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, (UINT)nSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		m_pd3dInputElementDescs[nIndex++] = { "INSTANCEPOS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, (UINT)nSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 };
	}
}

void CShader::GetShaderName(UINT nVertexElementType, LPCSTR *ppszVSShaderName, LPCSTR *ppszVSShaderModel, LPCSTR *ppszPSShaderName, LPCSTR *ppszPSShaderModel)
{
	int nInputElements = 0, nIndex = 0;
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_COLOR_ELEMENT)) { *ppszVSShaderName = "VSDiffusedColor", *ppszPSShaderName = "PSDiffusedColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT)) { *ppszVSShaderName = "VSLightingColor", *ppszPSShaderName = "PSLightingColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_TEXTURE_ELEMENT_0)) { *ppszVSShaderName = "VSTexturedColor", *ppszPSShaderName = "PSTexturedColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_TEXTURE_ELEMENT_0 | VERTEX_TEXTURE_ELEMENT_1)) { *ppszVSShaderName = "VSDetailTexturedColor", *ppszPSShaderName = "PSDetailTexturedColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0)) { *ppszVSShaderName = "VSTexturedLightingColor", *ppszPSShaderName = "PSTexturedLightingColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0 | VERTEX_TEXTURE_ELEMENT_1)) { *ppszVSShaderName = "VSDetailTexturedLightingColor", *ppszPSShaderName = "PSDetailTexturedLightingColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_BONE_ID_ELEMENT | VERTEX_BONE_WEIGHT_ELEMENT)) { *ppszVSShaderName = "VSSkinnedLightingColor", *ppszPSShaderName = "PSLightingColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0 | VERTEX_BONE_ID_ELEMENT | VERTEX_BONE_WEIGHT_ELEMENT)) { *ppszVSShaderName = "VSSkinnedTexturedLightingColor", *ppszPSShaderName = "PSTexturedLightingColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0 | VERTEX_INSTANCING_ELEMENT)) { *ppszVSShaderName = "VSInstancedTexturedLightingColor", *ppszPSShaderName = "PSInstancedTexturedLightingColor"; }
	if (nVertexElementType == (VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0 | VERTEX_BONE_ID_ELEMENT | VERTEX_BONE_WEIGHT_ELEMENT | VERTEX_INSTANCING_ELEMENT)) { *ppszVSShaderName = "VSSkinnedInstance", *ppszPSShaderName = "PSSkinnedInstance"; }
}

void CShader::OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->IASetInputLayout(m_pd3dVertexLayout);
	if (GAME_STATE_LOBBY == g_GameState) return;
	pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, NULL, 0);
	pd3dDeviceContext->PSSetShader(m_pd3dPixelShader, NULL, 0);
	pd3dDeviceContext->GSSetShader(m_pd3dGeometryShader, NULL, 0);
}

void CShader::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext);
}

ID3D11Buffer *CShader::CreateBuffer(ID3D11Device *pd3dDevice, UINT nStride, int nElements, void *pBufferData, UINT nBindFlags, D3D11_USAGE d3dUsage, UINT nCPUAccessFlags)
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = d3dUsage;
	d3dBufferDesc.ByteWidth = nStride * nElements;
	d3dBufferDesc.BindFlags = nBindFlags;
	d3dBufferDesc.CPUAccessFlags = nCPUAccessFlags;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pBufferData;

	ID3D11Buffer *pd3dInstanceBuffer = NULL;
	HRESULT hr = S_FALSE; 
	hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, (pBufferData) ? &d3dBufferData : NULL, &pd3dInstanceBuffer);
	hr;
	return(pd3dInstanceBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

void CTerrainShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSTerrainDetailTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSTerrainDetailTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

void CSkyBoxShader::CreateShader(ID3D11Device *pd3dDevice)
{
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSSkyBoxTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSSkyBoxTexturedColor", "ps_5_0", &m_pd3dPixelShader);
#else
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSSkyBoxTexturedColor", "ps_5_0", &m_pd3dPixelShader);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CWaterShader::CWaterShader()
{
}

CWaterShader::~CWaterShader()
{
}

void CWaterShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSAnimatedDetailTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSDetailTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CObjectsShader::CObjectsShader(int nObjects)
{
	m_ppObjects = NULL;

	m_nObjects = nObjects;
	if (m_nObjects > 0)
	{
		m_ppObjects = new CGameObject*[m_nObjects];
		for (int i = 0; i < m_nObjects; i++) m_ppObjects[i] = NULL;
	}

	m_pMaterial = NULL;
	m_pContext = NULL;

	m_nIndexToAdd = 0;

	space_partition.Initialize_space_division(2048, 5, 1);
}

CObjectsShader::~CObjectsShader()
{
}

void CObjectsShader::SetObject(int nIndex, CGameObject *pGameObject)
{
	if (m_ppObjects[nIndex]) m_ppObjects[nIndex]->Release();
	m_ppObjects[nIndex] = pGameObject;
	if (pGameObject) pGameObject->AddRef();
}

void CObjectsShader::AddObject(CGameObject *pGameObject)
{
	m_ppObjects[m_nIndexToAdd++] = pGameObject;
	if (pGameObject) pGameObject->AddRef();
}

void CObjectsShader::SetMaterial(CMaterial *pMaterial)
{
	m_pMaterial = pMaterial;
	if (pMaterial) pMaterial->AddRef();
}

void CObjectsShader::BuildObjects(ID3D11Device *pd3dDevice, void *pContext)
{
	m_pContext = pContext;
}

void CObjectsShader::BuildObjects(ID3D11Device *pd3dDevice, std::vector<CGameObject*>& rObjectVec)
{
	m_nObjects = rObjectVec.size();

	m_ppObjects = new CGameObject*[m_nObjects];
	//m_ppObjects = NULL;
	int j = 0;
	for (auto i = rObjectVec.begin(); i != rObjectVec.end(); ++i) {
		D3DXVECTOR3 d3dxvPosition = (*i)->GetPosition();
		int index = space_partition.serch_space(d3dxvPosition.x, d3dxvPosition.y, d3dxvPosition.z);
		space_partition.SetSpace(index, *i);
		m_ppObjects[j++] = *i;
	}
	//이렇게 하면 될라나?
}

void CObjectsShader::ReleaseObjects()
{
	if (m_pMaterial) m_pMaterial->Release();

	//if (m_ppObjects)
	//{
	//	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
	//	delete[] m_ppObjects;
	//}
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed, NULL);
	}
}

void CObjectsShader::OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext)
{
	CShader::OnPrepareRender(pd3dDeviceContext);
}

void CObjectsShader::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	CShader::Render(pd3dDeviceContext, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariable(pd3dDeviceContext);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			if (m_ppObjects[j]->IsVisible(pCamera))
			{
				m_ppObjects[j]->Render(pd3dDeviceContext, pCamera);
			}
		}
	}
}
void CObjectsShader::Render(D3DXVECTOR3 d3dxvPosition,ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera) {
	if (GAME_STATE_PLAY == g_GameState)
	{
		int index = space_partition.serch_space(d3dxvPosition.x, d3dxvPosition.y, d3dxvPosition.z);
		vector<int> visible_vector = space_partition.serch_visible_section(d3dxvPosition.x, d3dxvPosition.y, d3dxvPosition.z);
		auto size = space_partition.GetObjectsIntheSpace(index).size();
		CShader::Render(pd3dDeviceContext, pCamera);
		if (m_pMaterial) m_pMaterial->UpdateShaderVariable(pd3dDeviceContext);
		int visible_vec_size = visible_vector.size();
		for (auto i = 0; i < visible_vec_size; ++i) {
			vector<CGameObject*> v = space_partition.GetObjectsIntheSpace(visible_vector[i]);
			auto begin = v.begin();
			auto end = v.end();
			while (begin != end) {
				//if((*begin)->IsVisible(pCamera))
				(*begin)->Render(pd3dDeviceContext, pCamera);
				begin++;
			}
		}
	}
	else if (GAME_STATE_LOBBY == g_GameState)
	{
		CShader::Render(pd3dDeviceContext, pCamera);

		vector<vector<CGameObject*>> vec;

		vector<CGameObject*> v1 = space_partition.GetObjectsIntheSpace(0);
		vec.push_back(v1);
		vector<CGameObject*> v2 = space_partition.GetObjectsIntheSpace(1);
		vec.push_back(v2);
		vector<CGameObject*> v3 = space_partition.GetObjectsIntheSpace(2);
		vec.push_back(v3);
		vector<CGameObject*> v4 = space_partition.GetObjectsIntheSpace(32);
		vec.push_back(v4);
		vector<CGameObject*> v5 = space_partition.GetObjectsIntheSpace(33);
		vec.push_back(v5);
		vector<CGameObject*> v6 = space_partition.GetObjectsIntheSpace(34);
		vec.push_back(v6);
		vector<CGameObject*> v7 = space_partition.GetObjectsIntheSpace(64);
		vec.push_back(v7);
		vector<CGameObject*> v8 = space_partition.GetObjectsIntheSpace(65);
		vec.push_back(v8);
		vector<CGameObject*> v9 = space_partition.GetObjectsIntheSpace(66);
		vec.push_back(v9);

		for (auto b = vec.begin(); b != vec.end(); ++b) {
			for (auto e = (*b).begin(); e != (*b).end(); ++e) {
				(*e)->Render(pd3dDeviceContext, pCamera);
			}
		}
	}
}

CGameObject *CObjectsShader::PickObjectByRayIntersection(D3DXVECTOR3 *pd3dxvPickPosition, D3DXMATRIX *pd3dxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	int nIntersected = 0;
	float fNearHitDistance = FLT_MAX;
	CGameObject *pSelectedObject = NULL;
	MESHINTERSECTINFO d3dxIntersectInfo;
	for (int i = 0; i < m_nObjects; i++)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(pd3dxvPickPosition, pd3dxmtxView, &d3dxIntersectInfo);
		if ((nIntersected > 0) && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
		{
			fNearHitDistance = d3dxIntersectInfo.m_fDistance;
			pSelectedObject = m_ppObjects[i];
			if (pd3dxIntersectInfo) *pd3dxIntersectInfo = d3dxIntersectInfo;
		}
	}
	return(pSelectedObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CInstancedObjectsShader::CInstancedObjectsShader(int nObjects) : CObjectsShader(nObjects)
{
	m_nInstanceBufferStride = sizeof(D3DXMATRIX);
	m_nInstanceBufferOffset = 0;
	m_pd3dInstanceBuffer = NULL;

	m_pMesh = NULL;

	m_nType = VERTEX_INSTANCING_ELEMENT;
}

CInstancedObjectsShader::~CInstancedObjectsShader()
{
}

void CInstancedObjectsShader::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CInstancedObjectsShader::CreateShader(ID3D11Device *pd3dDevice)
{
	if (m_pMesh) CreateShader(pd3dDevice, m_pMesh->GetType());
}

void CInstancedObjectsShader::CreateShader(ID3D11Device *pd3dDevice, UINT nType)
{
	CObjectsShader::CreateShader(pd3dDevice, nType);
}

void CInstancedObjectsShader::BuildObjects(ID3D11Device *pd3dDevice, void *pContext)
{
	CObjectsShader::BuildObjects(pd3dDevice, pContext);

	m_pd3dInstanceBuffer = CreateBuffer(pd3dDevice, m_nInstanceBufferStride, m_nObjects, NULL, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	m_pMesh->AssembleToVertexBuffer(1, &m_pd3dInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);
}

void CInstancedObjectsShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();

	if (m_pMesh) m_pMesh->Release();

	if (m_pd3dInstanceBuffer) m_pd3dInstanceBuffer->Release();
}

void CInstancedObjectsShader::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariable(pd3dDeviceContext);

    if (m_ppObjects[0]->GetWireFlag())
        pd3dDeviceContext->RSSetState(m_ppObjects[0]->GetWireRS());
    else
        pd3dDeviceContext->RSSetState(m_ppObjects[0]->GetSolidRS());

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;

	int nInstances = 0;
	pd3dDeviceContext->Map(m_pd3dInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	D3DXMATRIX *pd3dxmtxInstances = (D3DXMATRIX *)d3dMappedResource.pData;
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			if (m_ppObjects[j]->IsVisible(pCamera))
			{
				D3DXMatrixTranspose(&pd3dxmtxInstances[nInstances++], &m_ppObjects[j]->m_d3dxmtxWorld);
			}
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dInstanceBuffer, 0);

	m_pMesh->RenderInstanced(pd3dDeviceContext, nInstances, 0);
}

CTextureToScreenShader::CTextureToScreenShader()
{
}

CTextureToScreenShader::~CTextureToScreenShader()
{
}

void CTextureToScreenShader::CreateShader(ID3D11Device *pd3dDevice, UINT nType)
{
    m_nType |= nType;
    GetInputElementDesc(m_nType);
    LPCSTR pszVSShaderModel = "vs_5_0", pszPSShaderModel = "ps_5_0";
    CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSTextureToScreen", pszVSShaderModel, &m_pd3dVertexShader, m_pd3dInputElementDescs, m_nInputElements, &m_pd3dVertexLayout);
    CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSTextureToScreen", pszPSShaderModel, &m_pd3dPixelShader);
}


CCharacterShader::CCharacterShader(int nObjects) : CObjectsShader(nObjects)
{
	m_ppObjects = NULL;

	m_nObjects = nObjects;
	if (m_nObjects > 0)
	{
		m_ppObjects = new CGameObject*[m_nObjects];
		for (int i = 0; i < m_nObjects; i++) m_ppObjects[i] = NULL;
	}

	m_pMaterial = NULL;
	//m_nIndexToAdd = 0;
}


CCharacterShader::~CCharacterShader()
{
}

void CCharacterShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSSkinned", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSSkinned", "ps_5_0", &m_pd3dPixelShader);
}

void CMinimapPlayerShader::CreateShader(ID3D11Device *pd3dDevice, UINT nType)
{
    m_nType |= nType;
    GetInputElementDesc(m_nType);
    LPCSTR pszVSShaderModel = "vs_5_0", pszPSShaderModel = "ps_5_0";
    CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSDiffusedProjection", pszVSShaderModel, &m_pd3dVertexShader, m_pd3dInputElementDescs, m_nInputElements, &m_pd3dVertexLayout);
    CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSDiffusedProjection", pszPSShaderModel, &m_pd3dPixelShader);
}

void CMinimapRadarShader::CreateShader(ID3D11Device *pd3dDevice, UINT nType)
{
    m_nType |= nType;
    GetInputElementDesc(m_nType);
    LPCSTR pszVSShaderModel = "vs_5_0", pszPSShaderModel = "ps_5_0";
    CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSRadar", pszVSShaderModel, &m_pd3dVertexShader, m_pd3dInputElementDescs, m_nInputElements, &m_pd3dVertexLayout);
    CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSRadar", pszPSShaderModel, &m_pd3dPixelShader);
}

void CMinimapRadarGridShader::CreateShader(ID3D11Device *pd3dDevice, UINT nType)
{
    m_nType |= nType;
    GetInputElementDesc(m_nType);
    LPCSTR pszVSShaderModel = "vs_5_0", pszPSShaderModel = "ps_5_0";
    CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSRadar", pszVSShaderModel, &m_pd3dVertexShader, m_pd3dInputElementDescs, m_nInputElements, &m_pd3dVertexLayout);
    CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSRadarGrid", pszPSShaderModel, &m_pd3dPixelShader);
}


///////////////////////////////////////////////////////
// 정헌 shader

CSteerGSInstancedObjectShader::CSteerGSInstancedObjectShader(int nObjects) : CObjectsShader(nObjects) {
	m_nInstanceBufferStride = sizeof(D3DMATRIX);
	m_nInstanceBufferOffset = 0;
	m_pd3dInstanceBuffer = nullptr;

	m_pMesh = nullptr;
}
CSteerGSInstancedObjectShader::~CSteerGSInstancedObjectShader() {

}
void CSteerGSInstancedObjectShader::SetMesh(CMesh *pMesh) {
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}
void CSteerGSInstancedObjectShader::CreateShader(ID3D11Device *pd3dDevice) {
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEPOS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEPOS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEPOS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VS_STEER", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PS_STEER", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"Effect.fx", "GS_STEER", "gs_5_0", &m_pd3dGeometryShader);
}
void CSteerGSInstancedObjectShader::BuildObjects(ID3D11Device *pd3dDevice, void *pContext) {
	CObjectsShader::BuildObjects(pd3dDevice, pContext);
	m_pd3dInstanceBuffer = CreateBuffer(pd3dDevice, m_nInstanceBufferStride, m_nObjects, NULL, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	m_pMesh->AssembleToVertexBuffer(1, &m_pd3dInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);
}
void CSteerGSInstancedObjectShader::ReleaseObjects() {
	CObjectsShader::ReleaseObjects();
	if (m_pMesh) m_pMesh->Release();
	if (m_pd3dInstanceBuffer) m_pd3dInstanceBuffer->Release();
}
void CSteerGSInstancedObjectShader::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera) {
	OnPrepareRender(pd3dDeviceContext);
	if (m_pMaterial) m_pMaterial->UpdateShaderVariable(pd3dDeviceContext);

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	int nInstances = 0;
	pd3dDeviceContext->Map(m_pd3dInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	D3DXMATRIX *pd3dmtxInstances = (D3DXMATRIX*)d3dMappedResource.pData;
	for (auto i = 0; i < m_nObjects; ++i) {
		if (m_ppObjects[i]) {
			if (m_ppObjects[i]->IsVisible(pCamera)) {
				D3DXMatrixTranspose(&pd3dmtxInstances[nInstances++], &m_ppObjects[i]->m_d3dxmtxWorld);
			}
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dInstanceBuffer, 0);
	m_pMesh->RenderInstanced(pd3dDeviceContext, nInstances, 0);
}
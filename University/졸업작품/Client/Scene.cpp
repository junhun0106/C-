//-----------------------------------------------------------------------------
// File: Scene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "resource.h"
#include "SkinnedMesh.h"
#include "PlayerObject.h"

#include <chrono>

extern bool g_zoom;
extern float particleCreateRange;
extern bool g_bBlood;
float RandF(float a, float b)
{
	return a + float(rand()) / float(RAND_MAX) * (b - a);
}

ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device *d3dDevice)
{
	//
	// 무작위 자료를 생성한다.
	//

	D3DXVECTOR4 randomValues[1024];

	for (int i = 0; i < 1024; ++i) {
		randomValues[i].x = RandF(-1.0f, 1.0f);
		randomValues[i].y = RandF(-1.0f, 1.0f);
		randomValues[i].z = RandF(-1.0f, 1.0f);
		randomValues[i].w = RandF(-1.0f, 1.0f);
	}

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024 * sizeof(D3DXVECTOR4);
	initData.SysMemSlicePitch = 0;

	//
	// 텍스쳐를 생성한다.
	//
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture1D* randomTex = nullptr;
	HRESULT hr = d3dDevice->CreateTexture1D(&texDesc, &initData, &randomTex);

	//
	// 리소스 뷰를 생성한다.
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MostDetailedMip = texDesc.MipLevels - 1;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;

	ID3D11ShaderResourceView* randomTexSRV = nullptr;
	hr = d3dDevice->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV);

	randomTex->Release();

	return randomTexSRV;
}

ID3D11ShaderResourceView *CreateTexture2DArray(ID3D11Device *pd3dDevice, wchar_t *pPath, int nTextures)
{
	//
	// 파일로부터 2D 텍스쳐 배열을 만들때 사용될 값들을 정한다.
	//
	D3DX11_IMAGE_LOAD_INFO d3dxImageLoadInfo;
	d3dxImageLoadInfo.Width = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Height = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Depth = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.FirstMipLevel = 0;
	d3dxImageLoadInfo.MipLevels = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Usage = D3D11_USAGE_STAGING;
	d3dxImageLoadInfo.BindFlags = 0;
	d3dxImageLoadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	d3dxImageLoadInfo.MiscFlags = 0;
	d3dxImageLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dxImageLoadInfo.Filter = D3DX11_FILTER_NONE;
	d3dxImageLoadInfo.MipFilter = D3DX11_FILTER_LINEAR;
	d3dxImageLoadInfo.pSrcInfo = nullptr;

	//
	// 2D 텍스쳐 배열에 사용될 각각의 텍스쳐들을 파일로부터 만들어낸다.
	//
	ID3D11Texture2D **pd3dTextures = new ID3D11Texture2D *[nTextures];
	for (int i = 0; i < nTextures; ++i) {
		wchar_t pstrTextureName[80];
		swprintf_s(pstrTextureName, 80, L"%s%02d.dds", pPath, i);
		D3DX11CreateTextureFromFile(pd3dDevice, pstrTextureName, &d3dxImageLoadInfo, nullptr, (ID3D11Resource **)&pd3dTextures[i], nullptr);
	}

	//
	// 위에서 만든 텍스쳐들을 사용하여 2D 텍스쳐 배열을 갱신한다.
	//
	D3D11_TEXTURE2D_DESC d3dTexture2DDesc;
	D3D11_TEXTURE2D_DESC d3dTexture2DArrayDesc;
	pd3dTextures[0]->GetDesc(&d3dTexture2DDesc);
	d3dTexture2DArrayDesc.Width = d3dTexture2DDesc.Width;
	d3dTexture2DArrayDesc.Height = d3dTexture2DDesc.Height;
	d3dTexture2DArrayDesc.MipLevels = d3dTexture2DDesc.MipLevels;
	d3dTexture2DArrayDesc.ArraySize = nTextures;
	d3dTexture2DArrayDesc.Format = d3dTexture2DDesc.Format;
	d3dTexture2DArrayDesc.SampleDesc.Count = 1;
	d3dTexture2DArrayDesc.SampleDesc.Quality = 0;
	d3dTexture2DArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dTexture2DArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	d3dTexture2DArrayDesc.CPUAccessFlags = 0;
	d3dTexture2DArrayDesc.MiscFlags = 0;
	ID3D11Texture2D *pd3dTexture2DArray;
	pd3dDevice->CreateTexture2D(&d3dTexture2DArrayDesc, nullptr, &pd3dTexture2DArray);

	ID3D11DeviceContext *pd3dDeviceContext;
	pd3dDevice->GetImmediateContext(&pd3dDeviceContext);
	D3D11_MAPPED_SUBRESOURCE pd3dMappedSubresource;
	for (int i = 0; i < nTextures; ++i) {
		for (int j = 0; j < d3dTexture2DArrayDesc.MipLevels; ++j) {
			pd3dDeviceContext->Map(pd3dTextures[i], j, D3D11_MAP_READ, 0, &pd3dMappedSubresource);
			pd3dDeviceContext->UpdateSubresource(pd3dTexture2DArray, D3D11CalcSubresource(j, i, d3dTexture2DArrayDesc.MipLevels),
				nullptr, pd3dMappedSubresource.pData, pd3dMappedSubresource.RowPitch, pd3dMappedSubresource.DepthPitch);
			pd3dDeviceContext->Unmap(pd3dTextures[i], j);
		}
	}

	//
	// 2D 텍스쳐 배열을 사용하기 위한 뷰를 만든다.
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC d3dTexture2DArraySRVDesc;
	d3dTexture2DArraySRVDesc.Format = d3dTexture2DArrayDesc.Format;
	d3dTexture2DArraySRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	d3dTexture2DArraySRVDesc.Texture2DArray.ArraySize = nTextures;
	d3dTexture2DArraySRVDesc.Texture2DArray.FirstArraySlice = 0;
	d3dTexture2DArraySRVDesc.Texture2DArray.MipLevels = d3dTexture2DArrayDesc.MipLevels;
	d3dTexture2DArraySRVDesc.Texture2DArray.MostDetailedMip = 0;
	ID3D11ShaderResourceView *pd3dTexture2DArraySRV;
	HRESULT hr = pd3dDevice->CreateShaderResourceView(pd3dTexture2DArray, &d3dTexture2DArraySRVDesc, &pd3dTexture2DArraySRV);

	//
	// 사용이 완료된 메모리들을 해제한다.
	//
	for (int i = 0; i < nTextures; ++i) if (pd3dTextures[i]) pd3dTextures[i]->Release();
	if (pd3dTexture2DArray) pd3dTexture2DArray->Release();
	delete[] pd3dTextures;

	return pd3dTexture2DArraySRV;
}

CScene::CScene()
{
    m_ppObjects = NULL;
    m_nObjects = 0;

    m_ppObjectShaders = NULL;
    m_nObjectShaders = 0;

    m_ppInstancingShaders = NULL;
    m_nInstancingShaders = 0;

    m_pLights = NULL;
    m_pd3dcbLights = NULL;

    m_pCamera = NULL;
    m_pSelectedObject = NULL;

	m_wsa_send_buf.buf = m_csend_buf;
	m_wsa_send_buf.len = BUFF_SIZE;

	m_pParticles = nullptr;
}

CScene::~CScene()
{
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
    switch (nMessageID)
    {
    case WM_LBUTTONDOWN:
        m_pSelectedObject = PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_RBUTTONDOWN:
        break;
    case WM_MOUSEMOVE:
        break;
    case WM_LBUTTONUP:
        break;
    case WM_RBUTTONUP:
        break;
    default:
        break;
    }
    return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
    return(false);
}

void CScene::OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, CMaterial *pMaterial, int nIndex)
{
	HRESULT result = S_FALSE;
	ID3D11ShaderResourceView *pd3dsrvTexture = NULL;
	//result = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building01a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	result = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("SkyDome/SimpleSky.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pMaterial->m_pTexture->SetTexture(0, pd3dsrvTexture); pd3dsrvTexture->Release();   
}

void CScene::BuildObjects(ID3D11Device *pd3dDevice)
{

	auto start = chrono::high_resolution_clock::now();
    ID3D11SamplerState *pd3dSamplerState = NULL;
    D3D11_SAMPLER_DESC d3dSamplerDesc;
    ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    d3dSamplerDesc.MinLOD = 0;
    d3dSamplerDesc.MaxLOD = 0;
    pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
    CTexture *pSkyboxTexture = new CTexture(1, 1, PS_SLOT_TEXTURE_SKYBOX, PS_SLOT_SAMPLER_SKYBOX);
#else
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
    CTexture *pSkyboxTexture = new CTexture(1, 1, PS_SLOT_TEXTURE_SKYBOX, PS_SLOT_SAMPLER_SKYBOX);
#else
    CTexture *pSkyboxTexture = new CTexture(6, 1, PS_SLOT_TEXTURE_SKYBOX, PS_SLOT_SAMPLER_SKYBOX);
#endif
#endif
    pSkyboxTexture->SetSampler(0, pd3dSamplerState);

    CMaterial *pSkyboxMaterial = new CMaterial(NULL);
    pSkyboxMaterial->SetTexture(pSkyboxTexture);
    OnChangeSkyBoxTextures(pd3dDevice, pSkyboxMaterial, 0);

	CSkyDomeMesh *pSkyBoxMesh = new CSkyDomeMesh(pd3dDevice);

	CSkyBox *pSkyBox = new CSkyBox(pd3dDevice);
    pSkyBox->SetMesh(pSkyBoxMesh, 0);
    pSkyBox->SetMaterial(pSkyboxMaterial);

    CShader *pSkyBoxShader = new CSkyBoxShader();
    pSkyBoxShader->CreateShader(pd3dDevice);
    pSkyBox->SetShader(pSkyBoxShader);

    ID3D11SamplerState *pd3dBaseSamplerState = NULL;
    ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    d3dSamplerDesc.MinLOD = 0;
    d3dSamplerDesc.MaxLOD = 0;
    pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dBaseSamplerState);

    ID3D11SamplerState *pd3dDetailSamplerState = NULL;
    d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dDetailSamplerState);

    CTexture *pTerrainTexture = new CTexture(2, 2, PS_SLOT_TEXTURE_TERRAIN, PS_SLOT_SAMPLER_TERRAIN);

    ID3D11ShaderResourceView *pd3dsrvBaseTexture = NULL;
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Terrain/Base_Texture.jpg"), NULL, NULL, &pd3dsrvBaseTexture, NULL);
    pTerrainTexture->SetTexture(0, pd3dsrvBaseTexture);
    pTerrainTexture->SetSampler(0, pd3dBaseSamplerState);
    pd3dsrvBaseTexture->Release();
    pd3dBaseSamplerState->Release();

    ID3D11ShaderResourceView *pd3dsrvDetailTexture = NULL;
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Terrain/Detail_Texture_7.jpg"), NULL, NULL, &pd3dsrvDetailTexture, NULL);
    pTerrainTexture->SetTexture(1, pd3dsrvDetailTexture);
    pTerrainTexture->SetSampler(1, pd3dDetailSamplerState);
    pd3dsrvDetailTexture->Release();
    pd3dDetailSamplerState->Release();

    CMaterialColors *pTerrainColors = new CMaterialColors();
    pTerrainColors->m_d3dxcDiffuse = D3DXCOLOR(0.8f, 1.0f, 0.2f, 1.0f);
    pTerrainColors->m_d3dxcAmbient = D3DXCOLOR(0.1f, 0.3f, 0.1f, 1.0f);
    pTerrainColors->m_d3dxcSpecular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 5.0f);
    pTerrainColors->m_d3dxcEmissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

    CMaterial *pTerrainMaterial = new CMaterial(pTerrainColors);
    pTerrainMaterial->SetTexture(pTerrainTexture);

    D3DXVECTOR3 d3dxvScale(8.0f, 0.0f, 8.0f);
#ifdef _WITH_TERRAIN_PARTITION
    CHeightMapTerrain *pTerrain = new CHeightMapTerrain(pd3dDevice, _T("Image/Terrain/HeightMap?.raw"), 257, 257, 17, 17, d3dxvScale);
#else
    CHeightMapTerrain *pTerrain = new CHeightMapTerrain(pd3dDevice, _T("Image/Terrain/HeightMap.raw"), 257, 257, 257, 257, d3dxvScale);
#endif
    pTerrain->SetMaterial(pTerrainMaterial);

    CShader *pTerrainShader = new CTerrainShader();
    pTerrainShader->CreateShader(pd3dDevice);
    pTerrain->SetShader(pTerrainShader);
    pTerrain->CreateRasterizerState(pd3dDevice);
    pTerrain->SetWireFlag(true);

    // ZoomGUI

		CTextureToScreenRectMesh *pZoomMesh = new CTextureToScreenRectMesh(pd3dDevice);
		CTextureToScreenShader *pZoomShader = new CTextureToScreenShader();
		pZoomShader->CreateShader(pd3dDevice, pZoomMesh->GetType());

		CTexture *pZoomTexture = new CTexture(1, 1, PS_SLOT_TEXTURE_ZOOM, PS_SLOT_SAMPLER_ZOOM);

		ID3D11SamplerState *pd3dZoomSamplerState = nullptr;
		ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
		d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		d3dSamplerDesc.MinLOD = 0;
		d3dSamplerDesc.MaxLOD = 0;
		pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dZoomSamplerState);

		ID3D11ShaderResourceView *pd3dsrvZoomTexture = nullptr;
		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/ZoomUI.dds"), nullptr, nullptr, &pd3dsrvZoomTexture, nullptr);
		pZoomTexture->SetTexture(0, pd3dsrvZoomTexture);
		pZoomTexture->SetSampler(0, pd3dZoomSamplerState);
		pd3dsrvZoomTexture->Release();
		pd3dZoomSamplerState->Release();

		CMaterial *pZoomMaterial = new CMaterial();
		pZoomMaterial->SetTexture(pZoomTexture);

		CGameObject *pZoom = new CGameObject(1);
		pZoom->SetMesh(pZoomMesh);
		pZoom->SetShader(pZoomShader);
		pZoom->SetMaterial(pZoomMaterial);
		pZoom->CreateBlendState(pd3dDevice);
	
    // Aim
	
		CTextureToScreenRectMesh* pAimMesh = new CTextureToScreenRectMesh(pd3dDevice, 0.06f, 0.06f);
		CTextureToScreenShader* pAimShader = pZoomShader;
		CTexture* pAimTexture = new CTexture(1, 1, PS_SLOT_TEXTURE_ZOOM, PS_SLOT_SAMPLER_ZOOM);

		ID3D11ShaderResourceView* pd3dsrvAimTexture = nullptr;

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/aim.dds"), nullptr, nullptr, &pd3dsrvAimTexture, nullptr);

		pAimTexture->SetTexture(0, pd3dsrvAimTexture);
		pAimTexture->SetSampler(0, pd3dSamplerState);
		pd3dsrvAimTexture->Release();

		CMaterial *pAimMaterial = new CMaterial();
		pAimMaterial->SetTexture(pAimTexture);

		CGameObject* pAim = new CGameObject(1);
		pAim->SetMesh(pAimMesh);
		pAim->SetShader(pAimShader);
		pAim->SetMaterial(pAimMaterial);
	
	// Map, Hp, Magazine
	CTextureToScreenRectMesh* pUIMesh = new CTextureToScreenRectMesh(pd3dDevice);
	CTextureToScreenShader* pUIShader = pZoomShader;
	CTexture* pUITexture = new CTexture(1, 1, PS_SLOT_TEXTURE_ZOOM, PS_SLOT_SAMPLER_ZOOM);

	ID3D11ShaderResourceView* pd3dsrvUITexture = nullptr;

	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/MapUI.dds"), nullptr, nullptr, &pd3dsrvUITexture, nullptr);

	pUITexture->SetTexture(0, pd3dsrvUITexture);
	pUITexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvUITexture->Release();

	CMaterial *pUIMaterial = new CMaterial();
	pUIMaterial->SetTexture(pUITexture);

	CGameObject* pUI = new CGameObject(1);
	pUI->SetMesh(pUIMesh);
	pUI->SetShader(pUIShader);
	pUI->SetMaterial(pUIMaterial);

    // Dummy Zombie
    CCubeMeshTextured* pZombieMesh = new CCubeMeshTextured(pd3dDevice, 100.0f, 100.0f, 100.0f);

    CShader* pZombieShader = new CShader();
    pZombieShader->CreateShader(pd3dDevice, pZombieMesh->GetType());

    ID3D11ShaderResourceView* pd3dsrvZombieTexture = nullptr;
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/zombie.dds"), nullptr, nullptr, &pd3dsrvZombieTexture, nullptr);

    CTexture* pZombieTexture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
    pZombieTexture->SetSampler(0, pd3dSamplerState);
    pZombieTexture->SetTexture(0, pd3dsrvZombieTexture);
    pd3dsrvZombieTexture->Release();

    CMaterial* pZombieMaterial = new CMaterial();
    pZombieMaterial->SetTexture(pZombieTexture);

    CGameObject* pZombie = new CGameObject(1);
    pZombie->SetMesh(pZombieMesh);
    pZombie->SetShader(pZombieShader);
    pZombie->SetMaterial(pZombieMaterial);
    pZombie->SetPosition(1000.0f, 0.0f, 1000.0f);
    pZombie->CreateRasterizerState(pd3dDevice);
    pZombie->SetWireFlag(true);

    m_nObjects = 6;
    m_ppObjects = new CGameObject*[m_nObjects];

    m_ppObjects[0] = pSkyBox;
    m_ppObjects[1] = pTerrain;
    m_ppObjects[2] = pZoom;
    m_ppObjects[3] = pAim;
    m_ppObjects[4] = pZombie;
	m_ppObjects[5] = pUI;

    // Bullet
    CCubeMeshTextured *pBulletMesh = new CCubeMeshTextured(pd3dDevice, 0.5f, 0.5f, 0.5f);

    CShader *pBulletShader = new CShader();
    pBulletShader->CreateShader(pd3dDevice, pBulletMesh->GetType());

    ID3D11ShaderResourceView *pd3dsrvBullet;
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/bullet.dds"), nullptr, nullptr, &pd3dsrvBullet, nullptr);

    CTexture *pBulletTexture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
    pBulletTexture->SetSampler(0, pd3dSamplerState);
    pBulletTexture->SetTexture(0, pd3dsrvBullet);
  
    pd3dsrvBullet->Release();

    CMaterial *pBulletMaterial = new CMaterial();
    pBulletMaterial->SetTexture(pBulletTexture);

    for (int i = 0; i < 30; ++i)
    {
        m_pBullets[i].SetMesh(pBulletMesh);
        m_pBullets[i].SetShader(pBulletShader);
        m_pBullets[i].SetMaterial(pBulletMaterial);
        m_pBullets[i].SetActive(false);
    }

     //NPC
	
	ID3D11ShaderResourceView *pd3dsrvNPCTexture;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Character/SimpleMilitary_GasMask_White.png"), nullptr, nullptr, &pd3dsrvNPCTexture, nullptr);	
	CTexture *pNPCTexture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
	pNPCTexture->SetSampler(0, pd3dSamplerState);
	pNPCTexture->SetTexture(0, pd3dsrvNPCTexture);
	pd3dsrvNPCTexture->Release();

	CMaterialColors* pNPCColors = new CMaterialColors();
	pNPCColors->m_d3dxcDiffuse = D3DXCOLOR(0, 1.0f, 0.0f, 1.0f);
	CMaterial* NPCMaterial = new CMaterial(pNPCColors);
	NPCMaterial->SetTexture(pNPCTexture);

    CSkinnedMesh* pNPCMesh = new CSkinnedMesh(pd3dDevice, "Character/Gasmask.data", 0.011f);

	//무조건 됨

    m_nInstancingShaders = 1;
    m_ppInstancingShaders = new CInstancedObjectsShader*[m_nInstancingShaders];//쉐이더를 만드는 것?

    m_ppInstancingShaders[0] = new CInstancedObjectsShader(MAX_ENEMY);
    m_ppInstancingShaders[0]->SetMesh(pNPCMesh);
    m_ppInstancingShaders[0]->SetMaterial(NPCMaterial);
	//여기까지 됐음
	unsigned int NPCmeshtype =
		VERTEX_POSITION_ELEMENT |
		VERTEX_NORMAL_ELEMENT |
		VERTEX_TEXTURE_ELEMENT_0 |
		VERTEX_BONE_ID_ELEMENT |
		VERTEX_BONE_WEIGHT_ELEMENT |
		VERTEX_INSTANCING_ELEMENT;

    m_ppInstancingShaders[0]->BuildObjects(pd3dDevice, NULL);
    m_ppInstancingShaders[0]->CreateShader(pd3dDevice, NPCmeshtype);

   
	for (int i = 0; i < MAX_ENEMY; ++i)
	{
		CGameObject* pNPC = pNPC = new CGameObject();
		pNPC->SetPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
		pNPC->SetWireFlag(false);
		pNPC->CreateRasterizerState(pd3dDevice);
		m_ppInstancingShaders[0]->AddObject(pNPC);
	}

	


	// OtherPlayer
	CMaterialColors *pOtherColor = new CMaterialColors();
	CMaterial *pOtherMat = new CMaterial(pOtherColor);

	ID3D11ShaderResourceView *pd3dsrvOtherPlayerTexture;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Character/SimpleMilitary_SpecialForces_03_White.png"), nullptr, nullptr, &pd3dsrvOtherPlayerTexture, nullptr);
	CTexture *pOtherPlayerTexture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
	pOtherPlayerTexture->SetSampler(0, pd3dSamplerState);
	pOtherPlayerTexture->SetTexture(0, pd3dsrvOtherPlayerTexture);
	pd3dsrvOtherPlayerTexture->Release();
	//pd3dSamplerState->Release();

	pOtherMat->SetTexture(pOtherPlayerTexture);

	//CSkinnedMesh *pOtherPlayermesh = new CSkinnedMesh(pd3dDevice, "Character/speicalforce3B.data", 0.066f);
	//CCubeMeshDiffused *pCubeMesh = new CCubeMeshDiffused(pd3dDevice, 10.0f, 10.0f, 10.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f));
	CCharacterShader* pOtherPlayerShader = new CCharacterShader();
	pOtherPlayerShader->CreateShader(pd3dDevice);
	
	m_nOtherPlayer = MAX_USER;
	m_ppOtherPlayer = new CGameObject*[m_nOtherPlayer];

	CGameObject *pOtherPlayer = nullptr;
	
	for (int i = 0; i < m_nOtherPlayer; ++i)
	{
		
		pOtherPlayer = new CGameObject(1);
		pOtherPlayer->SetMesh(new CSkinnedMesh(pd3dDevice, "Character/speicalforce3B.data", 0.008f));
		pOtherPlayer->GetMesh()->IdleSet();
		pOtherPlayer->SetPosition(0.0f,1.3f, i * 10.0f);
		pOtherPlayer->SetMaterial(pOtherMat);
		pOtherPlayer->SetShader(pOtherPlayerShader);
		
		m_ppOtherPlayer[i] = pOtherPlayer;
	}

	//Steer Test

	CMaterialColors *pSteerColor = new CMaterialColors();
	CMaterial **ppSteerMaterial = new CMaterial*[3];
	for (auto i = 0; i < 3; ++i) {
		ppSteerMaterial[i] = new CMaterial(pSteerColor);
	}

	ID3D11ShaderResourceView *pd3dsrvSteerCrow1Texture;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Crow1.png"), nullptr, nullptr, &pd3dsrvSteerCrow1Texture, nullptr);
	CTexture *pSteerCrow1Texture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
	pSteerCrow1Texture->SetSampler(0, pd3dSamplerState);
	pSteerCrow1Texture->SetTexture(0, pd3dsrvSteerCrow1Texture);
	pd3dsrvSteerCrow1Texture->Release();
	ppSteerMaterial[0]->SetTexture(pSteerCrow1Texture);

	ID3D11ShaderResourceView *pd3dsrvSteerCrow2Texture;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Crow2.png"), nullptr, nullptr, &pd3dsrvSteerCrow2Texture, nullptr);
	CTexture *pSteerCrow2Texture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
	pSteerCrow2Texture->SetSampler(0, pd3dSamplerState);
	pSteerCrow2Texture->SetTexture(0, pd3dsrvSteerCrow2Texture);
	pd3dsrvSteerCrow2Texture->Release();
	ppSteerMaterial[1]->SetTexture(pSteerCrow2Texture);

	ID3D11ShaderResourceView *pd3dsrvSteerCrow3Texture;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Crow3.png"), nullptr, nullptr, &pd3dsrvSteerCrow3Texture, nullptr);
	CTexture *pSteerCrow3Texture = new CTexture(1, 1, PS_SLOT_TEXTURE, PS_SLOT_SAMPLER);
	pSteerCrow3Texture->SetSampler(0, pd3dSamplerState);
	pSteerCrow3Texture->SetTexture(0, pd3dsrvSteerCrow3Texture);
	pd3dsrvSteerCrow3Texture->Release();
	ppSteerMaterial[2]->SetTexture(pSteerCrow3Texture);

	pd3dSamplerState->Release();

	CGSRectMesh **ppSteerMesh = new CGSRectMesh*[3];
	m_ppSteerShader = new CSteerGSInstancedObjectShader*[3];

	for (auto i = 0; i < 3; ++i) {
		m_ppSteerShader[i] = new CSteerGSInstancedObjectShader(100);
		ppSteerMesh[i] = new CGSRectMesh(pd3dDevice, 0, 0);
		m_ppSteerShader[i]->SetMesh(ppSteerMesh[i]);
		m_ppSteerShader[i]->SetMaterial(ppSteerMaterial[i]);
		m_ppSteerShader[i]->BuildObjects(pd3dDevice, nullptr);
		m_ppSteerShader[i]->CreateShader(pd3dDevice);
	}

	CGameObject *pSteer = nullptr;
	for (auto k = 0; k < 3; ++k) {
		for (auto i = 0; i < 100; ++i) {
			pSteer = new CGameObject(1);
			pSteer->SetMesh(ppSteerMesh[k]);
			pSteer->SetPosition(0.0f, 200.0f, 0.0f);
			m_ppSteerShader[k]->AddObject(pSteer);
		}
	}


#define TEST508
#ifdef TEST508
	ID3D11ShaderResourceView *pd3dsrvTexture = NULL;
#pragma region
	CTexture *pBuilding01a_Orange_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building01a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding01a_Orange_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding01a_Orange_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBuilding01b_Red_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building01b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding01b_Red_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding01b_Red_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBuilding01c_Brown_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building01c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding01c_Brown_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding01c_Brown_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();


	CTexture *pBuilding01d_Green_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/STL_Building_Apartment_02.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding01d_Green_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding01d_Green_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	//clear
	CTexture *pBuilding02a_Video_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building02a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding02a_Video_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding02a_Video_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBuilding02b_Pawn_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building02b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding02b_Pawn_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding02b_Pawn_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBuilding02c_Drug_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building02c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding02c_Drug_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding02c_Drug_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear
	CTexture *pBuilding02d_Book_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Shop_Damaged.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding02d_Book_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding02d_Book_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear
	CTexture *pBuilding03a_Blue_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building03a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding03a_Blue_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding03a_Blue_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBuilding03b_Grey_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building03b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding03b_Grey_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding03b_Grey_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBuilding03c_Brown_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Building03c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding03c_Brown_Texture->SetTexture(0, pd3dsrvTexture);
	pBuilding03c_Brown_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pHouse_01_Red_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/House_01.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pHouse_01_Red_Texture->SetTexture(0, pd3dsrvTexture);
	pHouse_01_Red_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pHouse_02_Green_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/House_02.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pHouse_02_Green_Texture->SetTexture(0, pd3dsrvTexture);
	pHouse_02_Green_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pHouse_03_Orange_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/House_03.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pHouse_03_Orange_Texture->SetTexture(0, pd3dsrvTexture);
	pHouse_03_Orange_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pNature_TreeTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Nature_Trees.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pNature_TreeTexture->SetTexture(0, pd3dsrvTexture);
	pNature_TreeTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pPropertyTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Props.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pPropertyTexture->SetTexture(0, pd3dsrvTexture);
	pPropertyTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pProps_Billboard_01_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Props_Billboard.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pProps_Billboard_01_Texture->SetTexture(0, pd3dsrvTexture);
	pProps_Billboard_01_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pProps_Billboard_02_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/STL_Props_Billboard_02.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pProps_Billboard_02_Texture->SetTexture(0, pd3dsrvTexture);
	pProps_Billboard_02_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pProps_DumpsterGreenTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Props_Dumpster.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pProps_DumpsterGreenTexture->SetTexture(0, pd3dsrvTexture);
	pProps_DumpsterGreenTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pProps_DumpsterBlueTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/STL_Props_Dumpster.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pProps_DumpsterBlueTexture->SetTexture(0, pd3dsrvTexture);
	pProps_DumpsterBlueTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pRoadTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Road.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pRoadTexture->SetTexture(0, pd3dsrvTexture);
	pRoadTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pRoadDividerTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Road_divider.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pRoadDividerTexture->SetTexture(0, pd3dsrvTexture);
	pRoadDividerTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pSimpleTownTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/SimpleTown.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pSimpleTownTexture->SetTexture(0, pd3dsrvTexture);
	pSimpleTownTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear
	CTexture *pBuilding_PizzaTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/STL_Building_Pizza.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pBuilding_PizzaTexture->SetTexture(0, pd3dsrvTexture);
	pBuilding_PizzaTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

#pragma endregion

	//vehicle texture start
#pragma region
	CTexture *pVehi_Car_Blue_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Car01_a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Car_Blue_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Car_Blue_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Car_Red_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Car01_b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Car_Red_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Car_Red_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Car_Green_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Car01_c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Car_Green_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Car_Green_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();


	CTexture *pVehi_Bus_Brown_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Bus_a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Bus_Brown_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Bus_Brown_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Bus_Grey_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Bus_b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Bus_Grey_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Bus_Grey_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Bus_Blue_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Bus_c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Bus_Blue_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Bus_Blue_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pVehi_Ambulance_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehice_Ambulance.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Ambulance_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Ambulance_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_FireTruck_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Fire_Truck.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_FireTruck_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_FireTruck_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_PoliceCarTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Police.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_PoliceCarTexture->SetTexture(0, pd3dsrvTexture);
	pVehi_PoliceCarTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_RubbishTruckTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Rubbish_Truck.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_RubbishTruckTexture->SetTexture(0, pd3dsrvTexture);
	pVehi_RubbishTruckTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_TaxiTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Taxi.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_TaxiTexture->SetTexture(0, pd3dsrvTexture);
	pVehi_TaxiTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pVehi_Ute_Red_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Ute_a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Ute_Red_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Ute_Red_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Ute_Yellow_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Ute_b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Ute_Yellow_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Ute_Yellow_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Ute_Blue_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Ute_c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Ute_Blue_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Ute_Blue_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//clear

	CTexture *pVehi_Van_Blue_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Van_a.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Van_Blue_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Van_Blue_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Van_Green_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Van_b.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Van_Green_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Van_Green_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_Van_Red_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/Vehicle_Van_c.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_Van_Red_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_Van_Red_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_PizzaCar_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/STL_Vehicle_PizzaCar.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_PizzaCar_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_PizzaCar_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pVehi_HotDogTruck_Texture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Textures/STL_Vehicle_HotdogTruck.png"), NULL, NULL, &pd3dsrvTexture, NULL);
	pVehi_HotDogTruck_Texture->SetTexture(0, pd3dsrvTexture);
	pVehi_HotDogTruck_Texture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	//Texture All Clear 2016.04.13
#pragma endregion
	pd3dSamplerState->Release();


	CAssetMesh* pAmbulanceMesh = new CAssetMesh(pd3dDevice, Vehicle::Ambulance);
	CAssetMesh* pCopMesh = new CAssetMesh(pd3dDevice, Vehicle::Cop);
	CAssetMesh* pCarMesh = new CAssetMesh(pd3dDevice, Vehicle::Car);
	CAssetMesh* pBusMesh = new CAssetMesh(pd3dDevice, Vehicle::Bus);
	CAssetMesh* pTaxiMesh = new CAssetMesh(pd3dDevice, Vehicle::taxi);
	CAssetMesh* pUTEMesh = new CAssetMesh(pd3dDevice, Vehicle::ute);
	CAssetMesh* pUTEemptyMesh = new CAssetMesh(pd3dDevice, Vehicle::ute_empty);
	CAssetMesh* pRubbish_truckMesh = new CAssetMesh(pd3dDevice, Vehicle::Rubbish_truck);
	CAssetMesh* pVanMesh = new CAssetMesh(pd3dDevice, Vehicle::van);
	CAssetMesh* pFireTruckMesh = new CAssetMesh(pd3dDevice, Vehicle::fire_truck);
	CAssetMesh* pPizzaCarMesh = new CAssetMesh(pd3dDevice, Vehicle::PizzaCar_seperate);
	CAssetMesh* pHotDogTruckMesh = new CAssetMesh(pd3dDevice, Vehicle::HotdogTruck_seperate);

	CAssetMesh* pBoat3 = new CAssetMesh(pd3dDevice, Vehicle::Boat3);
	CAssetMesh* pBoat2 = new CAssetMesh(pd3dDevice, Vehicle::Boat2);
	CAssetMesh* pBoat1 = new CAssetMesh(pd3dDevice, Vehicle::Boat1);


	//Building
	CAssetMesh* pApartment_Large_mesh = new CAssetMesh(pd3dDevice, Building::Apartment_large);
	CAssetMesh* pApartment_Small_mesh = new CAssetMesh(pd3dDevice, Building::Apartment_small);
	CAssetMesh* pBarbershop_mesh = new CAssetMesh(pd3dDevice, Building::BarberShop);
	CAssetMesh* pCinema_mesh = new CAssetMesh(pd3dDevice, Building::Cinema);
	CAssetMesh* pCoffeeShop_mesh = new CAssetMesh(pd3dDevice, Building::CoffeeShop);
	CAssetMesh* pGarage01_mesh = new CAssetMesh(pd3dDevice, Building::Garage1);
	CAssetMesh* pGarage02_mesh = new CAssetMesh(pd3dDevice, Building::Garage2);
	CAssetMesh* pGarage03_mesh = new CAssetMesh(pd3dDevice, Building::Garage3);
	CAssetMesh* pAutoRepair_mesh = new CAssetMesh(pd3dDevice, Building::AutoRepair);
	CAssetMesh* pGym_mesh = new CAssetMesh(pd3dDevice, Building::Gym);
	CAssetMesh* pHouse1_mesh = new CAssetMesh(pd3dDevice, Building::House1);//to 15	
	CAssetMesh* pHouse2_mesh = new CAssetMesh(pd3dDevice, Building::House2);//to 15	
	CAssetMesh* pHouse3_mesh = new CAssetMesh(pd3dDevice, Building::House3);//to 15	
	CAssetMesh* pHouse4_mesh = new CAssetMesh(pd3dDevice, Building::House4);//to 15	
	CAssetMesh* pHouse5_mesh = new CAssetMesh(pd3dDevice, Building::House5);//to 15	
	CAssetMesh* pHouse6_mesh = new CAssetMesh(pd3dDevice, Building::House6);//to 15	
	CAssetMesh* pHouse7_mesh = new CAssetMesh(pd3dDevice, Building::House7);//to 15	
	CAssetMesh* pHouse8_mesh = new CAssetMesh(pd3dDevice, Building::House8);//to 15	
	CAssetMesh* pHouse9_mesh = new CAssetMesh(pd3dDevice, Building::House9);//to 15	
	CAssetMesh* pHouse10_mesh = new CAssetMesh(pd3dDevice, Building::House10);//to 15	
	CAssetMesh* pHouse11_mesh = new CAssetMesh(pd3dDevice, Building::House11);//to 15	
	CAssetMesh* pHouse12_mesh = new CAssetMesh(pd3dDevice, Building::House12);//to 15	
	CAssetMesh* pHouse13_mesh = new CAssetMesh(pd3dDevice, Building::House13);//to 15	
	CAssetMesh* pHouse14_mesh = new CAssetMesh(pd3dDevice, Building::House14);//to 15	
	CAssetMesh* pHouse15_mesh = new CAssetMesh(pd3dDevice, Building::House15);//to 15	

	CAssetMesh* pMall_A_mesh = new CAssetMesh(pd3dDevice, Building::Mall_A);//A to B
	CAssetMesh* pMall_B_mesh = new CAssetMesh(pd3dDevice, Building::Mall_B);//
	CAssetMesh* pShop1_mesh = new CAssetMesh(pd3dDevice, Building::Shop1);
	CAssetMesh* pShop2_mesh = new CAssetMesh(pd3dDevice, Building::Shop2);
	CAssetMesh* pShop3_mesh = new CAssetMesh(pd3dDevice, Building::Shop3);
	CAssetMesh* pShop4_mesh = new CAssetMesh(pd3dDevice, Building::Shop4);
	CAssetMesh* pShop5_mesh = new CAssetMesh(pd3dDevice, Building::Shop5);
	CAssetMesh* pStripClub_mesh = new CAssetMesh(pd3dDevice, Building::StripClub);
	CAssetMesh* pStore_Corner_mesh = new CAssetMesh(pd3dDevice, Building::Store_Corner);
	CAssetMesh* pHouse_mesh = new CAssetMesh(pd3dDevice, Building::House);
	CAssetMesh* pOffice_Large_mesh = new CAssetMesh(pd3dDevice, Building::Office_large);
	CAssetMesh* pOffice_Medium_mesh = new CAssetMesh(pd3dDevice, Building::Office_medium);
	CAssetMesh* pOffice_Small_mesh = new CAssetMesh(pd3dDevice, Building::Office_small);
	CAssetMesh* pOffice_Stepped_mesh = new CAssetMesh(pd3dDevice, Building::Office_Stepped);
	CAssetMesh* pPizza_shop_mesh = new CAssetMesh(pd3dDevice, Building::Pizza_Shop);
	CAssetMesh* pApartment_large_thin_short_mesh = new CAssetMesh(pd3dDevice, Building::apartment_large_thin_short);
	CAssetMesh* pStore_small_mesh = new CAssetMesh(pd3dDevice, Building::store_small);

	//Property
	CAssetMesh* pAriel_mesh = new CAssetMesh(pd3dDevice, Property::ariel);
	CAssetMesh* pTree_large_mesh = new CAssetMesh(pd3dDevice, Property::tree_large);
	CAssetMesh* pTree_medium_mesh = new CAssetMesh(pd3dDevice, Property::tree_medium);
	CAssetMesh* pTree_small_mesh = new CAssetMesh(pd3dDevice, Property::tree_small);
	CAssetMesh* pTraffic_light_mesh = new CAssetMesh(pd3dDevice, Property::traffic_light);
	CAssetMesh* pTree01_mesh = new CAssetMesh(pd3dDevice, Property::Tree01);
	CAssetMesh* pTree02_mesh = new CAssetMesh(pd3dDevice, Property::Tree02);
	CAssetMesh* pUmbrella1_mesh = new CAssetMesh(pd3dDevice, Property::Umbrella1);
	CAssetMesh* pUmbrella2_mesh = new CAssetMesh(pd3dDevice, Property::Umbrella2);
	CAssetMesh* pUmbrella3_mesh = new CAssetMesh(pd3dDevice, Property::Umbrella3);
	CAssetMesh* pRoadSign1_mesh = new CAssetMesh(pd3dDevice, Property::RoadSign1);
	CAssetMesh* pRoadSign2_mesh = new CAssetMesh(pd3dDevice, Property::RoadSign2);
	CAssetMesh* pRoadSign3_mesh = new CAssetMesh(pd3dDevice, Property::RoadSign3);
	CAssetMesh* pTirePile = new CAssetMesh(pd3dDevice, Property::TirePile);
	CAssetMesh* pGrave_large = new CAssetMesh(pd3dDevice, Property::grave_large);
	CAssetMesh* pGrave_Medium = new CAssetMesh(pd3dDevice, Property::grave_medium);
	CAssetMesh* pGrave_small = new CAssetMesh(pd3dDevice, Property::grave_small);
	CAssetMesh* pBeachSeat1 = new CAssetMesh(pd3dDevice, Property::BeachSeat1);
	CAssetMesh* pBeachSeat2 = new CAssetMesh(pd3dDevice, Property::BeachSeat2);
	CAssetMesh* pBeachSeat3 = new CAssetMesh(pd3dDevice, Property::BeachSeat3);
	CAssetMesh* pLamp = new CAssetMesh(pd3dDevice, Property::lamp);
	CAssetMesh* pMemorial = new CAssetMesh(pd3dDevice, Property::memorial);
	CAssetMesh* pHydrant = new CAssetMesh(pd3dDevice, Property::hydrant);
	CAssetMesh* pPipeMesh = new CAssetMesh(pd3dDevice, Property::pipe_mesh);
	CAssetMesh* pHedge_mesh = new CAssetMesh(pd3dDevice, Property::hedge_mesh);
	CAssetMesh* pFence_long_mesh = new CAssetMesh(pd3dDevice, Property::fence_long);
	CAssetMesh* pFence_short_mesh = new CAssetMesh(pd3dDevice, Property::fence_short);
	CAssetMesh* pFence_long_spike_mesh = new CAssetMesh(pd3dDevice, Property::fence_long_spike);
	CAssetMesh* pFence_short_spike_mesh = new CAssetMesh(pd3dDevice, Property::fence_short_spike);
	CAssetMesh* pFlag = new CAssetMesh(pd3dDevice, Property::flag);
	CAssetMesh* pDumpster_mesh = new CAssetMesh(pd3dDevice, Property::dumpter_mesh);
	CAssetMesh* pDish_mesh = new CAssetMesh(pd3dDevice, Property::dish_mesh);
	CAssetMesh* pBush_large_mesh = new CAssetMesh(pd3dDevice, Property::bush_large_mesh);
	CAssetMesh* pBush_small_mesh = new CAssetMesh(pd3dDevice, Property::bush_small_mesh);
	CAssetMesh* pbin_mesh = new CAssetMesh(pd3dDevice, Property::bin_mesh);
	CAssetMesh* pbillboard_mesh = new CAssetMesh(pd3dDevice, Property::billboard_mesh);
	CAssetMesh* ptrash_mesh = new CAssetMesh(pd3dDevice, Property::trash_mesh);
	CAssetMesh* pProps_Buoy_01 = new CAssetMesh(pd3dDevice, Property::Buoy1);
	CAssetMesh* pProps_Buoy_02 = new CAssetMesh(pd3dDevice, Property::Buoy2);

	//environment
	CAssetMesh* pEnv_Beach_Corner = new CAssetMesh(pd3dDevice, Environment::Env_Beach_Corner);
	CAssetMesh* pEnv_Beach_Short = new CAssetMesh(pd3dDevice, Environment::Env_Beach_Short);
	CAssetMesh* pEnv_Beach_Straight = new CAssetMesh(pd3dDevice, Environment::Env_Beach_Straight);
	CAssetMesh* pEnv_Canal_Corner_01 = new CAssetMesh(pd3dDevice, Environment::Env_Canal_Corner_01);
	CAssetMesh* pEnv_Canal_Corner_02 = new CAssetMesh(pd3dDevice, Environment::Env_Canal_Corner_02);
	CAssetMesh* pEnv_Canal_Corner_03 = new CAssetMesh(pd3dDevice, Environment::Env_Canal_Corner_03);
	CAssetMesh* pEnv_Canal_End = new CAssetMesh(pd3dDevice, Environment::Env_Canal_End);
	CAssetMesh* pEnv_Canal_Pipe_01 = new CAssetMesh(pd3dDevice, Environment::Env_Canal_Pipe_01);
	CAssetMesh* pEnv_Canal_Pipe_02 = new CAssetMesh(pd3dDevice, Environment::Env_Canal_Pipe_02);
	CAssetMesh* pEnv_Canal_Straight = new CAssetMesh(pd3dDevice, Environment::Env_Canal_Straight);
	CAssetMesh* pEnv_Car_Bridge = new CAssetMesh(pd3dDevice, Environment::Env_Car_Bridge);
	CAssetMesh* pEnv_Car_Bridge_02 = new CAssetMesh(pd3dDevice, Environment::Env_Car_Bridge_02);
	CAssetMesh* pEnv_Foot_Bridge = new CAssetMesh(pd3dDevice, Environment::Env_Foot_Bridge);
	CAssetMesh* pEnv_Jetty = new CAssetMesh(pd3dDevice, Environment::Env_Jetty);
	CAssetMesh* pEnv_Planter = new CAssetMesh(pd3dDevice, Environment::Env_Planter);
	CAssetMesh* pEnv_Road_Corner = new CAssetMesh(pd3dDevice, Environment::Env_Road_Corner);
	CAssetMesh* pEnv_Road_Ramp = new CAssetMesh(pd3dDevice, Environment::Env_Road_Ramp);
	CAssetMesh* pEnv_Road_Ramp_Pillar = new CAssetMesh(pd3dDevice, Environment::Env_Road_Ramp_Pillar);
	CAssetMesh* pEnv_Road_Ramp_Straight = new CAssetMesh(pd3dDevice, Environment::Env_Road_Ramp_Straight);
	CAssetMesh* pEnv_Rocks_01 = new CAssetMesh(pd3dDevice, Environment::Env_Rocks_01);
	CAssetMesh* pEnv_Rocks_02 = new CAssetMesh(pd3dDevice, Environment::Env_Rocks_02);
	CAssetMesh* pEnv_Rocks_03 = new CAssetMesh(pd3dDevice, Environment::Env_Rocks_03);
	CAssetMesh* pEnv_Seawall_Straight = new CAssetMesh(pd3dDevice, Environment::Env_Seawall_Straight);
	CAssetMesh* pEnv_Seawall_Corner_01 = new CAssetMesh(pd3dDevice, Environment::Env_Seawall_Corner_01);
	CAssetMesh* pEnv_Seawall_Corner_02 = new CAssetMesh(pd3dDevice, Environment::Env_Seawall_Corner_02);
	CAssetMesh* pEnv_Seawall_Corner_03 = new CAssetMesh(pd3dDevice, Environment::Env_Seawall_Corner_03);
	CAssetMesh* pEnv_Seawall_Wall = new CAssetMesh(pd3dDevice, Environment::Env_Seawall_Wall);
	CAssetMesh* pEnv_Water_Tile = new CAssetMesh(pd3dDevice, Environment::Env_Water_Tile);
	CAssetMesh* pflower_mesh = new CAssetMesh(pd3dDevice, Environment::flower_mesh);
	CAssetMesh* pGrass_square2_mesh = new CAssetMesh(pd3dDevice, Environment::grass_square2_mesh);
	CAssetMesh* pGrass_square_mesh = new CAssetMesh(pd3dDevice, Environment::grass_square_mesh);
	CAssetMesh* pPath_cross_mesh = new CAssetMesh(pd3dDevice, Environment::path_cross_mesh);
	CAssetMesh* pPath_driveway = new CAssetMesh(pd3dDevice, Environment::path_driveway);
	CAssetMesh* pPath_straight_mesh = new CAssetMesh(pd3dDevice, Environment::path_straight_mesh);
	CAssetMesh* pRoadLane_NoLines_straight_Centered_mesh = new CAssetMesh(pd3dDevice, Environment::roadLane_NoLines_straight_Centered_mesh);
	CAssetMesh* pRoadLane_Nolines_straight_mesh = new CAssetMesh(pd3dDevice, Environment::roadLane_Nolines_straight_mesh);
	CAssetMesh* pRoadLane_straight_Centered_mesh = new CAssetMesh(pd3dDevice, Environment::roadLane_straight_Centered_mesh);
	CAssetMesh* pRoad_bend_left_mesh = new CAssetMesh(pd3dDevice, Environment::road_bend_left_mesh);
	CAssetMesh* pRoad_bend_right_mesh = new CAssetMesh(pd3dDevice, Environment::road_bend_right_mesh);
	CAssetMesh* pRoad_cornerLines_mesh = new CAssetMesh(pd3dDevice, Environment::road_cornerLines_mesh);
	CAssetMesh* pRoad_corner_mesh = new CAssetMesh(pd3dDevice, Environment::road_corner_mesh);
	CAssetMesh* pRoad_crossing_mesh = new CAssetMesh(pd3dDevice, Environment::road_crossing_mesh);
	CAssetMesh* pRoad_divider_mesh = new CAssetMesh(pd3dDevice, Environment::road_divider_mesh);
	CAssetMesh* pRoad_LaneTransition_Left = new CAssetMesh(pd3dDevice, Environment::road_LaneTransition_Left);
	CAssetMesh* pRoad_LaneTransition_Right = new CAssetMesh(pd3dDevice, Environment::road_LaneTransition_Right);
	CAssetMesh* pRoad_Roundabout = new CAssetMesh(pd3dDevice, Environment::road_Roundabout);
	CAssetMesh* pRoad_square_mesh = new CAssetMesh(pd3dDevice, Environment::road_square_mesh);
	CAssetMesh* pRoad_straight_clear_mesh = new CAssetMesh(pd3dDevice, Environment::road_straight_clear_mesh);
	CAssetMesh* pRoad_straight_mesh = new CAssetMesh(pd3dDevice, Environment::road_straight_mesh);
	CAssetMesh* pRoad_t_mesh = new CAssetMesh(pd3dDevice, Environment::road_t_mesh);

#pragma endregion

	std::map<std::string, ScriptContainer> script_map;
	//scriptname, mesh, texture
	script_map["road_straight_mesh"] = ScriptContainer{ pRoad_straight_mesh, pRoadTexture };
	script_map["road_straight_clear_mesh"] = ScriptContainer{ pRoad_straight_clear_mesh, pRoadTexture };
	script_map["road_square_mesh"] = ScriptContainer{ pRoad_square_mesh, pRoadTexture };
	script_map["Prop_Bin"] = ScriptContainer{ pbin_mesh, pPropertyTexture };
	script_map["Vehicles_PizzaCar"] = ScriptContainer{ pPizzaCarMesh, pVehi_PizzaCar_Texture };
	script_map["Vehicles_HotdogTruck"] = ScriptContainer{ pHotDogTruckMesh, pVehi_HotDogTruck_Texture };
	script_map["Building_PizzaShop"] = ScriptContainer{ pPizza_shop_mesh, pBuilding_PizzaTexture };
	script_map["Vehicle_Boat_03"] = ScriptContainer{ pBoat3, pSimpleTownTexture };
	script_map["Vehicle_Boat_02"] = ScriptContainer{ pBoat2, pSimpleTownTexture };
	script_map["Vehicle_Boat_01"] = ScriptContainer{ pBoat1, pSimpleTownTexture };
	script_map["van_seperate_red"] = ScriptContainer{ pVanMesh, pVehi_Van_Red_Texture };
	script_map["van_seperate_green"] = ScriptContainer{ pVanMesh, pVehi_Van_Green_Texture };
	script_map["van_seperate_blue"] = ScriptContainer{ pVanMesh, pVehi_Van_Blue_Texture };
	script_map["van_mesh_red"] = ScriptContainer{ pVanMesh, pVehi_Van_Red_Texture };
	script_map["van_mesh_green"] = ScriptContainer{ pVanMesh, pVehi_Van_Green_Texture };
	script_map["van_mesh_blue"] = ScriptContainer{ pVanMesh, pVehi_Van_Blue_Texture };
	script_map["ute_seperate_yellow"] = ScriptContainer{ pUTEMesh, pVehi_Ute_Yellow_Texture };
	script_map["ute_seperate_red"] = ScriptContainer{ pUTEMesh, pVehi_Ute_Red_Texture };
	script_map["ute_seperate_blue"] = ScriptContainer{ pUTEMesh, pVehi_Ute_Blue_Texture };
	script_map["ute_mesh_yellow"] = ScriptContainer{ pUTEMesh, pVehi_Ute_Yellow_Texture };
	script_map["ute_mesh_red"] = ScriptContainer{ pUTEMesh, pVehi_Ute_Red_Texture };
	script_map["ute_mesh_blue"] = ScriptContainer{ pUTEMesh, pVehi_Ute_Blue_Texture };
	script_map["ute_empty_seperate_yellow"] = ScriptContainer{ pUTEemptyMesh, pVehi_Ute_Yellow_Texture };
	script_map["ute_empty_seperate_red"] = ScriptContainer{ pUTEemptyMesh, pVehi_Ute_Red_Texture };
	script_map["ute_empty_seperate_blue"] = ScriptContainer{ pUTEemptyMesh, pVehi_Ute_Blue_Texture };
	script_map["ute_empty_yellow"] = ScriptContainer{ pUTEemptyMesh, pVehi_Ute_Yellow_Texture };
	script_map["ute_empty_red"] = ScriptContainer{ pUTEemptyMesh, pVehi_Ute_Red_Texture };
	script_map["ute_empty_blue"] = ScriptContainer{ pUTEemptyMesh, pVehi_Ute_Blue_Texture };
	script_map["taxi_seperate_mesh"] = ScriptContainer{ pTaxiMesh, pVehi_TaxiTexture };
	script_map["taxi_mesh"] = ScriptContainer{ pTaxiMesh, pVehi_TaxiTexture };
	script_map["rubbishTruck_mesh"] = ScriptContainer{ pRubbish_truckMesh, pVehi_RubbishTruckTexture };
	script_map["rubbish_truck_seperate_mesh"] = ScriptContainer{ pRubbish_truckMesh, pVehi_RubbishTruckTexture };
	script_map["fire_truck_seperate_mesh"] = ScriptContainer{ pFireTruckMesh, pVehi_FireTruck_Texture };
	script_map["fire_truck_mesh"] = ScriptContainer{ pFireTruckMesh, pVehi_FireTruck_Texture };
	script_map["cop_seperate_mesh"] = ScriptContainer{ pCopMesh, pVehi_PoliceCarTexture };
	script_map["cop_mesh"] = ScriptContainer{ pCopMesh, pVehi_PoliceCarTexture };
	script_map["car_seperate_red"] = ScriptContainer{ pCarMesh, pVehi_Car_Red_Texture };
	script_map["car_seperate_green"] = ScriptContainer{ pCarMesh, pVehi_Car_Green_Texture };
	script_map["car_seperate_blue"] = ScriptContainer{ pCarMesh, pVehi_Car_Blue_Texture };
	script_map["car_red"] = ScriptContainer{ pCarMesh, pVehi_Car_Red_Texture };
	script_map["car_green"] = ScriptContainer{ pCarMesh, pVehi_Car_Green_Texture };
	script_map["car_blue"] = ScriptContainer{ pCarMesh, pVehi_Car_Blue_Texture };
	script_map["bus_seperate_grey"] = ScriptContainer{ pBusMesh, pVehi_Bus_Grey_Texture };
	script_map["bus_seperate_brown"] = ScriptContainer{ pBusMesh, pVehi_Bus_Brown_Texture };
	script_map["bus_seperate_blue"] = ScriptContainer{ pBusMesh, pVehi_Bus_Blue_Texture };
	script_map["bus_grey"] = ScriptContainer{ pBusMesh, pVehi_Bus_Grey_Texture };
	script_map["bus_brown"] = ScriptContainer{ pBusMesh, pVehi_Bus_Brown_Texture };
	script_map["bus_blue"] = ScriptContainer{ pBusMesh, pVehi_Bus_Blue_Texture };
	script_map["ambo_seperate"] = ScriptContainer{ pAmbulanceMesh, pVehi_Ambulance_Texture };
	script_map["ambo_mesh"] = ScriptContainer{ pAmbulanceMesh, pVehi_Ambulance_Texture };
	script_map["tree_small_mesh"] = ScriptContainer{ pTree_small_mesh, pNature_TreeTexture };
	script_map["tree_medium_mesh"] = ScriptContainer{ pTree_medium_mesh, pNature_TreeTexture };
	script_map["tree_large_mesh"] = ScriptContainer{ pTree_large_mesh, pNature_TreeTexture };
	script_map["trash_mesh"] = ScriptContainer{ ptrash_mesh, pPropertyTexture };
	script_map["traffic_light_mesh"] = ScriptContainer{ pTraffic_light_mesh, pPropertyTexture };
	script_map["Props_Buoy_02"] = ScriptContainer{ pProps_Buoy_02, pSimpleTownTexture };
	script_map["Props_Buoy_01"] = ScriptContainer{ pProps_Buoy_01, pSimpleTownTexture };
	script_map["Prop_Umbrella_03"] = ScriptContainer{ pUmbrella3_mesh, pSimpleTownTexture };
	script_map["Prop_Umbrella_02"] = ScriptContainer{ pUmbrella2_mesh, pSimpleTownTexture };
	script_map["Prop_Umbrella_01"] = ScriptContainer{ pUmbrella1_mesh, pSimpleTownTexture };
	script_map["Prop_Tree_02"] = ScriptContainer{ pTree02_mesh, pSimpleTownTexture };
	script_map["Prop_Tree_01"] = ScriptContainer{ pTree01_mesh, pSimpleTownTexture };
	script_map["Prop_TirePile"] = ScriptContainer{ pCopMesh, pSimpleTownTexture };
	script_map["Prop_Roadsign_03"] = ScriptContainer{ pRoadSign3_mesh, pSimpleTownTexture };
	script_map["Prop_Roadsign_02"] = ScriptContainer{ pRoadSign2_mesh, pSimpleTownTexture };
	script_map["Prop_Roadsign_01"] = ScriptContainer{ pRoadSign1_mesh, pSimpleTownTexture };
	script_map["Prop_Beachseat_03"] = ScriptContainer{ pBeachSeat3, pSimpleTownTexture };
	script_map["Prop_Beachseat_02"] = ScriptContainer{ pBeachSeat2, pSimpleTownTexture };
	script_map["Prop_Beachseat_01"] = ScriptContainer{ pBeachSeat1, pSimpleTownTexture };
	script_map["pipe_mesh"] = ScriptContainer{ pPipeMesh, pPropertyTexture };
	script_map["path_straight_mesh"] = ScriptContainer{ pPath_straight_mesh, pRoadTexture };
	script_map["path_cross_mesh"] = ScriptContainer{ pPath_cross_mesh, pRoadTexture };
	script_map["memorial_mesh"] = ScriptContainer{ pMemorial, pPropertyTexture };
	script_map["lamp_mesh"] = ScriptContainer{ pLamp, pPropertyTexture };
	script_map["hydrant_mesh"] = ScriptContainer{ pHydrant, pPropertyTexture };
	script_map["hedge_mesh"] = ScriptContainer{ pHedge_mesh, pNature_TreeTexture };
	script_map["grave_small_mesh"] = ScriptContainer{ pGrave_small, pPropertyTexture };
	script_map["grave_medium_mesh"] = ScriptContainer{ pGrave_Medium, pPropertyTexture };
	script_map["grave_large_mesh"] = ScriptContainer{ pGrave_large, pPropertyTexture };
	script_map["grass_square_mesh"] = ScriptContainer{ pGrass_square_mesh, pSimpleTownTexture };
	script_map["grass_square2_mesh"] = ScriptContainer{ pGrass_square2_mesh, pNature_TreeTexture };
	script_map["flower_mesh"] = ScriptContainer{ pflower_mesh, pPropertyTexture };
	script_map["flag_mesh"] = ScriptContainer{ pFlag, pPropertyTexture };
	script_map["fence_long_spike"] = ScriptContainer{ pFence_long_spike_mesh, pPropertyTexture };
	script_map["fence_short_spike"] = ScriptContainer{ pFence_short_spike_mesh, pPropertyTexture };
	script_map["fence_short_mesh"] = ScriptContainer{ pFence_short_mesh, pPropertyTexture };
	script_map["fence_long_mesh"] = ScriptContainer{ pFence_long_mesh, pPropertyTexture };
	script_map["Env_Planter"] = ScriptContainer{ pEnv_Planter, pSimpleTownTexture };
	script_map["dish_mesh"] = ScriptContainer{ pDish_mesh, pPropertyTexture };
	script_map["bush_small_mesh"] = ScriptContainer{ pBush_small_mesh, pNature_TreeTexture };
	script_map["bush_large_mesh"] = ScriptContainer{ pBush_large_mesh, pNature_TreeTexture };
	script_map["Aerial_mesh"] = ScriptContainer{ pAriel_mesh, pPropertyTexture };
	script_map["roadLane_straight_Centered_mesh"] = ScriptContainer{ pRoadLane_straight_Centered_mesh, pRoadTexture };
	script_map["road_t_mesh"] = ScriptContainer{ pRoad_t_mesh, pRoadTexture };
	script_map["road_Roundabout"] = ScriptContainer{ pRoad_Roundabout, pSimpleTownTexture };
	script_map["road_LaneTransition_Right"] = ScriptContainer{ pRoad_LaneTransition_Right, pSimpleTownTexture };
	script_map["road_LaneTransition_Left"] = ScriptContainer{ pRoad_LaneTransition_Left, pSimpleTownTexture };
	script_map["road_divider_mesh"] = ScriptContainer{ pRoad_divider_mesh, pRoadDividerTexture };
	script_map["road_crossing_mesh"] = ScriptContainer{ pRoad_crossing_mesh, pRoadTexture };
	script_map["road_cornerLines_mesh"] = ScriptContainer{ pRoad_cornerLines_mesh, pRoadTexture };
	script_map["road_corner_mesh"] = ScriptContainer{ pRoad_corner_mesh, pRoadTexture };
	script_map["road_bend_right_mesh"] = ScriptContainer{ pRoad_bend_right_mesh, pRoadTexture };
	script_map["road_bend_left_mesh"] = ScriptContainer{ pRoad_bend_left_mesh, pRoadTexture };
	script_map["path_driveway"] = ScriptContainer{ pPath_driveway, pRoadTexture };
	script_map["Env_Water_Tile"] = ScriptContainer{ pEnv_Water_Tile, pSimpleTownTexture };
	script_map["Env_Seawall_Wall"] = ScriptContainer{ pEnv_Seawall_Wall, pSimpleTownTexture };
	script_map["Env_Seawall_Straight"] = ScriptContainer{ pEnv_Seawall_Straight, pSimpleTownTexture };
	script_map["Env_Seawall_Corner_03"] = ScriptContainer{ pEnv_Seawall_Corner_03, pSimpleTownTexture };
	script_map["Env_Seawall_Corner_02"] = ScriptContainer{ pEnv_Seawall_Corner_02, pSimpleTownTexture };
	script_map["Env_Seawall_Corner_01"] = ScriptContainer{ pEnv_Seawall_Corner_01, pSimpleTownTexture };
	script_map["Env_Rocks_03"] = ScriptContainer{ pEnv_Rocks_03, pSimpleTownTexture };
	script_map["Env_Rocks_02"] = ScriptContainer{ pEnv_Rocks_02, pSimpleTownTexture };
	script_map["Env_Rocks_01"] = ScriptContainer{ pEnv_Rocks_01, pSimpleTownTexture };
	script_map["Env_Road_Ramp_Straight"] = ScriptContainer{ pEnv_Road_Ramp_Straight, pSimpleTownTexture };
	script_map["Env_Road_Ramp_Pillar"] = ScriptContainer{ pEnv_Road_Ramp_Pillar, pSimpleTownTexture };
	script_map["Env_Road_Ramp"] = ScriptContainer{ pEnv_Road_Ramp, pSimpleTownTexture };
	script_map["Env_Road_Corner"] = ScriptContainer{ pEnv_Road_Corner, pSimpleTownTexture };
	script_map["Env_Foot_Bridge"] = ScriptContainer{ pEnv_Foot_Bridge, pSimpleTownTexture };
	script_map["Env_Car_Bridge_02"] = ScriptContainer{ pEnv_Car_Bridge_02, pSimpleTownTexture };
	script_map["Env_Car_Bridge"] = ScriptContainer{ pEnv_Car_Bridge, pSimpleTownTexture };
	script_map["Env_Canal_Straight"] = ScriptContainer{ pEnv_Canal_Straight, pSimpleTownTexture };
	script_map["Env_Canal_Pipe_02"] = ScriptContainer{ pEnv_Canal_Pipe_02, pSimpleTownTexture };
	script_map["Env_Canal_Pipe_01"] = ScriptContainer{ pEnv_Canal_Pipe_01, pSimpleTownTexture };
	script_map["Env_Canal_End"] = ScriptContainer{ pEnv_Canal_End, pSimpleTownTexture };
	script_map["Env_Canal_Corner_02"] = ScriptContainer{ pEnv_Canal_Corner_02, pSimpleTownTexture };
	script_map["Env_Canal_Corner_01"] = ScriptContainer{ pEnv_Canal_Corner_01, pSimpleTownTexture };
	script_map["Env_Beach_Straight"] = ScriptContainer{ pEnv_Beach_Straight, pSimpleTownTexture };
	script_map["Env_Beach_Short"] = ScriptContainer{ pEnv_Beach_Short, pSimpleTownTexture };
	script_map["Env_Beach_Corner"] = ScriptContainer{ pEnv_Beach_Corner, pSimpleTownTexture };
	script_map["Building_StripClub"] = ScriptContainer{ pStripClub_mesh, pSimpleTownTexture };
	script_map["Building_StoreCorner_Video"] = ScriptContainer{ pStore_Corner_mesh, pBuilding02a_Video_Texture };
	script_map["Building_StoreCorner_Pawn"] = ScriptContainer{ pStore_Corner_mesh, pBuilding02b_Pawn_Texture };
	script_map["Building_StoreCorner_Drug"] = ScriptContainer{ pStore_Corner_mesh, pBuilding02c_Drug_Texture };
	script_map["Building_Store_Video"] = ScriptContainer{ pStore_small_mesh, pBuilding02a_Video_Texture };
	script_map["Building_Store_Pawn"] = ScriptContainer{ pStore_small_mesh, pBuilding02b_Pawn_Texture };
	script_map["Building_Store_Drug"] = ScriptContainer{ pStore_small_mesh, pBuilding02c_Drug_Texture };
	script_map["Building_Shop_05"] = ScriptContainer{ pShop5_mesh, pSimpleTownTexture };
	script_map["Building_Shop_04"] = ScriptContainer{ pShop4_mesh, pSimpleTownTexture };
	script_map["Building_Shop_03"] = ScriptContainer{ pShop3_mesh, pSimpleTownTexture };
	script_map["Building_Shop_02"] = ScriptContainer{ pShop2_mesh, pSimpleTownTexture };
	script_map["Building_Shop_01"] = ScriptContainer{ pShop1_mesh, pSimpleTownTexture };
	script_map["Building_OfficeStepped_Grey"] = ScriptContainer{ pOffice_Stepped_mesh, pBuilding03b_Grey_Texture };
	script_map["Building_OfficeStepped_Brown"] = ScriptContainer{ pOffice_Stepped_mesh, pBuilding03c_Brown_Texture };
	script_map["Building_OfficeStepped_Blue"] = ScriptContainer{ pOffice_Stepped_mesh, pBuilding03a_Blue_Texture };
	script_map["Building_OfficeSmall_Grey"] = ScriptContainer{ pOffice_Small_mesh, pBuilding03b_Grey_Texture };
	script_map["Building_OfficeSmall_Brown"] = ScriptContainer{ pOffice_Small_mesh, pBuilding03c_Brown_Texture };
	script_map["Building_OfficeSmall_Blue"] = ScriptContainer{ pOffice_Small_mesh, pBuilding03a_Blue_Texture };
	script_map["Building_OfficeMedium_Grey"] = ScriptContainer{ pOffice_Medium_mesh, pBuilding03b_Grey_Texture };
	script_map["Building_OfficeMedium_Brown"] = ScriptContainer{ pOffice_Medium_mesh, pBuilding03c_Brown_Texture };
	script_map["Building_OfficeMedium_Blue"] = ScriptContainer{ pOffice_Medium_mesh, pBuilding03a_Blue_Texture };
	script_map["Building_OfficeLarge_Grey"] = ScriptContainer{ pOffice_Large_mesh, pBuilding03b_Grey_Texture };
	script_map["Building_OfficeLarge_Brown"] = ScriptContainer{ pOffice_Large_mesh, pBuilding03c_Brown_Texture };
	script_map["Building_OfficeLarge_Blue"] = ScriptContainer{ pOffice_Large_mesh, pBuilding03a_Blue_Texture };
	script_map["Building_Mall"] = ScriptContainer{ pMall_A_mesh, pSimpleTownTexture };
	script_map["Building_House_Red"] = ScriptContainer{ pHouse_mesh, pHouse_01_Red_Texture };
	script_map["Building_House_Orange"] = ScriptContainer{ pHouse_mesh, pHouse_03_Orange_Texture };
	script_map["Building_House_Green"] = ScriptContainer{ pHouse_mesh, pHouse_02_Green_Texture };
	script_map["Building_House_015"] = ScriptContainer{ pHouse15_mesh, pSimpleTownTexture };
	script_map["Building_House_014"] = ScriptContainer{ pHouse14_mesh, pSimpleTownTexture };
	script_map["Building_House_013"] = ScriptContainer{ pHouse13_mesh, pSimpleTownTexture };
	script_map["Building_House_012"] = ScriptContainer{ pHouse12_mesh, pSimpleTownTexture };
	script_map["Building_House_011"] = ScriptContainer{ pHouse11_mesh, pSimpleTownTexture };
	script_map["Building_House_010"] = ScriptContainer{ pHouse10_mesh, pSimpleTownTexture };
	script_map["Building_House_09"] = ScriptContainer{ pHouse9_mesh, pSimpleTownTexture };
	script_map["Building_House_08"] = ScriptContainer{ pHouse8_mesh, pSimpleTownTexture };
	script_map["Building_House_07"] = ScriptContainer{ pHouse7_mesh, pSimpleTownTexture };
	script_map["Building_House_06"] = ScriptContainer{ pHouse6_mesh, pSimpleTownTexture };
	script_map["Building_House_05"] = ScriptContainer{ pHouse5_mesh, pSimpleTownTexture };
	script_map["Building_House_04"] = ScriptContainer{ pHouse4_mesh, pSimpleTownTexture };
	script_map["Building_House_03"] = ScriptContainer{ pHouse3_mesh, pSimpleTownTexture };
	script_map["Building_House_02"] = ScriptContainer{ pHouse2_mesh, pSimpleTownTexture };
	script_map["Building_House_01"] = ScriptContainer{ pHouse1_mesh, pSimpleTownTexture };
	script_map["Building_Gym"] = ScriptContainer{ pGym_mesh, pSimpleTownTexture };
	script_map["Building_Garage_03"] = ScriptContainer{ pGarage03_mesh, pSimpleTownTexture };
	script_map["Building_Garage_02"] = ScriptContainer{ pGarage02_mesh, pSimpleTownTexture };
	script_map["Building_Garage_01"] = ScriptContainer{ pGarage01_mesh, pSimpleTownTexture };
	script_map["Building_CoffeeShop"] = ScriptContainer{ pCoffeeShop_mesh, pSimpleTownTexture };
	script_map["Building_Cinema"] = ScriptContainer{ pCinema_mesh, pSimpleTownTexture };
	script_map["Building_BaberShop"] = ScriptContainer{ pBarbershop_mesh, pSimpleTownTexture };
	script_map["Building_AutoRepair"] = ScriptContainer{ pAutoRepair_mesh, pSimpleTownTexture };
	script_map["Building_ApartmentSmall_Red"] = ScriptContainer{ pApartment_Small_mesh, pBuilding01b_Red_Texture };
	script_map["Building_ApartmentSmall_Orange"] = ScriptContainer{ pApartment_Small_mesh, pBuilding01a_Orange_Texture };
	script_map["Building_ApartmentSmall_Brown"] = ScriptContainer{ pApartment_Small_mesh, pBuilding01c_Brown_Texture };
	script_map["Building_ApartmentLarge_Red"] = ScriptContainer{ pApartment_Large_mesh, pBuilding01b_Red_Texture };
	script_map["Building_ApartmentLarge_Orange"] = ScriptContainer{ pApartment_Large_mesh, pBuilding01a_Orange_Texture };
	script_map["Building_ApartmentLarge_Brown"] = ScriptContainer{ pApartment_Large_mesh, pBuilding01c_Brown_Texture };
	script_map["Prop_Dumpster"] = ScriptContainer{ pDumpster_mesh, pProps_DumpsterGreenTexture };//일단 녹색만 추가한다
	script_map["billboard_mesh"] = ScriptContainer{ pbillboard_mesh, pProps_Billboard_02_Texture };
	//script load
	std::ifstream in("Scripts/cla.script");//스크립트 파일 이름

	CGameObject* pTempGameObject;
	std::string ignore;
	std::string objectname;
	int nGameObjects = 0;
	float pos_x = 0, pos_y = 0, pos_z = 0;
	float yaw = 0, pitch = 0, roll = 0;
	
	in >> ignore >> ignore;// Scriptname
	in >> ignore >> nGameObjects;
	std::vector<CGameObject*> lvpGameObjects;
	lvpGameObjects.reserve(nGameObjects * 4);
	int FileObjects_count = 0;
	int MeshObjects_count = 0;

	//nGameObjects *= 4;

	for (FileObjects_count;
	FileObjects_count < nGameObjects;)
	{
		in >> ignore >> objectname;//오브젝트 이름 로드
		in >> ignore >> MeshObjects_count;//메쉬 당 오브젝트 개수 로드

		for (int j = 0; j < MeshObjects_count; ++FileObjects_count, ++j)
		{
			in >> ignore >> pos_x >> pos_y >> pos_z;//Pos: x y z;
			in >> ignore >> pitch >> yaw >> roll; //Rotate: yaw pitch roll	
			in >> ignore >> ignore >> ignore >> ignore >> ignore;
			pos_y += 0.5;//terrain에 파묻히기 때문에, 좀 띄워본다.

			pTempGameObject = new CGameObject(1);
			pTempGameObject->SetPosition(pos_x, pos_y, pos_z);
			pTempGameObject->Rotate(yaw, pitch, roll);
			pTempGameObject->SetMesh(script_map[objectname].m_tempMesh, 0);
			
			CMaterialColors* MatColor = new CMaterialColors();
			CMaterial* TempMat = new CMaterial(MatColor);
			TempMat->SetTexture(script_map[objectname].m_tempTexture);
			pTempGameObject->SetMaterial(TempMat);
			//pTempGameObject->SetTexture(script_map[objectname].m_tempTexture);
			
			lvpGameObjects.push_back(pTempGameObject);
		}
	}
	in.close();

	m_nObjectShaders++;
	if (m_nObjectShaders == 1)
		m_ppObjectShaders = new CObjectsShader*[m_nObjectShaders];

	UINT meshtype2 = VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0;

	CObjectsShader *pModuleShader = new CObjectsShader();
	pModuleShader->CreateShader(pd3dDevice, meshtype2);
	pModuleShader->BuildObjects(pd3dDevice, lvpGameObjects);// 여기에 다 때려박는다.
	m_ppObjectShaders[0] = pModuleShader;

#endif

	auto end = chrono::high_resolution_clock::now() - start;

	//cout << chrono::duration_cast<chrono::milliseconds>(end).count() << endl;

	//ksh
	in.open("Scripts/fire.script");//스크립트 파일 이름

	int nParticleObjects = 0;

	in >> ignore >> ignore;// Scriptname
	in >> ignore >> nParticleObjects;

	int ParticleObjects_count = 0;

	in >> ignore >> ignore;//오브젝트 이름 로드
	in >> ignore >> ParticleObjects_count;//메쉬 당 오브젝트 개수 로드

	float p_x;
	float p_y;
	float p_z;

	vector<float> vec_p_x;
	vector<float> vec_p_y;
	vector<float> vec_p_z;

	for (int j = 0; j < ParticleObjects_count; ++j)
	{
		in >> ignore >> p_x >> p_y >> p_z;//Pos: x y z;
		vec_p_x.push_back(p_x);
		vec_p_y.push_back(p_y);
		vec_p_z.push_back(p_z);
		m_vFirePosition.push_back(make_pair(p_x, p_z));
	}

	in.close();
	//ksh

	// Particle System
	m_nParticles = ParticleObjects_count * 2 + 1;
	m_pParticles = new ParticleSystem[m_nParticles];

	ID3D11ShaderResourceView *pd3dsrvRandomTexture = CreateRandomTexture1DSRV(pd3dDevice);
	ID3D11ShaderResourceView *pd3dsrvParticleTexture = CreateTexture2DArray(pd3dDevice, L"Assets/Image/Effect/FP", 3);
	ID3D11ShaderResourceView *pd3dsrvSmokeTexture = CreateTexture2DArray(pd3dDevice, L"Assets/Image/Effect/SP", 1);
	ID3D11ShaderResourceView *pd3dsrvBloodTexture = CreateTexture2DArray(pd3dDevice, L"Assets/Image/Effect/BLD", 1);

	m_fire_space_parition.Initialize_space_division(2048, 5, 1, true);

	for (int i = 0; i < m_nParticles / 2; ++i) { // flare
		m_pParticles[i].Initialize(pd3dDevice, pd3dsrvParticleTexture, pd3dsrvRandomTexture, PARTICLES);

		int index = m_fire_space_parition.serch_space(vec_p_x[i], 0, vec_p_z[i]);
		m_fire_space_parition.SetSpace(index, D3DXVECTOR3(vec_p_x[i], vec_p_y[i], vec_p_z[i]), i);

		m_pParticles[i].CreateShaderVariables(pd3dDevice, D3DXVECTOR3(vec_p_x[i], vec_p_y[i], vec_p_z[i]), 
			D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
		m_pParticles[i].CreateShader(pd3dDevice, PARTICLE_TYPE_FLARE);
		m_pParticles[i].particleID = i;
	}
	for (int i = m_nParticles / 2; i < m_nParticles - 1; ++i) { // smoke
		m_pParticles[i].Initialize(pd3dDevice, pd3dsrvSmokeTexture, pd3dsrvRandomTexture, 200);
		m_pParticles[i].CreateShaderVariables(pd3dDevice, m_pParticles[i - m_nParticles / 2].EmitterPos,
			D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_REV_SUBTRACT);
		m_pParticles[i].CreateShader(pd3dDevice, PARTICLE_TYPE_SMOKE);
		m_pParticles[i].particleID = i;
	}
	// blood
	m_pParticles[ParticleObjects_count * 2].Initialize(pd3dDevice, pd3dsrvBloodTexture, pd3dsrvRandomTexture, 100);
	m_pParticles[ParticleObjects_count * 2].CreateShaderVariables(pd3dDevice, m_pPlayer->GetPosition(),
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
	m_pParticles[ParticleObjects_count * 2].CreateShader(pd3dDevice, PARTICLE_TYPE_BLOOD);
	m_pParticles[ParticleObjects_count * 2].particleID = ParticleObjects_count * 2;
	//

	CreateShaderVariables(pd3dDevice);
}

CHeightMapTerrain *CScene::GetTerrain()
{
    return((CHeightMapTerrain *)m_ppObjects[1]);
}

void CScene::ReleaseObjects()
{
    ReleaseShaderVariables();

    for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
    if (m_ppObjects) delete[] m_ppObjects;

    for (int j = 0; j < m_nObjectShaders; j++)
    {
        if (m_ppObjectShaders[j]) m_ppObjectShaders[j]->ReleaseObjects();
        if (m_ppObjectShaders[j]) m_ppObjectShaders[j]->Release();
    }
    if (m_ppObjectShaders) delete[] m_ppObjectShaders;

    for (int j = 0; j < m_nInstancingShaders; j++)
    {
        if (m_ppInstancingShaders[j]) m_ppInstancingShaders[j]->ReleaseObjects();
        if (m_ppInstancingShaders[j]) m_ppInstancingShaders[j]->Release();
    }
    if (m_ppInstancingShaders) delete[] m_ppInstancingShaders;
}

void CScene::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));
	m_pLights->m_d3dxcGlobalAmbient = D3DXCOLOR(0.001f, 0.001f, 0.001f, 1.0f);

	D3DXVECTOR3 dir = D3DXVECTOR3(0.3f, -1.0f, 0.3f);
	D3DXVec3Normalize(&dir, &dir);

	m_pLights->m_pDirectionalLights[0].m_d3dxcAmbient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights->m_pDirectionalLights[0].m_d3dxcDiffuse = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pDirectionalLights[0].m_d3dxcSpecular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pDirectionalLights[0].m_d3dxvDirection = dir;

	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		m_pLights->m_pPointLights[i].m_d3dxcAmbient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
		m_pLights->m_pPointLights[i].m_d3dxcDiffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
		m_pLights->m_pPointLights[i].m_d3dxcSpecular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
		m_pLights->m_pPointLights[i].m_d3dxvPosition = D3DXVECTOR3(m_vFirePosition[i].first, 2.0f, m_vFirePosition[i].second);
		m_pLights->m_pPointLights[i].m_fRange = 20.0f;
		m_pLights->m_pPointLights[i].m_d3dxvAttenuation = D3DXVECTOR3(1.0f, 0.001f, 0.00001f);
	}

	m_pLights->m_pSpotLights[0].m_d3dxcAmbient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pSpotLights[0].m_d3dxcDiffuse = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pSpotLights[0].m_d3dxcSpecular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pSpotLights[0].m_d3dxvPosition = D3DXVECTOR3(0.0f, 1000.0f, 0.0f);
	m_pLights->m_pSpotLights[0].m_fRange = 2000.0f;
	m_pLights->m_pSpotLights[0].m_d3dxvDirection = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	m_pLights->m_pSpotLights[0].m_fFalloff = 5.0f;
	m_pLights->m_pSpotLights[0].m_d3dxvAttenuation = D3DXVECTOR3(1.0f, 0.01f, 0.001f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof(LIGHTS);
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pLights;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights);
}

void CScene::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
	for (int i = 0; i < m_nParticles; ++i)
		m_pParticles[i].UpdateShaderVariables(pd3dDeviceContext);

    D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
    pd3dDeviceContext->Map(m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
    LIGHTS *pcbLight = (LIGHTS *)d3dMappedResource.pData;
    memcpy(pcbLight, pLights, sizeof(LIGHTS));
    pd3dDeviceContext->Unmap(m_pd3dcbLights, 0);
    pd3dDeviceContext->PSSetConstantBuffers(PS_CB_SLOT_LIGHT, 1, &m_pd3dcbLights);
	pd3dDeviceContext->GSSetConstantBuffers(GS_CB_SLOT_LIGHT, 1, &m_pd3dcbLights);
}

void CScene::ReleaseShaderVariables()
{
    if (m_pLights) delete m_pLights;
    if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
    return(false);
}

CGameObject *CScene::PickObjectPointedByCursor(int xClient, int yClient)
{
    if (!m_pCamera) return(NULL);

    D3DXMATRIX d3dxmtxView = m_pCamera->GetViewMatrix();
    D3DXMATRIX d3dxmtxProjection = m_pCamera->GetProjectionMatrix();
    D3D11_VIEWPORT d3dViewport = m_pCamera->GetViewport();

    D3DXVECTOR3 d3dxvPickPosition;
    d3dxvPickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / d3dxmtxProjection._11;
    d3dxvPickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / d3dxmtxProjection._22;
    d3dxvPickPosition.z = 1.0f;

    int nIntersected = 0;
    float fNearHitDistance = FLT_MAX;
    MESHINTERSECTINFO d3dxIntersectInfo;
    CGameObject *pIntersectedObject = NULL, *pNearestObject = NULL;
    for (int i = 0; i < m_nObjectShaders; i++)
    {
       /* pIntersectedObject = m_ppObjectShaders[i]->PickObjectByRayIntersection(&d3dxvPickPosition, &d3dxmtxView, &d3dxIntersectInfo);
        if (pIntersectedObject && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
        {
            fNearHitDistance = d3dxIntersectInfo.m_fDistance;
            pNearestObject = pIntersectedObject;
        }*/
    }
    for (int i = 1; i < m_nObjects; i++)
    {
        nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(&d3dxvPickPosition, &d3dxmtxView, &d3dxIntersectInfo);
        if ((nIntersected > 0) && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
        {
            fNearHitDistance = d3dxIntersectInfo.m_fDistance;
            pNearestObject = m_ppObjects[i];
        }
    }

    return(pNearestObject);
}

void CScene::AnimateBullets(float fTimeElapsed, CBulletList* pBulletList, ID3D11Device* pd3dDevice)
{
	CNode* pStart = pBulletList->getHead()->m_pNext;
	CNode* pEnd = pBulletList->getTail();

	for (int i = 0; i < 30; ++i)
		m_pBullets[i].SetActive(false);

	D3DXVECTOR3 pos;
	int key;

	CGameObject **pNPC = m_ppInstancingShaders[0]->GetNPC();
	int nNPC_Count = m_ppInstancingShaders[0]->GetNPC_Count();

success_remove:
	while (pStart != pEnd)
	{
		pos = pStart->m_d3dxvPos;
		key = pStart->getKey();

		AABB zombie = m_ppObjects[4]->GetMesh()->GetBoundingCube();
		zombie.Update(&m_ppObjects[4]->m_d3dxmtxWorld);

		if ([](AABB box, D3DXVECTOR3 point) -> bool {
			if (point.x < box.m_d3dxvMinimum.x) return false;
			if (point.y < box.m_d3dxvMinimum.y) return false;
			if (point.z < box.m_d3dxvMinimum.z) return false;

			if (point.x > box.m_d3dxvMaximum.x) return false;
			if (point.y > box.m_d3dxvMaximum.y) return false;
			if (point.z > box.m_d3dxvMaximum.z) return false;

			return true;
		}(zombie, pos))
		{
			bool WireFlag = m_ppObjects[4]->GetWireFlag();
			WireFlag ^= true;
			m_pBullets[key].SetActive(false);
			m_ppObjects[4]->SetWireFlag(WireFlag);

			CNode* pTemp = pStart;
			pStart = pStart->m_pNext;
			pBulletList->Remove(pTemp);
			continue;
		}
		else
		{
			m_pBullets[key].SetActive(true);
			m_pBullets[key].SetPosition(pStart->m_d3dxvPos);
		}

		AABB npc;
		for (int i = 0; i < nNPC_Count; ++i)
		{
			if (!pNPC[i]->GetActive()) continue;

			npc = m_ppInstancingShaders[0]->GetMesh()->GetBoundingCube();
			npc.Update(&(pNPC[i]->m_d3dxmtxWorld));
			
			if ([](AABB box, D3DXVECTOR3 point) -> bool {
				if (point.x + 3.0f > box.m_d3dxvMinimum.x &&
					point.y + 3.0f > box.m_d3dxvMinimum.y &&
					point.z + 3.0f > box.m_d3dxvMinimum.z &&
					point.x - 3.0f < box.m_d3dxvMaximum.x &&
					point.y - 3.0f < box.m_d3dxvMaximum.y &&
					point.z - 3.0f< box.m_d3dxvMaximum.z) return true;

				return false;
			}(npc, pos))
			{
				m_pBullets[key].SetActive(false);
				//pNPC[i]->SetActive(false);
				CNode* pTemp2 = pStart;
				pStart = pStart->m_pNext; pBulletList->Remove(pTemp2);

				g_bBlood = true;
				m_pParticles[m_nParticles - 1].SetPosition(pd3dDevice, pos);

				cs_packet_npc_state *npc_sate = reinterpret_cast<cs_packet_npc_state*>(m_csend_buf);
				npc_sate->id = i;
				npc_sate->size = sizeof(cs_packet_npc_state);
				m_wsa_send_buf.len = sizeof(cs_packet_npc_state);
				npc_sate->type = PLAYER_EVENT_ATTACK;
				npc_sate->state = false;

				DWORD iobyte;
				WSASend(m_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);

				goto success_remove;
			}
			else
			{
				m_pBullets[key].SetActive(true);
				m_pBullets[key].SetPosition(pStart->m_d3dxvPos);
			}
		}
		pStart = pStart->m_pNext;
	}
}


void CMinimapScene::BuildObjects(ID3D11Device *pd3dDevice)
{
    m_nMinimapObjects = 1 + 1 + 1 + 4000; // 1 Radar, 1 Radar grid, 1 Player, 4000 Enemy
    m_ppMinimapObjects = new CGameObject*[m_nMinimapObjects];

    // radar
    CMinimapRadarMesh *pRadarMesh = new CMinimapRadarMesh(pd3dDevice);
    CMinimapRadarPlaneMesh *pRadarPlaneMesh = new CMinimapRadarPlaneMesh(pd3dDevice);

    CMinimapRadarShader *pRadarShader = new CMinimapRadarShader();
    pRadarShader->CreateShader(pd3dDevice, pRadarMesh->GetType());

    ID3D11ShaderResourceView *pd3dsrvRadar = nullptr;
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/RadarUI_ver2.dds"), nullptr, nullptr, &pd3dsrvRadar, nullptr);

    CTexture *pRadarTexture = new CTexture(1, 0, PS_SLOT_TEXTURE, 0);
    pRadarTexture->SetTexture(0, pd3dsrvRadar);

    CMaterial *pRadarMaterial = new CMaterial();
    pRadarMaterial->SetTexture(pRadarTexture);

    CRadar *pRadar = new CRadar(1);
    pRadar->SetMesh(pRadarMesh, 0);
    pRadar->SetMesh(pRadarPlaneMesh, 1);
    pRadar->SetShader(pRadarShader);
    pRadar->SetMaterial(pRadarMaterial);
    pRadar->CreateDepthStencilState(pd3dDevice);
    pRadar->CreateBlendState(pd3dDevice);

    m_ppMinimapObjects[0] = pRadar;

    // radar grid
    CMinimapRadarPlaneMesh *pRadarGridMesh = new CMinimapRadarPlaneMesh(pd3dDevice);
    CMinimapRadarGridShader *pRadarGridShader = new CMinimapRadarGridShader();
    pRadarGridShader->CreateShader(pd3dDevice, pRadarGridMesh->GetType());

    ID3D11ShaderResourceView *pd3dsrvRadarGrid = nullptr;
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("Image/Etc/RadarGrid.dds"), nullptr, nullptr, &pd3dsrvRadarGrid, nullptr);

    CTexture *pRadarGridTexture = new CTexture(1, 0, PS_SLOT_TEXTURE, 0);
    pRadarGridTexture->SetTexture(0, pd3dsrvRadarGrid);

    CMaterial *pRadarGridMaterial = new CMaterial();
    pRadarGridMaterial->SetTexture(pRadarGridTexture);

    CGameObject *pRadarGrid = new CGameObject(1);
    pRadarGrid->SetMesh(pRadarGridMesh, 0);
    pRadarGrid->SetShader(pRadarGridShader);
    pRadarGrid->SetMaterial(pRadarGridMaterial);

    m_ppMinimapObjects[1] = pRadarGrid;

    // player
    CMinimapPlayerMesh *pPlayerMesh = new CMinimapPlayerMesh(pd3dDevice); // ok

    CMinimapPlayerShader *pPlayerShader = new CMinimapPlayerShader();
    pPlayerShader->CreateShader(pd3dDevice, pPlayerMesh->GetType()); // ok

    CGameObject *pPlayer = new CGameObject(1);
    pPlayer->SetMesh(pPlayerMesh);
    pPlayer->SetShader(pPlayerShader);

    m_ppMinimapObjects[2] = pPlayer;

    // enemy
    CMinimapEnemyMesh *pEnemyMesh = new CMinimapEnemyMesh(pd3dDevice);

    CShader *pEnemyShader = new CShader();
    pEnemyShader->CreateShader(pd3dDevice, pEnemyMesh->GetType());

    CGameObject *pEnemy;
    for (int i = 3; i < m_nMinimapObjects; ++i)
    {
        pEnemy = new CGameObject(1);
        pEnemy->SetMesh(pEnemyMesh);
        pEnemy->SetShader(pEnemyShader);

        m_ppMinimapObjects[i] = pEnemy;
    }
}

void CMinimapScene::ReleaseObjects()
{
    for (int i = 0; i < m_nMinimapObjects; ++i)
        if (m_ppMinimapObjects[i]) m_ppMinimapObjects[i]->Release();
    if (m_ppMinimapObjects) delete[] m_ppMinimapObjects;
}

void CMinimapScene::AnimateObjects(float fTimeElapsed, CPlayer *pPlayer, 
	CInstancedObjectsShader **pNPCOwner, CConnectServer *server_info)
{
    static float radar_rotation = 0;
    D3DXMATRIX mtxRadarRotate;
    D3DXMATRIX mtxRotate;

    D3DXMatrixRotationAxis(&mtxRadarRotate, &D3DXVECTOR3(0.0f, 0.0f, -1.0f), float(D3DXToRadian(radar_rotation)));

    radar_rotation += 60.0f * fTimeElapsed;

    m_ppMinimapObjects[0]->m_d3dxmtxLocal = mtxRadarRotate;

    D3DXMatrixRotationAxis(&mtxRotate, &D3DXVECTOR3(0.0f, 0.0f, -1.0f), float(D3DXToRadian(pPlayer->GetYaw())));
    m_ppMinimapObjects[2]->m_d3dxmtxLocal = mtxRotate;

    for (int i = 3; i < m_nMinimapObjects; ++i)
    {
		if (((pNPCOwner[0])->GetNPC()[i - 3])->GetActive()) {
			//if(server_info->GetEnemyNPCView(i-3)){
			D3DXVECTOR3 d3dvPosition = D3DXVECTOR3(server_info->GetEnemyNPCPos(i-3).first.first, 1.3f, server_info->GetEnemyNPCPos(i-3).second);
			//m_ppMinimapObjects[i]->SetPosition(d3dvPosition);
			D3DXVECTOR3 d3dxvPos = (pNPCOwner[0])->GetNPC()[i - 3]->GetPosition();
			m_ppMinimapObjects[i]->SetPosition(((pNPCOwner[0])->GetNPC()[i - 3])->GetPosition());
			m_ppMinimapObjects[i]->SetActive(true);
		}
        else
            m_ppMinimapObjects[i]->SetActive(false);
    }
}

void CMinimapScene::Render(ID3D11DeviceContext	*pd3dDeviceContext, CCamera *pCamera)
{
    D3DXMATRIX mtxIdentity;
    D3DXMatrixIdentity(&mtxIdentity);

    // radar stencil circle
    pd3dDeviceContext->OMSetDepthStencilState(static_cast<CRadar *>(m_ppMinimapObjects[0])->m_pd3d_DSS_Radar_Step_1, 1);
    m_ppMinimapObjects[0]->Update(nullptr);
    m_ppMinimapObjects[0]->UpdateShaderVariable(pd3dDeviceContext, &mtxIdentity);
    m_ppMinimapObjects[0]->GetShader()->Render(pd3dDeviceContext, pCamera);
    m_ppMinimapObjects[0]->GetMesh(0)->Render(pd3dDeviceContext);
    pd3dDeviceContext->OMSetDepthStencilState(static_cast<CRadar *>(m_ppMinimapObjects[0])->m_pd3d_DSS_Radar_Step_2, 1);

    // radar original plane
    m_ppMinimapObjects[0]->GetMaterial()->UpdateShaderVariable(pd3dDeviceContext);
    m_ppMinimapObjects[0]->GetMesh(1)->Render(pd3dDeviceContext);
    pd3dDeviceContext->OMSetDepthStencilState(static_cast<CRadar *>(m_ppMinimapObjects[0])->m_pd3d_DSS_Radar_Step_3, 1);

    // enemy
	m_ppMinimapObjects[3]->GetShader()->Render(pd3dDeviceContext, pCamera);
    for (int i = 3; i < m_nMinimapObjects; ++i)
    {
		if (!m_ppMinimapObjects[i]->GetActive()) continue;
        m_ppMinimapObjects[i]->Update(nullptr);
        m_ppMinimapObjects[i]->UpdateShaderVariable(pd3dDeviceContext, &m_ppMinimapObjects[i]->m_d3dxmtxWorld);
        m_ppMinimapObjects[i]->RenderMesh(pd3dDeviceContext, pCamera);
    }

    // radar alpha plane
    pd3dDeviceContext->OMSetBlendState(m_ppMinimapObjects[0]->m_pd3dBlendState, nullptr, 0xffffffff);
    m_ppMinimapObjects[0]->GetMaterial()->UpdateShaderVariable(pd3dDeviceContext);
    m_ppMinimapObjects[0]->UpdateShaderVariable(pd3dDeviceContext, &m_ppMinimapObjects[0]->m_d3dxmtxWorld);
    m_ppMinimapObjects[0]->GetShader()->Render(pd3dDeviceContext, pCamera);
    m_ppMinimapObjects[0]->GetMesh(1)->Render(pd3dDeviceContext);
    pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

    // radar grid
    m_ppMinimapObjects[1]->GetMaterial()->UpdateShaderVariable(pd3dDeviceContext);
    m_ppMinimapObjects[1]->UpdateShaderVariable(pd3dDeviceContext, &mtxIdentity);
    m_ppMinimapObjects[1]->GetShader()->Render(pd3dDeviceContext, pCamera);
    m_ppMinimapObjects[1]->GetMesh(0)->Render(pd3dDeviceContext);

    // player
    m_ppMinimapObjects[2]->Update(nullptr);
    m_ppMinimapObjects[2]->UpdateShaderVariable(pd3dDeviceContext, &m_ppMinimapObjects[2]->m_d3dxmtxWorld);
    m_ppMinimapObjects[2]->GetShader()->Render(pd3dDeviceContext, pCamera);
    m_ppMinimapObjects[2]->RenderMesh(pd3dDeviceContext, pCamera);
}


void CScene::AnimateObjects(float fTimeElapsed)
{
    D3DXVECTOR3 d3dxvCameraPosition = m_pCamera->GetPosition();
    if (m_pLights && m_pd3dcbLights)
    {
        m_pLights->m_d3dxvCameraPosition = D3DXVECTOR4(d3dxvCameraPosition, 1.0f);
		//cout << m_pLights->m_d3dxvCameraPosition.x << endl;
		int index = m_fire_space_parition.serch_space(m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, m_pPlayer->GetPosition().z);
		int size = m_fire_space_parition.GetObjectPositionSpace(index).size();
		vector <pair<int, D3DXVECTOR3>> vec = m_fire_space_parition.GetObjectPositionSpace(index);
		auto begin = vec.begin();
		auto end = vec.end();
		int j = 0;
		for (; begin !=end; begin++, j++) 
		{
			int k = m_fire_space_parition.GetObjectPositionSpace(index)[j].first;
			int key = vec[j].first;
			if (j < MAX_POINT_LIGHTS) {
			
				m_pLights->m_pPointLights[j].m_d3dxvPosition = m_pParticles[(*begin).first].EmitterPos;
			}
		}

		// directional light
		m_pLights->m_pDirectionalLights[0].m_d3dxvDirection = m_d3dLightPosition;
    }

    for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fTimeElapsed, NULL);
    for (int i = 0; i < m_nObjectShaders; i++) m_ppObjectShaders[i]->AnimateObjects(fTimeElapsed);

	for (int i = 0; i < m_nInstancingShaders; i++)
	{
		m_ppInstancingShaders[i]->AnimateObjects(fTimeElapsed);
		if(m_ppInstancingShaders[i]->GetMesh()->FBXFrameAdvance(fTimeElapsed))//좀비 애니메이션 프레임 Set
			m_ppInstancingShaders[i]->GetMesh()->ZombieWalk();
	}

	// particle
	for (int i = 0; i < m_nParticles; i++)
		m_pParticles[i].Update(fTimeElapsed, float(m_nParticles));
}

void CScene::OnPreRender(ID3D11DeviceContext *pd3dDeviceContext)
{

}

void CScene::Render(ID3D11DeviceContext	*pd3dDeviceContext, CCamera *pCamera, D3DXVECTOR3 playerPos, bool ShadowMap)
{
	if (!ShadowMap) if (m_pLights && m_pd3dcbLights) UpdateShaderVariable(pd3dDeviceContext, m_pLights);

	if (!ShadowMap) if (m_ppObjects && m_ppObjects[0]) m_ppObjects[0]->Render(pd3dDeviceContext, pCamera); //SkyBox

	for (int i = 2; i < m_nObjects; i++)
	{
		if (2 == i) continue; // Zoom
		if (3 == i) continue; // Aim
		if (5 == i) continue; // UI
		if (m_ppObjects[i]->IsVisible(pCamera)) m_ppObjects[i]->Render(pd3dDeviceContext, pCamera);
	}
	for (int i = 0; i < m_nObjectShaders; i++) m_ppObjectShaders[i]->Render(m_pPlayer->GetPosition(), pd3dDeviceContext, pCamera);

	CMesh* NPCMesh = m_ppInstancingShaders[0]->GetMesh();
	NPCMesh->UpdateBoneTransform(pd3dDeviceContext, NPCMesh->GetFBXAnimationNum(), NPCMesh->GetFBXNowFrameNum());
	for (int i = 0; i < m_nInstancingShaders; i++) m_ppInstancingShaders[i]->Render(pd3dDeviceContext, pCamera);

	// OtherPlayer
	for (int i = 0; i < m_nOtherPlayer; ++i)
	{
		if (m_ppOtherPlayer[i]->IsVisible(pCamera))
		{
			m_ppOtherPlayer[i]->SetActive(true);
			CMesh* TempMesh = m_ppOtherPlayer[i]->GetMesh();
			TempMesh->UpdateBoneTransform(pd3dDeviceContext, TempMesh->GetFBXAnimationNum(), TempMesh->GetFBXNowFrameNum());
			m_ppOtherPlayer[i]->Render(pd3dDeviceContext, pCamera);
		}
	}
	for (auto i = 0; i < 3; ++i) {
		m_ppSteerShader[i]->Render(pd3dDeviceContext, pCamera);
	}

	if (!ShadowMap) {
		for (int i = 0; i < 30; ++i)
		{
			if (m_pBullets[i].IsVisible())
				m_pBullets[i].Render(pd3dDeviceContext, pCamera);
		}

		if (g_zoom && m_ppObjects[2])
			m_ppObjects[2]->Render(pd3dDeviceContext, pCamera); //Zoom
		if (!g_zoom && m_ppObjects[3]) m_ppObjects[3]->Render(pd3dDeviceContext, pCamera); //Aim
		if (m_ppObjects[5]) m_ppObjects[5]->Render(pd3dDeviceContext, pCamera); //UI

																				// particle
		if (g_bBlood) {
			m_pParticles[m_nParticles - 1].Reset(pd3dDeviceContext);
			g_bBlood = false;
		}
		int index = m_fire_space_parition.serch_space(playerPos.x, playerPos.y, playerPos.z);
		vector<int> visible_vector = m_fire_space_parition.serch_visible_section(playerPos.x, playerPos.y, playerPos.z);
		auto size = m_fire_space_parition.GetObjectPositionSpace(index).size();
		int visible_vec_size = visible_vector.size();
		for (auto i = 0; i < visible_vec_size; ++i) {
			vector<pair<int, D3DXVECTOR3>> v = m_fire_space_parition.GetObjectPositionSpace(visible_vector[i]);
			auto begin = v.begin();
			auto end = v.end();
			while (begin != end) {
				if (D3DXVec3Length(&(m_pParticles[(*begin).first].EmitterPos - playerPos)) < particleCreateRange)
					m_pParticles[(*begin).first].Render(pd3dDeviceContext);
				begin++;
			}
		}

		m_pParticles[m_nParticles - 1].Render(pd3dDeviceContext);
		// particle
		for (int i = m_nParticles / 2; i < m_nParticles - 1; i++) {
			if (D3DXVec3Length(&(m_pParticles[i].EmitterPos - playerPos)) < particleCreateRange)
				m_pParticles[i].Render(pd3dDeviceContext);
		}
	}
}

void CScene::RenderLobby(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	for (int i = 0; i < m_nObjectShaders; i++) m_ppObjectShaders[i]->Render(m_pPlayer->GetPosition(), pd3dDeviceContext, pCamera);
}
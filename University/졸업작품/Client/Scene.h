//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Camera.h"
#include "Shader.h"
#include "SpacePartition.h"
#include "ConnectServer.h"
#include "ParticleSystem.h";

//
// Lighting
//
#define MAX_DIRECTIONAL_LIGHTS		1
#define MAX_POINT_LIGHTS			30
#define MAX_SPOT_LIGHTS				1

struct DIRECTIONAL_LIGHT
{
	D3DXCOLOR m_d3dxcAmbient;
	D3DXCOLOR m_d3dxcDiffuse;
	D3DXCOLOR m_d3dxcSpecular;
	D3DXVECTOR3 m_d3dxvDirection;
	float padding;
};

struct POINT_LIGHT
{
	D3DXCOLOR m_d3dxcAmbient;
	D3DXCOLOR m_d3dxcDiffuse;
	D3DXCOLOR m_d3dxcSpecular;
	D3DXVECTOR3 m_d3dxvPosition;
	float m_fRange;
	D3DXVECTOR3 m_d3dxvAttenuation;
	float padding;
};

struct SPOT_LIGHT
{
	D3DXCOLOR m_d3dxcAmbient;
	D3DXCOLOR m_d3dxcDiffuse;
	D3DXCOLOR m_d3dxcSpecular;
	D3DXVECTOR3 m_d3dxvPosition;
	float m_fRange;
	D3DXVECTOR3 m_d3dxvDirection;
	float m_fFalloff;
	D3DXVECTOR3 m_d3dxvAttenuation;
	float padding;
};
//

struct LIGHTS
{
	DIRECTIONAL_LIGHT	m_pDirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	POINT_LIGHT			m_pPointLights[MAX_POINT_LIGHTS];
	SPOT_LIGHT			m_pSpotLights[MAX_SPOT_LIGHTS];
	D3DXCOLOR			m_d3dxcGlobalAmbient;
	D3DXVECTOR4			m_d3dxvCameraPosition;
};
struct ScriptContainer
{
	CMesh* m_tempMesh;
	CTexture* m_tempTexture;
};

class CScene
{
public:
	CScene();
	virtual ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void ReleaseObjects();

	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	void ReleaseShaderVariables();

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateBullets(float fTimeElapsed, CBulletList* pBulletList, ID3D11Device* pd3dDevice);
	void AnimateObjects(float fTimeElapsed);
	void OnPreRender(ID3D11DeviceContext *pd3dDeviceContext);
	virtual void Render(ID3D11DeviceContext	*pd3dDeviceContext, CCamera *pCamera, D3DXVECTOR3 playerPos, bool ShadowMap);

	void RenderLobby(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);

	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);

	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	void SetSocket(SOCKET socket) { m_socket = socket; }

	CHeightMapTerrain *GetTerrain();
	void OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, CMaterial *pMaterial, int nIndex = 0);

	CInstancedObjectsShader* GetEnemyObject(int index) { return m_ppInstancingShaders[index]; }
	CSteerGSInstancedObjectShader* GetSteerObject(int index) { return m_ppSteerShader[index]; }

	CGameObject* GetOtherPlayer(int index) { return m_ppOtherPlayer[index]; }


    CInstancedObjectsShader **GetNPCOwner() { return m_ppInstancingShaders; }

	D3DXVECTOR3 m_d3dLightPosition;
	void SetLightPosition(D3DXVECTOR3 d3dposition) { m_d3dLightPosition = d3dposition; }

private:
	CGameObject						**m_ppObjects;
	int								m_nObjects;

	CObjectsShader					**m_ppObjectShaders;
	int								m_nObjectShaders;

	CInstancedObjectsShader			**m_ppInstancingShaders;
	int								m_nInstancingShaders;

	CSteerGSInstancedObjectShader   **m_ppSteerShader;

	CPlayer							*m_pPlayer;
	CCamera							*m_pCamera;
	CGameObject						*m_pSelectedObject;

	LIGHTS							*m_pLights;
	ID3D11Buffer					*m_pd3dcbLights;

    CGameObject                     m_pBullets[30];

	////////////////////////////////////////////////
	CGameObject                     **m_ppOtherPlayer;
	int                             m_nOtherPlayer;

	////////////////////////////////////////////////


	////////////////////////////////////////////////

	WSABUF							m_wsa_send_buf;
	char							m_csend_buf[BUFF_SIZE];
	SOCKET							m_socket;

	CSpacePartition					m_fire_space_parition;
	vector<pair<float, float>>      m_vFirePosition;
	//
	// Particle System
	//
	int				m_nParticles;
	ParticleSystem*	m_pParticles;
};

class CLobbyScene : public CScene
{
public:
	static int m_loading_stat;
	bool m_bIsInLobby;
	bool m_bIsInLoading;
	bool m_bIsInGame;

	CLobbyScene() {}
	~CLobbyScene() {}

	virtual void BuildObjects(ID3D11Device *pd3dDevice) {}
	virtual void ReleaseObjects() {}
};

class CMinimapScene : public CScene
{
    CGameObject   **m_ppMinimapObjects;
    int           m_nMinimapObjects;

public:
    CMinimapScene()
    {
        m_ppMinimapObjects = nullptr;
        m_nMinimapObjects = 0;
    }
    ~CMinimapScene() {}

    virtual void BuildObjects(ID3D11Device *pd3dDevice);
    virtual void ReleaseObjects();

    virtual void AnimateObjects(float fTimeElapsed, CPlayer *pPlayer, CInstancedObjectsShader **pNPCOwner, CConnectServer *server_info);
    virtual void Render(ID3D11DeviceContext	*pd3dDeviceContext, CCamera *pCamera);
};
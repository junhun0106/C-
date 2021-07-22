#pragma once

#include "Object.h"
#include "Camera.h"

class CNode
{
public:
    const float BULLET_SPEED = 500.0f;
    float m_fLifeTime = 3.0f;
    D3DXVECTOR3 m_d3dxvPos;
    D3DXVECTOR3 m_d3dxvDir;
    CNode *m_pNext;

    CNode() : m_pNext(nullptr), m_nKey(-1)
    {
        m_d3dxvPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_d3dxvDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
    CNode(D3DXVECTOR3 d3dxvPos, D3DXVECTOR3 d3dxvDir, CNode *pNext, int nKey) : m_pNext(pNext), m_nKey(nKey)
    {
        m_d3dxvPos = d3dxvPos;
        m_d3dxvDir = d3dxvDir;
    }
    ~CNode() {}

    int getKey() { return m_nKey; }

private:
    int m_nKey;
};

class CBulletList
{    
public:
    CBulletList() { mHead.m_pNext = &mTail; }
    ~CBulletList() {}

    void Init();
    void Add(D3DXVECTOR3 d3dxvPos, D3DXVECTOR3 d3dxvDir);
    void Remove(CNode *pDest);
    void Update(float deltaTime);
    CNode* getHead() { return &mHead; }
    CNode* getTail() { return &mTail; }

	int GetBulletCount() { return m_nBulletCount; }

    //int m_nBulletCount = 0;
private:
    const int MAX_BULLET = 30;
    int m_nBulletCount = 0;
    CNode mHead, mTail;
};


class CPlayer : public CGameObject
{
protected:
	D3DXVECTOR3					m_d3dxvPosition;
	D3DXVECTOR3					m_d3dxvRight;
	D3DXVECTOR3					m_d3dxvUp;
	D3DXVECTOR3					m_d3dxvLook;

	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;

	D3DXVECTOR3					m_d3dxvVelocity;
	D3DXVECTOR3     			m_d3dxvGravity;
	float           			m_fMaxVelocityXZ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

	LPVOID						m_pPlayerUpdatedContext;
	LPVOID						m_pCameraUpdatedContext;

	CCamera						*m_pCamera;

    CBulletList                 m_BulletList;
	unsigned int				m_playerState;

public:
	enum { IDLE = 0, MOVE = 1, SHOOT = 2, RELOAD = 3 };//Player State Á¤ÀÇ

	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();

	D3DXVECTOR3 GetPosition() { return(m_d3dxvPosition); }
	D3DXVECTOR3 GetLookVector() { return(m_d3dxvLook); }
	D3DXVECTOR3 GetUpVector() { return(m_d3dxvUp); }
	D3DXVECTOR3 GetRightVector() { return(m_d3dxvRight); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const D3DXVECTOR3& d3dxvGravity) { m_d3dxvGravity = d3dxvGravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const D3DXVECTOR3& d3dxvVelocity) { m_d3dxvVelocity = d3dxvVelocity; }
	void SetPosition(const D3DXVECTOR3& d3dxvPosition) { Move((d3dxvPosition - m_d3dxvPosition), false); }

	const D3DXVECTOR3& GetVelocity() const { return(m_d3dxvVelocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const D3DXVECTOR3& d3dxvShift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void Update(float fTimeElapsed);

	virtual void OnPlayerUpdated(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdated(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);

	CCamera *OnChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual void ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Animate(float fTimeElapsed, D3DXMATRIX *pd3dxmtxParent);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);

    CBulletList* getBulletList() { return &m_BulletList; }
	virtual unsigned int GetPlayerState() { return m_playerState; }
	virtual void SetPlayerState(unsigned int current_state) { m_playerState = current_state; }
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer(int nMeshes = 1);
	virtual ~CTerrainPlayer();

	virtual void ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdated(float fTimeElapsed);
	virtual void OnCameraUpdated(float fTimeElapsed);
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer(int nMeshes = 1);
	virtual ~CAirplanePlayer();

	virtual void OnPrepareRender();

	virtual void OnPlayerUpdated(float fTimeElapsed);
	virtual void ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed);
};

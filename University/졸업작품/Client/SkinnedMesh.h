#pragma once
#include "Mesh.h"

struct VS_CB_SKINNED
{
	D3DXMATRIX	m_d3dxmtxBone[30];
};

struct BoneAnimationData
{
	int m_nFrameCount;//필요한가? 필요하다
	float	*m_pfAniTime;//불필요하다. 허나 아직 모름
	D3DXVECTOR3 *m_pd3dxvScale;
	D3DXVECTOR3 *m_pd3dxvTranslate;
	D3DXVECTOR4 *m_pd3dxvQuaternion;
};

class CSkinnedMesh : public CMeshTexturedIlluminated
{
private:
	static BoneAnimationData **s_ppBoneAnimationData;//이건 static으로 공유할 수 있는 데이터
	static int s_nBoneCount;//18
	static int s_nAnimationClip;//2
	static D3DXMATRIX *s_pd3dxmtxBoneOffsets;

	D3DXVECTOR3 *m_pd3dxvPositions;//필요
	D3DXVECTOR3	*m_pd3dxvNormals;//필요
	D3DXVECTOR2 *m_pd3dxvTexCoords;//필요
	D3DXVECTOR4 *m_pd3dxvBoneWeights;//개별적 필요한 데이터
	D3DXVECTOR4 *m_pd3dxvBoneIndices;//개별적 필요한 데이터

	D3DXMATRIX *m_pd3dxmtxSQTTransform;//이것도 필요하다.
	D3DXMATRIX *m_pd3dxmtxFinalBone;//최종행렬 배열(이건 필요하다.)

	D3DXMATRIX m_d3dxmtxLocalTransform;//개별적으로 필요한 로컬 변환 행렬

									   // i번 뼈대의 부모 색인(parentIndex)를 담는다.
									   // i번 뼈대는 애니메이션 클립의 i번째 BoneAnimation 인스턴스에 대응.
	UINT *m_pBoneHierarchy;


	ID3D11Buffer *m_pd3dWeightBuffer;//개별적으로 필요한 가중치 버퍼
	ID3D11Buffer *m_pd3dBoneIndiceBuffer;//개별적으로 필요한 본 인덱스 버퍼

										 // 뼈대 상수버퍼
	ID3D11Buffer *m_pd3dcbBones;//필요하다

	float m_fFBXModelSize;		// 모델의 사이즈 (보통 Animate에서 조절해주기 위함)
	float m_fFBXAnimationTime;	// 모델의 AnimationTime (공통적으로 0.0333333f 씩 증가)	
	int m_nFBXAnimationNum;		// 모델이 실행할 애니메이션의 Set넘버 0 혹은 1이다.

	int m_nFBXStartFrameNum;	// 모델의 애니메이션이 시작하는 프레임 넘버
	int m_nFBXMaxFrameNum;		// 모델이 실행할 애니메이션의 최대 프레임 수.

	int m_nFBXNowFrameNum;		// 모델이 진행중인 애니메이션의 현재 프레임 값.

	unsigned int m_uiAnimationState;
public:

	CSkinnedMesh(ID3D11Device *pd3dDevice, char *pszFileName, float fSize);
	virtual ~CSkinnedMesh();

	// 해당 프레임의 SR(Q)T 회전이 반영된 행렬을 반환
	void MakeBoneMatrix(int nNowframe, int nAnimationNum, int nBoneNum, D3DXMATRIX& BoneMatrix);
	// 상수 버퍼로 전달할 최종 본 행렬을 구한다.
	void UpdateBoneTransform(ID3D11DeviceContext *pd3dDeviceContext, int nAnimationNum, int nNowFrame);
	// 뼈대 상수 버퍼 설정
	void CreateConstantBuffer(ID3D11Device *pd3dDevice);

	float GetFBXModelSize() { return m_fFBXModelSize; }
	float GetFBXAnimationTime() { return m_fFBXAnimationTime; }
	int GetFBXAnimationNum() { return m_nFBXAnimationNum; }
	int GetFBXNowFrameNum() { return m_nFBXNowFrameNum; }
	int GetFBXMaxFrameNum() { return m_nFBXMaxFrameNum; }
	//void SetFBXAnimationTime(float fFBXAnimationTime) { m_fFBXAnimationTime = fFBXAnimationTime; }
	bool FBXFrameAdvance(float fTimeElapsed);
	void SetAnimation(int nFBXAnimationNum);

	void WalkAnimationSet();
	void RunAnimationSet();
	void ShootRifle();
	void ShootPistol();
	void IdleSet();
	void HelloSet();
	void ShootSniper();
	void ReloadRifle();
	void ZombieWalk();
	unsigned int GetAnimationState() { return m_uiAnimationState; }
	void SetAnimationState(unsigned int CurrentState)
	{
		m_uiAnimationState = CurrentState;
	}
	void GetHandMatrix(D3DXMATRIX& mtxTransform) { mtxTransform = m_pd3dxmtxFinalBone[14] * m_d3dxmtxLocalTransform; }

	static void LoadAnimationSet();
	static void AnimationDestroy();
};


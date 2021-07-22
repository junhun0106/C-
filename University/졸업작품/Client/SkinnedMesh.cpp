#include "stdafx.h"
#include "SkinnedMesh.h"

int CSkinnedMesh::s_nAnimationClip = 2;
int CSkinnedMesh::s_nBoneCount = 18;
BoneAnimationData** CSkinnedMesh::s_ppBoneAnimationData = NULL;

D3DXMATRIX* CSkinnedMesh::s_pd3dxmtxBoneOffsets = NULL;

CSkinnedMesh::CSkinnedMesh(ID3D11Device *pd3dDevice, char *pszFileName, float fSize) : CMeshTexturedIlluminated(pd3dDevice)
{
	m_fFBXModelSize = fSize;
	m_fFBXAnimationTime = 0.0f;
	m_nFBXAnimationNum = 0;

	ifstream fin(pszFileName);

	D3DXMatrixIdentity(&m_d3dxmtxLocalTransform);
	D3DXMATRIX mtxScale;
	D3DXMATRIX mtxRotate;
	D3DXMatrixScaling(&mtxScale, m_fFBXModelSize, m_fFBXModelSize, m_fFBXModelSize);
	D3DXMatrixRotationYawPitchRoll(&mtxRotate, (float)D3DXToRadian(-90.0f), (float)D3DXToRadian(-90.0f), (float)D3DXToRadian(-90.0f));
	m_d3dxmtxLocalTransform = mtxScale * mtxRotate;

	string ignore;

	if (!fin.fail())
	{
		// �����͸� �о�� �ʿ��� ����, �ε���, ��, �ִϸ��̼� �� �ľ�
		fin >> ignore;//[FBX_META_DATA]
		fin >> ignore >> ignore;
		fin >> ignore;
		fin >> ignore >> m_nVertices;
		fin >> ignore >> m_nIndices;
		fin >> ignore >> ignore;
		fin >> ignore >> ignore;

		// ���� �����͸� ����
		m_pd3dxvPositions = new D3DXVECTOR3[m_nVertices];
		m_pd3dxvNormals = new D3DXVECTOR3[m_nVertices];
		m_pd3dxvTexCoords = new D3DXVECTOR2[m_nVertices];

		if (s_nBoneCount)
		{
			m_pd3dxvBoneIndices = new D3DXVECTOR4[m_nVertices];
			m_pd3dxvBoneWeights = new D3DXVECTOR4[m_nVertices];
		}

		fin >> ignore;

		for (int i = 0; i < m_nVertices; i++)
		{
			fin >> ignore >> m_pd3dxvPositions[i].x >> m_pd3dxvPositions[i].y >> m_pd3dxvPositions[i].z;
			fin >> ignore >> m_pd3dxvNormals[i].x >> m_pd3dxvNormals[i].y >> m_pd3dxvNormals[i].z;
			fin >> ignore >> m_pd3dxvTexCoords[i].x >> m_pd3dxvTexCoords[i].y;
			if (s_nBoneCount)
			{
				fin >> ignore >> m_pd3dxvBoneIndices[i].x >> m_pd3dxvBoneIndices[i].y >> m_pd3dxvBoneIndices[i].z >> m_pd3dxvBoneIndices[i].w;
				fin >> ignore >> m_pd3dxvBoneWeights[i].x >> m_pd3dxvBoneWeights[i].y >> m_pd3dxvBoneWeights[i].z >> m_pd3dxvBoneWeights[i].w;
			}
		}
		m_pnIndices = new UINT[m_nIndices];
		fin >> ignore;//[INDEX_DATA]
		for (int i = 0; i < m_nIndices; ++i)
			fin >> m_pnIndices[i];

		// (�ִϸ��̼��� ������ �޽��� ���) �� ������ �ִϸ��̼� ���� ����
		if (s_nBoneCount)
		{
			m_pd3dxmtxSQTTransform = new D3DXMATRIX[s_nBoneCount];
			m_pd3dxmtxFinalBone = new D3DXMATRIX[s_nBoneCount];
		}
	}
	fin.close();

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pd3dPositionBuffer = CreateBuffer(pd3dDevice, sizeof(D3DXVECTOR3), m_nVertices, m_pd3dxvPositions, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0);
	m_pd3dNormalBuffer = CreateBuffer(pd3dDevice, sizeof(D3DXVECTOR3), m_nVertices, m_pd3dxvNormals, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0);
	m_pd3dTexCoordBuffer = CreateBuffer(pd3dDevice, sizeof(D3DXVECTOR2), m_nVertices, m_pd3dxvTexCoords, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0);
	m_pd3dWeightBuffer = CreateBuffer(pd3dDevice, sizeof(D3DXVECTOR4), m_nVertices, m_pd3dxvBoneWeights, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0);
	m_pd3dBoneIndiceBuffer = CreateBuffer(pd3dDevice, sizeof(D3DXVECTOR4), m_nVertices, m_pd3dxvBoneIndices, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0);
	ID3D11Buffer *pd3dBuffers[5] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dBoneIndiceBuffer, m_pd3dWeightBuffer};
	UINT pnBufferStrides[5] = { sizeof(D3DXVECTOR3), sizeof(D3DXVECTOR3), sizeof(D3DXVECTOR2), sizeof(D3DXVECTOR4), sizeof(D3DXVECTOR4) };
	UINT pnBufferOffsets[5] = { 0, 0, 0, 0, 0 };
	AssembleToVertexBuffer(5, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_pd3dIndexBuffer = CreateBuffer(pd3dDevice, sizeof(UINT), m_nIndices, m_pnIndices, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT, 0);

	//CreateRasterizerState(pd3dDevice);
	CreateConstantBuffer(pd3dDevice);

	IdleSet();//�ִϸ��̼� ������ �ʱ�ȭ	

	float fx ,fy,fz;
	fx = fy = fz = 0.2f;

	m_bcBoundingCube.m_d3dxvMinimum = D3DXVECTOR3(-fx, -fy, -fz);
	m_bcBoundingCube.m_d3dxvMaximum = D3DXVECTOR3(+fx, +fy, +fz);
}


CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pd3dWeightBuffer) m_pd3dWeightBuffer->Release();
	if (m_pd3dBoneIndiceBuffer) m_pd3dBoneIndiceBuffer->Release();
	if (m_pd3dcbBones) m_pd3dcbBones->Release();

	if (m_pd3dxvPositions) delete[] m_pd3dxvPositions;
	if (m_pd3dxvNormals) delete[] m_pd3dxvNormals;
	if (m_pd3dxvTexCoords) delete[] m_pd3dxvTexCoords;
	if (m_pd3dxvBoneWeights) delete[] m_pd3dxvBoneWeights;
	if (m_pd3dxvBoneIndices) delete[] m_pd3dxvBoneIndices;
	if (m_pd3dxmtxFinalBone) delete[] m_pd3dxmtxFinalBone;
	if (m_pd3dxmtxSQTTransform) delete[] m_pd3dxmtxSQTTransform;
}

void CSkinnedMesh::UpdateBoneTransform(ID3D11DeviceContext *pd3dDeviceContext, int nAnimationNum, int nNowFrame)
{
	for (int i = 0; i < s_nBoneCount; i++)
	{
		MakeBoneMatrix(nNowFrame, nAnimationNum, i, *(m_pd3dxmtxSQTTransform + i));
	}
	// ���������� ���� �⺻ ����������� �����־� ���� ����� ������ش�.
	for (int i = 0; i < s_nBoneCount; i++)
	{
		D3DXMATRIX offset = s_pd3dxmtxBoneOffsets[i];
		D3DXMATRIX toRoot = m_pd3dxmtxSQTTransform[i];
		D3DXMatrixMultiply(&m_pd3dxmtxFinalBone[i], &offset, &toRoot);
	}

	// ������۷� ���� ��İ��� �Ѱ�����.
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbBones, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_CB_SKINNED *pcbBones = (VS_CB_SKINNED*)d3dMappedResource.pData;
	for (int i = 0; i < s_nBoneCount; i++)
	{
		D3DXMatrixTranspose(&pcbBones->m_d3dxmtxBone[i], &m_pd3dxmtxFinalBone[i]);
	}
	//World��İ� ����Ļ����� <ĳ���� ��ġ���ÿ� ���> ����, ���߿� �� �� ����ȭ ����� ã�ƺ��ڴ�.
	D3DXMatrixTranspose(&pcbBones->m_d3dxmtxBone[27], &m_d3dxmtxLocalTransform);
	pd3dDeviceContext->Unmap(m_pd3dcbBones, 0);
	//��� ���۸� ����(VS_SLOT_SKINNEDBONE)�� �����Ѵ�.
	pd3dDeviceContext->VSSetConstantBuffers(10, 1, &m_pd3dcbBones);
}

void CSkinnedMesh::MakeBoneMatrix(int nNowframe, int nAnimationNum, int nBoneNum, D3DXMATRIX& BoneMatrix)
{
	// XMAffine �Լ������� scale�� VECTOR3�� ������
	// D3DXAffine �Լ������� scale�� ����� ����Ѵ�.
	if (s_ppBoneAnimationData[nAnimationNum][nBoneNum].m_nFrameCount != 0)
	{
		float fScale = s_ppBoneAnimationData[nAnimationNum][nBoneNum].m_pd3dxvScale[nNowframe].z;
		D3DXVECTOR3 d3dxvTranslate = s_ppBoneAnimationData[nAnimationNum][nBoneNum].m_pd3dxvTranslate[nNowframe];
		D3DXQUATERNION d3dxvQuaternion = s_ppBoneAnimationData[nAnimationNum][nBoneNum].m_pd3dxvQuaternion[nNowframe];
		D3DXVECTOR3 d3dxvZero = { 0.0f, 0.0f, 0.0f };

		D3DXMatrixAffineTransformation(&BoneMatrix, fScale, &d3dxvZero, &d3dxvQuaternion, &d3dxvTranslate);
	}
	else // �ش� ���� �ִϸ��̼� �������� �ƿ� ���� ��� ��������� ��������.
	{
		D3DXMatrixIdentity(&BoneMatrix);
	}
}

void CSkinnedMesh::CreateConstantBuffer(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VS_CB_SKINNED);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&bd, NULL, &m_pd3dcbBones);
}

// ���� �ִϸ��̼��� ���� ���������� �ѱ��.
// �߰����� �ִϸ��̼� ������ ���� ������ �������� ��� true�� �����Ѵ�.
bool CSkinnedMesh::FBXFrameAdvance(float fTimeElapsed)//2016.5.14 ������ �κ�
{
	m_fFBXAnimationTime += fTimeElapsed;

	if (m_fFBXAnimationTime > ANIFRAMETIME)	// 0.0333333f�ʰ� ������ 1������ �ø���.
	{
		if (m_nFBXNowFrameNum < m_nFBXMaxFrameNum - 1)
		{
			m_nFBXNowFrameNum++;
			m_fFBXAnimationTime = 0.0f;//
			return false;//�ִϸ��̼��� �����°�?
		}
		else
		{
			m_nFBXNowFrameNum = m_nFBXStartFrameNum;//������
			m_fFBXAnimationTime = 0.0f;
			return true;//�ִϸ��̼��� �����°�?
		}
	}
	else
		return false;//�ִϸ��̼��� �����°�?
}

void CSkinnedMesh::SetAnimation(int nFBXAnimation) // �ǹ̾��� �ڵ�δ�. ������ ������ ������
{
	m_nFBXAnimationNum = nFBXAnimation;
	m_nFBXNowFrameNum = 0;
	m_nFBXMaxFrameNum = s_ppBoneAnimationData[nFBXAnimation][1].m_nFrameCount;//2016.5.13 ������ 0�� ���� 1�̾���.
																			  //m_nFBXMaxFrameNum = 81;
	m_fFBXAnimationTime = 0.0f;
}

void CSkinnedMesh::IdleSet()
{
	m_nFBXAnimationNum = 0;// ANIM_STATIC;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 0;
	m_nFBXMaxFrameNum = 50;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}

void CSkinnedMesh::WalkAnimationSet()
{
	m_nFBXAnimationNum = 0;// ANIM_STATIC;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 51;
	m_nFBXMaxFrameNum = 80;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}

void CSkinnedMesh::RunAnimationSet()
{
	m_nFBXAnimationNum = 0;// ANIM_STATIC;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 81;
	m_nFBXMaxFrameNum = 101;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}
void CSkinnedMesh::HelloSet()
{
	m_nFBXAnimationNum = 0;// ANIM_STATIC;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 321;
	m_nFBXMaxFrameNum = 380;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}
void CSkinnedMesh::ShootPistol()
{
	m_nFBXAnimationNum = 1;// ANIM_IK;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 8;
	m_nFBXMaxFrameNum = 22;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}
void CSkinnedMesh::ShootRifle()
{
	m_nFBXAnimationNum = 1;// ANIM_IK;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 94;
	m_nFBXMaxFrameNum = 100;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}
void CSkinnedMesh::ShootSniper()
{
	m_nFBXAnimationNum = 1;// ANIM_IK;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 301;
	m_nFBXMaxFrameNum = 400;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}

void CSkinnedMesh::ReloadRifle()
{
	m_nFBXAnimationNum = 1;// ANIM_IK;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 100;
	m_nFBXMaxFrameNum = 160;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}

void CSkinnedMesh::ZombieWalk()
{
	m_nFBXAnimationNum = 0;// ANIM_IK;
	m_fFBXAnimationTime = 0.0f;

	m_nFBXStartFrameNum = 149;
	m_nFBXMaxFrameNum = 206;
	m_nFBXNowFrameNum = m_nFBXStartFrameNum;
}

void CSkinnedMesh::LoadAnimationSet()
{
	ifstream fin("Character/Animation.data");

	string ignore;
	fin >> ignore >> ignore >> ignore;//[AnimationData]

	s_pd3dxmtxBoneOffsets = new D3DXMATRIX[s_nBoneCount];
	// ���� ��ü�� ������ ����� ����
	fin >> ignore; //[OFFSET_MATRIX]
	for (int i = 0; i < s_nBoneCount; i++)
		fin >> ignore >> s_pd3dxmtxBoneOffsets[i]._11 >> s_pd3dxmtxBoneOffsets[i]._12 >> s_pd3dxmtxBoneOffsets[i]._13 >> s_pd3dxmtxBoneOffsets[i]._14
		>> s_pd3dxmtxBoneOffsets[i]._21 >> s_pd3dxmtxBoneOffsets[i]._22 >> s_pd3dxmtxBoneOffsets[i]._23 >> s_pd3dxmtxBoneOffsets[i]._24
		>> s_pd3dxmtxBoneOffsets[i]._31 >> s_pd3dxmtxBoneOffsets[i]._32 >> s_pd3dxmtxBoneOffsets[i]._33 >> s_pd3dxmtxBoneOffsets[i]._34
		>> s_pd3dxmtxBoneOffsets[i]._41 >> s_pd3dxmtxBoneOffsets[i]._42 >> s_pd3dxmtxBoneOffsets[i]._43 >> s_pd3dxmtxBoneOffsets[i]._44;

	fin >> ignore;//[Animation_clip]

				  // ���⿡������ �ִϸ��̼��� ��´�.
	s_ppBoneAnimationData = new BoneAnimationData*[s_nAnimationClip];
	BoneAnimationData *pBoneAnimationData;
	for (int k = 0; k < s_nAnimationClip; k++)
	{
		fin >> ignore >> ignore;//clip_name;
		pBoneAnimationData = new BoneAnimationData[s_nBoneCount];

		for (int i = 0; i < s_nBoneCount; i++)//bone ������ŭ
		{
			fin >> ignore >> ignore >> pBoneAnimationData[i].m_nFrameCount;//i��° bone�� ������ī����

			pBoneAnimationData[i].m_pd3dxvTranslate = new D3DXVECTOR3[pBoneAnimationData[i].m_nFrameCount];
			pBoneAnimationData[i].m_pd3dxvScale = new D3DXVECTOR3[pBoneAnimationData[i].m_nFrameCount];
			pBoneAnimationData[i].m_pd3dxvQuaternion = new D3DXVECTOR4[pBoneAnimationData[i].m_nFrameCount];
			pBoneAnimationData[i].m_pfAniTime = new float[pBoneAnimationData[i].m_nFrameCount];

			for (int j = 0; j < pBoneAnimationData[i].m_nFrameCount; j++)
			{
				fin >> ignore >> pBoneAnimationData[i].m_pfAniTime[j];

				fin >> ignore >> pBoneAnimationData[i].m_pd3dxvTranslate[j].x >> pBoneAnimationData[i].m_pd3dxvTranslate[j].y
					>> pBoneAnimationData[i].m_pd3dxvTranslate[j].z;
				fin >> ignore >> pBoneAnimationData[i].m_pd3dxvScale[j].x >> pBoneAnimationData[i].m_pd3dxvScale[j].y
					>> pBoneAnimationData[i].m_pd3dxvScale[j].z;
				fin >> ignore >> pBoneAnimationData[i].m_pd3dxvQuaternion[j].x >> pBoneAnimationData[i].m_pd3dxvQuaternion[j].y
					>> pBoneAnimationData[i].m_pd3dxvQuaternion[j].z >> pBoneAnimationData[i].m_pd3dxvQuaternion[j].w;
			}
		}
		s_ppBoneAnimationData[k] = pBoneAnimationData;
	}
	fin.close();
}

void CSkinnedMesh::AnimationDestroy()
{
	if (s_pd3dxmtxBoneOffsets)
		delete[]s_pd3dxmtxBoneOffsets;
	if (s_ppBoneAnimationData)
	{
		for (int i = 0; i < s_nAnimationClip; i++)
		{
			for (int j = 0; j < s_nBoneCount; j++)
			{
				if (s_ppBoneAnimationData[i][j].m_nFrameCount != 0)
				{
					delete[] s_ppBoneAnimationData[i][j].m_pd3dxvTranslate;
					delete[] s_ppBoneAnimationData[i][j].m_pd3dxvScale;
					delete[] s_ppBoneAnimationData[i][j].m_pd3dxvQuaternion;
					delete[] s_ppBoneAnimationData[i][j].m_pfAniTime;
				}
			}
			delete[] s_ppBoneAnimationData[i];
		}
		delete[] s_ppBoneAnimationData;
	}
}
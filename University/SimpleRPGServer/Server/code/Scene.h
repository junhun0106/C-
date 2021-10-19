#pragma once

#include "Player.h"

class CScene
{
public:
	CScene();
	virtual ~CScene();

	void LoadWolrdData() {} // 초기 세상 데이터 로드 함수

	virtual void SetUpObjects() {} // 초기 세상 빌드 함수
	virtual void UpdateObjects() {} // Rendering 세상 빌드 함수
};
class WorldScene : public CScene
{
	CObjects **m_ppStaticObjects;
	CObjects **m_ppDynamicObjects;

public:
	WorldScene() {}
	virtual ~WorldScene() {}


	virtual void SetUpObjects() {}
	virtual void UpadateObjects() {} 

};
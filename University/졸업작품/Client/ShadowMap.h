#pragma once

#include "Scene.h"

class ShadowMap
{
public:
	ShadowMap(ID3D11Device *pd3dDevice, UINT width, UINT height);
	~ShadowMap();

	void BindDSV_And_SetNullRenderTarget(ID3D11DeviceContext* pd3dDeviceContext);

private:
	UINT mWidth;
	UINT mHeight;

	D3D11_VIEWPORT mViewport;

public:
	ID3D11ShaderResourceView* mDepthMapSRV;
	ID3D11DepthStencilView* mDepthMapDSV;
};
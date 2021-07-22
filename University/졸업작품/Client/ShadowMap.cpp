#include "stdafx.h"
#include "ShadowMap.h"


ShadowMap::ShadowMap(ID3D11Device *pd3dDevice, UINT width, UINT height)
	: mWidth(width), mHeight(height), mDepthMapSRV(nullptr), mDepthMapDSV(nullptr)
{
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = mWidth;
	mViewport.Height = mHeight;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D *pDepthMap = nullptr;
	pd3dDevice->CreateTexture2D(&texDesc, nullptr, &pDepthMap);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	pd3dDevice->CreateDepthStencilView(pDepthMap, &dsvDesc, &mDepthMapDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	pd3dDevice->CreateShaderResourceView(pDepthMap, &srvDesc, &mDepthMapSRV);

	pDepthMap->Release();
}


ShadowMap::~ShadowMap()
{
}


void ShadowMap::BindDSV_And_SetNullRenderTarget(ID3D11DeviceContext* pd3dDeviceContext)
{
	pd3dDeviceContext->RSSetViewports(1, &mViewport);

	ID3D11RenderTargetView *pd3dRenderTargets[1] = { nullptr };
	pd3dDeviceContext->OMSetRenderTargets(1, pd3dRenderTargets, mDepthMapDSV);

	pd3dDeviceContext->ClearDepthStencilView(mDepthMapDSV, D3D11_CLEAR_DEPTH, 1.f, 0);
}
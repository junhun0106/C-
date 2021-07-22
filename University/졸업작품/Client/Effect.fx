//-------------------------------------------------------------------------------------------------------------------------------
// File: LabProject14.fx
//-------------------------------------------------------------------------------------------------------------------------------

//#define _WITH_SKYBOX_TEXTURE_ARRAY
#define _WITH_SKYBOX_TEXTURE_CUBE
//#define _WITH_TERRAIN_TEXTURE_ARRAY

//-------------------------------------------------------------------------------------------------------------------------------
// Constant Buffer Variables
//-------------------------------------------------------------------------------------------------------------------------------
cbuffer cbViewProjectionMatrix : register(b0)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
};

cbuffer cbWorldMatrix : register(b1)
{
	row_major matrix		gmtxWorld : packoffset(c0);
};

cbuffer cbTextureMatrix : register(b2)
{
	matrix		gmtxTexture : packoffset(c0);
};

cbuffer cbTerrain : register(b3)
{
	int4		gvTerrainTextureIndex : packoffset(c0);
};

cbuffer cbSkyBox : register(b4)//ÀÌ°Å ¹ö·Á¾ßµÊ
{
	int4		gvSkyBoxTextureIndex : packoffset(c0);
};

cbuffer cbSkinned : register(b10)
{
	matrix gBoneTransform[30];
}

//-------------------------------------------------------------------------------------------------------------------------------
// Shadow
//-------------------------------------------------------------------------------------------------------------------------------
static matrix gmtxProjectToTexture = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f };
Texture2D gtxtShadowMap : register(t17);
SamplerState gssShadowMap : register(s7);
SamplerComparisonState gssPCFShadow : register(s8);
cbuffer cbShadow : register(b6)
{
	matrix		gmtxShadowTransform : packoffset(c0);
};

Texture2D gtxtDefault : register(t0);
Texture2D gtxtDefaultDetail : register(t1);
Texture2D gtxtTerrain : register(t2);

#ifdef _WITH_TERRAIN_TEXTURE_ARRAY
Texture2D gtxtTerrainDetails[10] : register(t3);
#else
Texture2D gtxtTerrainDetail : register(t3);
#endif

#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
Texture2DArray gtxtSkyBox : register(t13);
#else
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
//TextureCube gtxtSkyBox : register(t13);
Texture2D gtxtSkyBox : register(t13);
#else
Texture2D gtxtSkyBox : register(t13);
#endif
#endif

Texture2D gtxtZoom : register(t32);

SamplerState gssDefault : register(s0);
SamplerState gssDefaultDetail : register(s1);
SamplerState gssTerrain : register(s2);
SamplerState gssTerrainDetail : register(s3);
SamplerState gssSkyBox : register(s4);

SamplerState gssZoom : register(s8);

#include "Light.fx"

// fog
static const float4 gcFogColor = float4(0.1f, 0.1f, 0.1f, 1.0f);
static const float gcFogRange = 100.0f;

float4 Fog(float4 cColor, float3 position_W)
{
    float distance = length(position_W - gvCameraPosition.xyz);
    float fogFactor = saturate((distance / gcFogRange) - 0.2f);

    float4 cColorByFog = lerp(cColor, gcFogColor, fogFactor);

    return (cColorByFog);
}

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3	position : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float4	position : SV_POSITION;
	float2	texCoord : TEXCOORD0;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_DETAIL_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

struct VS_DETAIL_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4 shadowPosition : TEXCOORD2;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_DETAIL_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

struct VS_DETAIL_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

struct VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_INSTANCED_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
	float4x4 mtxTransform : INSTANCEPOS;
};

struct VS_INSTANCED_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_INSTANCED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4x4 mtxTransform : INSTANCEPOS;
};

struct VS_INSTANCED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_INSTANCED_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float4x4 mtxTransform : INSTANCEPOS;
};

struct VS_INSTANCED_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

//-------------------------------------------------------------------------------------------------------------------------------
struct VS_INSTANCED_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4x4 mtxTransform : INSTANCEPOS;
};

struct VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;

	float2 texCoordShadow : TEXCOORD1;
};

//-------------------------------------------------------------------------------------------------------------------------------
VS_DIFFUSED_OUTPUT VSDiffusedColor(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output = (VS_DIFFUSED_OUTPUT)0;
	output.position = mul(float4(input.position, 1.0f), mul(mul(gmtxWorld, gmtxView), gmtxProjection));
	output.color = input.color;

	return(output);
}

float4 PSDiffusedColor(VS_DIFFUSED_OUTPUT input) : SV_Target
{
	return(input.color);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_LIGHTING_OUTPUT VSLightingColor(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output = (VS_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

float4 PSLightingColor(VS_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	[unroll]
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	[unroll]
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[unroll]
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cFogColor = Fog(cIllumination, input.positionW);

	return(cFogColor);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSTexturedColor(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSTexturedColor(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord);
	
	if (cColor.r > 0.9f &&
		cColor.g > 0.9f &&
		cColor.b > 0.9f) discard;

	return(cColor);
}

struct VS_SKINNED_INSTANCED_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4 boneindices : BONEIDS;
	float4 weights : BLENDWEIGHTS;
	float4x4 mtxTransform : INSTANCEPOS;
};
//-------------------------------------------------------------------------------------------------------------------------------
VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLightingColor(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output = (VS_TEXTURED_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	matrix shadowProject = mul(mul(gmtxWorld, gmtxShadowTransform), gmtxProjectToTexture);
	output.shadowPosition = mul(float4(input.position, 1.0f), shadowProject);

	return(output);
}

float4 PSTexturedLightingColor(VS_TEXTURED_LIGHTING_OUTPUT input) : SV_Target
{
	float fShadowFactor = 1.0f, fBias = 0.0006f;
	float3 fShadowPositionH = input.shadowPosition.xyz / input.shadowPosition.w;
	float fsDepth = gtxtShadowMap.Sample(gssShadowMap, fShadowPositionH.xy).r;
	if (fShadowPositionH.z > (fsDepth + fBias)) fShadowFactor = 0.3f;

	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
	{
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
		A[i] *= fShadowFactor;
		D[i] *= fShadowFactor;
		S[i] *= fShadowFactor;
	}
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord) * cIllumination;

	float4 cFogColor = Fog(cColor, input.positionW);

	return(cFogColor);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_DETAIL_TEXTURED_OUTPUT VSDetailTexturedColor(VS_DETAIL_TEXTURED_INPUT input)
{
	VS_DETAIL_TEXTURED_OUTPUT output = (VS_DETAIL_TEXTURED_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

float4 PSDetailTexturedColor(VS_DETAIL_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cBaseTexColor = gtxtDefault.Sample(gssDefault, input.texCoordBase);
	float4 cDetailTexColor = gtxtDefaultDetail.Sample(gssDefaultDetail, input.texCoordDetail);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	//    float4 cAlphaTexColor = gtxtTerrainAlphaTexture.Sample(gTerrainSamplerState, input.texcoord0);
	//    float4 cColor = cIllumination * lerp(cBaseTexColor, cDetailTexColor, cAlphaTexColor.r);
	return(cColor);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_DETAIL_TEXTURED_LIGHTING_OUTPUT VSDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_INPUT input)
{
	VS_DETAIL_TEXTURED_LIGHTING_OUTPUT output = (VS_DETAIL_TEXTURED_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

VS_DETAIL_TEXTURED_LIGHTING_OUTPUT VSAnimatedDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_INPUT input)
{
	VS_DETAIL_TEXTURED_LIGHTING_OUTPUT output = (VS_DETAIL_TEXTURED_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = mul(float4(input.texCoordDetail, 0.0f, 1.0f), gmtxTexture).xy;

	return(output);
}

float4 PSDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cBaseTexColor = gtxtDefault.Sample(gssDefault, input.texCoordBase);
	float4 cDetailTexColor = gtxtDefaultDetail.Sample(gssDefaultDetail, input.texCoordDetail);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
//    float4 cAlphaTexColor = gtxtTerrainAlphaTexture.Sample(gTerrainSamplerState, input.texcoord0);
//    float4 cColor = cIllumination * lerp(cBaseTexColor, cDetailTexColor, cAlphaTexColor.r);
	return(cColor*cIllumination);
}

VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_OUTPUT VSTerrainDetailTexturedLightingColor(VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_INPUT input)
{
	VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_OUTPUT output = (VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

#ifdef _WITH_TERRAIN_TEXTURE_ARRAY
float4 PSTerrainDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cBaseTexColor = gtxtTerrain.Sample(gssTerrain, input.texCoordBase);
	float4 cDetailTexColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	//	cDetailTexColor = gtxtTerrainDetails[gvTerrainTextureIndex.a].Sample(gssTerrainDetail, input.texCoordDetail); //Error
	///*
	switch (gvTerrainTextureIndex.a)
	{
	case 0:
		cDetailTexColor = gtxtTerrainDetails[0].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 1:
		cDetailTexColor = gtxtTerrainDetails[1].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 2:
		cDetailTexColor = gtxtTerrainDetails[2].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 3:
		cDetailTexColor = gtxtTerrainDetails[3].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 4:
		cDetailTexColor = gtxtTerrainDetails[4].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 5:
		cDetailTexColor = gtxtTerrainDetails[5].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 6:
		cDetailTexColor = gtxtTerrainDetails[6].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 7:
		cDetailTexColor = gtxtTerrainDetails[7].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 8:
		cDetailTexColor = gtxtTerrainDetails[8].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	case 9:
		cDetailTexColor = gtxtTerrainDetails[9].Sample(gssTerrainDetail, input.texCoordDetail);
		break;
	}
	//*/
	float4 cColor = saturate((cIllumination * cBaseTexColor * 0.7f) + (cDetailTexColor * 0.3f));

	return(cColor);
}
#else
float4 PSTerrainDetailTexturedLightingColor(VS_TERRAIN_DETAIL_TEXTURED_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i) 
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cBaseTexColor = gtxtTerrain.Sample(gssTerrain, input.texCoordBase);
	float4 cDetailTexColor = gtxtTerrainDetail.Sample(gssTerrainDetail, input.texCoordDetail);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)) * cIllumination;

	return(cColor);
}
#endif

//-------------------------------------------------------------------------------------------------------------------------------
#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
float4 PSSkyBoxTexturedColor(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float3 uvw = float3(input.texCoord, gvSkyBoxTextureIndex.a);
	float4 cColor = gtxtSkyBox.Sample(gssSkyBox, uvw);
	return(cColor);
}
#else
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBoxTexturedColor(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output = (VS_SKYBOX_CUBEMAP_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;
	//position, f3
	//texcoord f2
	//out.positionL -> f3
	//out.position ->f4
	return(output);
}

float4 PSSkyBoxTexturedColor(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtSkyBox.Sample(gssSkyBox, input.texCoord);

    cColor = lerp(cColor, gcFogColor, 0.7f);

	return(cColor);
}
#else
float4 PSSkyBoxTexturedColor(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtSkyBox.Sample(gssSkyBox, input.texCoord);
	return(cColor);
}
#endif
#endif

//-------------------------------------------------------------------------------------------------------------------------------
VS_INSTANCED_DIFFUSED_OUTPUT VSInstancedDiffusedColor(VS_INSTANCED_DIFFUSED_INPUT input)
{
	VS_INSTANCED_DIFFUSED_OUTPUT output = (VS_INSTANCED_DIFFUSED_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxView), gmtxProjection);
	output.color = input.color;
	return(output);
}

float4 PSInstancedDiffusedColor(VS_INSTANCED_DIFFUSED_OUTPUT input) : SV_Target
{
	return(input.color);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_INSTANCED_LIGHTING_OUTPUT VSInstancedLightingColor(VS_INSTANCED_LIGHTING_INPUT input)
{
	VS_INSTANCED_LIGHTING_OUTPUT output = (VS_INSTANCED_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)input.mtxTransform);
	output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

float4 PSInstancedLightingColor(VS_INSTANCED_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	return(cIllumination);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_INSTANCED_TEXTURED_OUTPUT VSInstancedTexturedColor(VS_INSTANCED_TEXTURED_INPUT input)
{
	VS_INSTANCED_TEXTURED_OUTPUT output = (VS_INSTANCED_TEXTURED_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSInstancedTexturedColor(VS_INSTANCED_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord);

	return(cColor);
}

//-------------------------------------------------------------------------------------------------------------------------------
VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT VSInstancedTexturedLightingColor(VS_INSTANCED_TEXTURED_LIGHTING_INPUT input)
{
	VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT output = (VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)input.mtxTransform);
	output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;
	return(output);
}

float4 PSInstancedTexturedLightingColor(VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord) * cIllumination;

	return(cColor);
}

// Texture To Screen
VS_TEXTURED_OUTPUT VSTextureToScreen(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
    output.position = float4(input.position, 1.0f);
    output.texCoord = input.texCoord;

    return(output);
}

float4 PSTextureToScreen(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 cColor = gtxtZoom.Sample(gssZoom, input.texCoord);
    if (cColor.a < 0.9f) discard;
    return(cColor);
}


// Diffused color Projection Shader
VS_DIFFUSED_OUTPUT VSDiffusedProjection(VS_DIFFUSED_INPUT input)
{
    VS_DIFFUSED_OUTPUT output = (VS_DIFFUSED_OUTPUT)0;
    output.position = mul(float4(input.position, 1.0f), gmtxWorld);
    output.color = input.color;

    return(output);
}

float4 PSDiffusedProjection(VS_DIFFUSED_OUTPUT input) : SV_Target
{
    return (input.color);
}

// radar shadar, textured color & rotating
VS_TEXTURED_OUTPUT VSRadar(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
    output.position = mul(float4(input.position, 1.0f), gmtxWorld);
    output.texCoord = input.texCoord;

    return (output);
}

float4 PSRadar(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord);

    return (cColor);
}

float4 PSRadarGrid(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord);

    if (cColor.a < 0.8f) discard;

    return (cColor);
}

struct VS_SKINNED_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4 boneindices : BONEIDS;
	float4 weights : BLENDWEIGHTS;
};

struct VS_SKINNED_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
};

VS_SKINNED_OUTPUT VSSkinned(VS_SKINNED_INPUT input)
{
	VS_SKINNED_OUTPUT output = (VS_SKINNED_OUTPUT)0;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weights.x;
	weights[1] = input.weights.y;
	weights[2] = input.weights.z;
	weights[3] = 1 - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; i++)
	{
		posL += weights[i] * mul(float4(input.position, 1.0f), gBoneTransform[input.boneindices[i]]).xyz;
		normalL += weights[i] * mul(input.normal, (float3x3)gBoneTransform[input.boneindices[i]]);
	}
	//¹ÚÁ¾Çõ
	output.positionW = mul(mul(float4(posL, 1.0f), gBoneTransform[27]), gmtxWorld).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = mul(normalL, (float3x3)gmtxWorld);
	output.texCoord = input.texCoord;

	return output;
}

float4 PSSkinned(VS_SKINNED_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord) * cIllumination;

return(cColor);
}

//////////////////////////////////////////////////////////////////////////////////////////

VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT VSSkinnedInstance(VS_SKINNED_INSTANCED_INPUT input)
{
	VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT output = (VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT)0;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weights.x;
	weights[1] = input.weights.y;
	weights[2] = input.weights.z;
	weights[3] = 1 - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; i++)
	{
		posL += weights[i] * mul(float4(input.position, 1.0f), gBoneTransform[input.boneindices[i]]).xyz;
		normalL += weights[i] * mul(input.normal, (float3x3)gBoneTransform[input.boneindices[i]]);
	}
	//¹ÚÁ¾Çõ
	output.positionW = mul(mul(float4(posL, 1.0f), gBoneTransform[27]), input.mtxTransform).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = mul(normalL, (float3x3)input.mtxTransform);
	output.texCoord = input.texCoord;

	return output;
}

float4 PSSkinnedInstance(VS_INSTANCED_TEXTURED_LIGHTING_OUTPUT input) : SV_Target
{
		input.normalW = normalize(input.normalW);

	float3 vToCamera = normalize(gvCameraPosition.xyz - input.positionW);
	float4 A[MAX_LIGHTS], D[MAX_LIGHTS], S[MAX_LIGHTS];
	[unroll]
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
		ComputeDirectionalLight(gMaterial, gDirectionalLights[i], input.normalW, vToCamera, A[i], D[i], S[i]);
	for(int j = 0; j < MAX_POINT_LIGHTS; ++j)
		ComputePointLight(gMaterial, gPointLights[j], input.positionW, input.normalW, vToCamera, 
			A[j + MAX_DIRECTIONAL_LIGHTS], D[j + MAX_DIRECTIONAL_LIGHTS], S[j + MAX_DIRECTIONAL_LIGHTS]);
	for(int k = 0; k < MAX_SPOT_LIGHTS; ++k)
		ComputeSpotLight(gMaterial, gSpotLights[k], input.positionW, input.normalW, vToCamera, 
			A[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], D[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS], 
			S[k + MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS]);
	float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int l = 0; l < MAX_LIGHTS; ++l)
		cIllumination += A[l] + D[l] + S[l];
	cIllumination.a = gMaterial.m_cDiffuse.a;

	float4 cColor = gtxtDefault.Sample(gssDefault, input.texCoord) * cIllumination;

    float4 cFogColor = Fog(cColor, input.positionW);

    return(cFogColor);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ±èÁ¤Çå

struct VS_GEOMETRY_INSTANCED_INPUT {
	float3 posW : POSITION;
	float4x4 MatrixTransform : INSTANCEPOS;
};
struct VS_GEOMETRY_INSTANCED_OUTPUT {
	float3 centerW : POSITION;
	float4x4 MatrixTransform : INSTANCEPOS;
};
struct GS_GEOMETRY_INSTANCED_OUTPUT {
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float2 texcoord : TEXCOORD;
};
VS_GEOMETRY_INSTANCED_OUTPUT VS_STEER(VS_GEOMETRY_INSTANCED_INPUT input) {
	VS_GEOMETRY_INSTANCED_OUTPUT output = (VS_GEOMETRY_INSTANCED_OUTPUT)0;
	output.centerW = input.posW;
	output.MatrixTransform = input.MatrixTransform;
	return (output);
}
[maxvertexcount(4)]
void GS_STEER(point VS_GEOMETRY_INSTANCED_OUTPUT input[1], inout TriangleStream<GS_GEOMETRY_INSTANCED_OUTPUT> tristream) {
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = float3(0.0f, 0.0f, 1.0f);
	float3 vRight = cross(vUp, vLook);
	float fHalf = 10.0f;
	float4 pVertices[4] = {
		float4(float3(input[0].centerW - (fHalf * vRight) + (fHalf * vLook)), 1.0f),
		float4(float3(input[0].centerW - (fHalf * vRight) - (fHalf * vLook)), 1.0f),
		float4(float3(input[0].centerW + (fHalf * vRight) + (fHalf * vLook)), 1.0f),
		float4(float3(input[0].centerW + (fHalf * vRight) - (fHalf * vLook)), 1.0f)
	};
	float2 pTexCoords[4] = {
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};
	GS_GEOMETRY_INSTANCED_OUTPUT output = (GS_GEOMETRY_INSTANCED_OUTPUT)0;
	for (int i = 0; i < 4; i++) {
		output.posH = mul(pVertices[i], input[0].MatrixTransform);
		output.posW = output.posH.xyz;
		output.posH = mul(output.posH, gmtxView);
		output.posH = mul(output.posH, gmtxProjection);
		output.texcoord = float2(pTexCoords[i]);
		tristream.Append(output);
	}
}
float4 PS_STEER(GS_GEOMETRY_INSTANCED_OUTPUT input) : SV_Target{
	float4 tex = gtxtDefault.Sample(gssDefault, input.texcoord);
	if (tex.r > 0.9 && tex.g > 0.9 && tex.b > 0.9) discard;
	return tex;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Particle Effect
//-------------------------------------------------------------------------------------------------------------------------------
#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE_1	1
#define PARTICLE_TYPE_FLARE_2	2
#define PARTICLE_TYPE_SPARK		3
#define PARTICLE_TYPE_SMOKE		4
#define PARTICLE_TYPE_BLOOD		5

Texture1D gtxtRandom : register(t0); // GS Resource
Texture2DArray gtxtArrFP : register(t19); // PS Resource
SamplerState gssParticleByGS : register(s0);
SamplerState gssParticleByPS : register(s8);

cbuffer cbParticleInfo : register(b7) // VS, GS Constant Buffer
{
	float3 gvParticleEmitPosition : packoffset(c0);
	float gfGameTime : packoffset(c0.w);
	float3 gvAcceleration : packoffset(c1);
	float gfTimeStep : packoffset(c1.w);
}

cbuffer cbLights_Particles : register(b8)
{
	DIRECTIONAL_LIGHT		gDirectionalLights_Particles[MAX_DIRECTIONAL_LIGHTS];
	POINT_LIGHT				gPointLights_Particles[MAX_POINT_LIGHTS];
	SPOT_LIGHT				gSpotLights_Particles[MAX_SPOT_LIGHTS];
	float4					gcLightGlobalAmbient_Particles;
	float4					gvCameraPosition_Particles;
};

float3 RandUnitVec3(float offset)
{
	float u = (gfGameTime + offset);
	float3 v = gtxtRandom.SampleLevel(gssParticleByGS, u, 0).xyz;
	return normalize(v);
} // didn't test yet

struct PARTICLE_INPUT
{
	float3 position : POSITION;
	float3 velocity : VELOCITY;
	float2 size : SIZE;
	float age : AGE;
	unsigned int type : TYPE;
};

struct PARTICLE_OUTPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	float color : COLOR;
	float age : AGE;
	unsigned int type : TYPE;
};

struct GS_PARTICLE_OUT
{
	float4 position : SV_POSITION;
	float color : COLOR;
	float3 texCoord : TEXCOORD;
};

float4x4 MatrixRotationAxis(float3 axis, float angle)
{
	float4x4 mtx;

	mtx._m00 = cos(radians(angle)) + (1 - cos(radians(angle))) * axis.x * axis.x;
	mtx._m01 = (1 - cos(radians(angle))) * axis.x * axis.y + sin(radians(angle)) * axis.z;
	mtx._m02 = (1 - cos(radians(angle))) * axis.x * axis.z - sin(radians(angle)) * axis.y;
	mtx._m03 = 0.0f;

	mtx._m10 = (1 - cos(radians(angle))) * axis.x * axis.y - sin(radians(angle)) * axis.z;
	mtx._m11 = cos(radians(angle)) + (1 - cos(radians(angle))) * axis.y * axis.y;
	mtx._m12 = (1 - cos(radians(angle))) * axis.y * axis.z + sin(radians(angle)) * axis.x;
	mtx._m13 = 0.0f;

	mtx._m20 = (1 - cos(radians(angle))) * axis.x * axis.z + sin(radians(angle)) * axis.y;
	mtx._m21 = (1 - cos(radians(angle))) * axis.y * axis.z - sin(radians(angle)) * axis.x;
	mtx._m22 = cos(radians(angle)) + (1 - cos(radians(angle))) * axis.z * axis.z;
	mtx._m23 = 0.0f;

	mtx._m30 = 0.0f;
	mtx._m31 = 0.0f;
	mtx._m32 = 0.0f;
	mtx._m33 = 1.0f;

	return mtx;
}

PARTICLE_INPUT VSParticleStreamOut(PARTICLE_INPUT input)
{
	return(input);
}

[maxvertexcount(3)]
void GSParticleStreamOut(point PARTICLE_INPUT input[1], inout PointStream<PARTICLE_INPUT> pointStream)
{
	input[0].age += gfTimeStep;
	float3 vRandom = gtxtRandom.SampleLevel(gssParticleByGS, gfGameTime, 0).xyz;
	float index = vRandom.x;
	if (input[0].type == PARTICLE_TYPE_EMITTER)
	{
		if (input[0].age > 0.02f) {
			vRandom = normalize(vRandom);

			// fire
			PARTICLE_INPUT particle = (PARTICLE_INPUT)0;
			particle.position = input[0].position + 5.0f * vRandom;
			particle.position.y = 2.0f;
			particle.velocity = -6.0f * vRandom;
			if (particle.velocity.y < 0.0f) particle.velocity.y = 0.0f;
			particle.size = float2(10.0f, 10.0f);
			particle.age = 0.0f;
			if(index * 0.5f + 0.5f < 0.5f)
				particle.type = PARTICLE_TYPE_FLARE_1;
			else 
				particle.type = PARTICLE_TYPE_FLARE_2;
			pointStream.Append(particle);

			// spark
			particle.position = input[0].position + 3.0f * vRandom;
			particle.position.y = 2.0f;
			particle.velocity = 8.0f * vRandom;
			particle.velocity.y = 14.0f;
			particle.size = float2(0.5f, 0.5f);
			particle.age = 0.0f;
			particle.type = PARTICLE_TYPE_SPARK;
			pointStream.Append(particle);

			input[0].age = 0.0f;
		}
		pointStream.Append(input[0]);
	}
	else if (input[0].type == PARTICLE_TYPE_FLARE_1 || input[0].type == PARTICLE_TYPE_FLARE_2)
	{
		if (input[0].age <= vRandom.x + 2.0f) pointStream.Append(input[0]);
	}
	else if (input[0].type == PARTICLE_TYPE_SPARK)
	{
		if (input[0].age <= vRandom.x + 2.0f) pointStream.Append(input[0]);
	}
}

PARTICLE_OUTPUT VSParticleDraw(PARTICLE_INPUT input)
{
	PARTICLE_OUTPUT output = (PARTICLE_OUTPUT)0;

	float t = input.age;
	if (input.type == PARTICLE_TYPE_FLARE_1 || input.type == PARTICLE_TYPE_FLARE_2)
		output.position = (0.5f * gvAcceleration * t * t) + (input.velocity * t) + input.position;
	else if (input.type == PARTICLE_TYPE_SPARK)
		output.position = (0.5f * float3(0.0f, 20.0f, 0.0f) * t * t) + (input.velocity * t) + input.position;

	float fOpacity = 1.0f - smoothstep(0.0f, 1.0f, t) * smoothstep(0.0f, 1.0f, t);
	output.color = fOpacity;

	output.size = input.size;
	output.type = input.type;
	output.age = input.age;

	return(output);
}

[maxvertexcount(4)]
void GSParticleDraw(point PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUT> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;

	float3 vLook = normalize(gvCameraPosition_Particles.xyz - input[0].position);
	float4x4 mtxRotation = MatrixRotationAxis(vLook, -180.0f * input[0].age);
	float3 vRight = normalize(mul(float4(normalize(cross(float3(0.0f, 1.0f, 0.0f), vLook)), 1.0f), mtxRotation).xyz);
	float3 vUp = normalize(cross(vLook, vRight));
	float fHalfWidth = 0.5f * input[0].size.x;
	float fHalfHeight = 0.5f * input[0].size.y;
	float4 vQuad[4] = {
		float4(input[0].position + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f),
		float4(input[0].position + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f),
		float4(input[0].position - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f),
		float4(input[0].position - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f)
	};

	float2 vQuadTexCoord[4] = {
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	matrix mtxViewProjection = mul(gmtxView, gmtxProjection);
	GS_PARTICLE_OUT output = (GS_PARTICLE_OUT)0;
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(vQuad[i], mtxViewProjection);
		if (input[0].type == PARTICLE_TYPE_FLARE_1)
			output.texCoord = float3(vQuadTexCoord[i], 0);
		else if (input[0].type == PARTICLE_TYPE_FLARE_2)
			output.texCoord = float3(vQuadTexCoord[i], 2);
		else if (input[0].type == PARTICLE_TYPE_SPARK)
			output.texCoord = float3(vQuadTexCoord[i], 1);
		output.color = input[0].color;
		triStream.Append(output);
	}
}

float4 PSParticleDraw(GS_PARTICLE_OUT input) : SV_Target
{
	float4 cColor = gtxtArrFP.Sample(gssParticleByPS, input.texCoord);
	return(cColor * input.color * 0.8f);
}

//
// Smoke Particle
//
PARTICLE_INPUT VSParticleStreamOut_Smoke(PARTICLE_INPUT input)
{
	return(input);
}

[maxvertexcount(2)]
void GSParticleStreamOut_Smoke(point PARTICLE_INPUT input[1], inout PointStream<PARTICLE_INPUT> pointStream)
{
	input[0].age += gfTimeStep;
	float3 vRandom = gtxtRandom.SampleLevel(gssParticleByGS, gfGameTime, 0).xyz;
	if (input[0].type == PARTICLE_TYPE_EMITTER)
	{
		if (input[0].age > 0.2f) {
			vRandom = normalize(vRandom);

			// Smoke
			PARTICLE_INPUT particle = (PARTICLE_INPUT)0;
			particle.position = input[0].position + 4.0f * vRandom;
			particle.position.y = 7.0f;
			particle.velocity = -7.0f * vRandom;
			if (particle.velocity.y < 0.0f) particle.velocity.y = 0.0f;
			particle.size = float2(20.0f, 20.0f);
			particle.age = 0.0f;
			particle.type = PARTICLE_TYPE_SMOKE;
			pointStream.Append(particle);
			input[0].age = 0.0f;
		}
		pointStream.Append(input[0]);
	}
	else if (input[0].type == PARTICLE_TYPE_SMOKE)
	{
		if (input[0].age <= vRandom.x + 3.0f) pointStream.Append(input[0]);
	}
}

PARTICLE_OUTPUT VSParticleDraw_Smoke(PARTICLE_INPUT input)
{
	PARTICLE_OUTPUT output = (PARTICLE_OUTPUT)0;

	float t = input.age;
	if (input.type == PARTICLE_TYPE_SMOKE)
		output.position = (0.5f * float3(0.0f, 25.0f, 0.0f) * t * t) + (input.velocity * t) + input.position;

	float fOpacity = 1.0f - smoothstep(0.0f, 1.0f, t * 0.5f) * smoothstep(0.0f, 1.0f, t * 0.5f);
	output.color = fOpacity;

	output.size = input.size * t;
	output.type = input.type;
	output.age = input.age;

	return(output);
}

[maxvertexcount(4)]
void GSParticleDraw_Smoke(point PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUT> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;
	float3 vLook = normalize(gvCameraPosition_Particles.xyz - input[0].position);
	float4x4 mtxRotation = MatrixRotationAxis(vLook, 240.0f * input[0].age);
	float3 vRight = normalize(mul(float4(normalize(cross(float3(0.0f, 1.0f, 0.0f), vLook)), 1.0f), mtxRotation).xyz);
	float3 vUp = normalize(cross(vLook, vRight));	float fHalfWidth = 0.5f * input[0].size.x;
	float fHalfHeight = 0.5f * input[0].size.y;
	float4 vQuad[4] = {
		float4(input[0].position + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f),
		float4(input[0].position + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f),
		float4(input[0].position - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f),
		float4(input[0].position - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f)
	};

	float2 vQuadTexCoord[4] = {
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	matrix mtxViewProjection = mul(gmtxView, gmtxProjection);
	GS_PARTICLE_OUT output = (GS_PARTICLE_OUT)0;
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(vQuad[i], mtxViewProjection);
		if (input[0].type == PARTICLE_TYPE_SMOKE)
			output.texCoord = float3(vQuadTexCoord[i], 0);
		output.color = input[0].color;
		triStream.Append(output);
	}
}

float4 PSParticleDraw_Smoke(GS_PARTICLE_OUT input) : SV_Target
{
	float4 cColor = gtxtArrFP.Sample(gssParticleByPS, input.texCoord);
	return(cColor * input.color * 0.8f);
}

// blood
PARTICLE_INPUT VSParticleStreamOut_Blood(PARTICLE_INPUT input)
{
	return(input);
}

[maxvertexcount(2)]
void GSParticleStreamOut_Blood(point PARTICLE_INPUT input[1], inout PointStream<PARTICLE_INPUT> pointStream)
{
	input[0].age += gfTimeStep;
	float3 vRandom = gtxtRandom.SampleLevel(gssParticleByGS, gfGameTime, 0).xyz;
	if (input[0].type == PARTICLE_TYPE_EMITTER)
	{
		if (input[0].age > 0.005f) {
			vRandom = normalize(vRandom);

			// Blood
			PARTICLE_INPUT particle = (PARTICLE_INPUT)0;
			particle.position = input[0].position;
			particle.position.y = 3.0f;
			particle.velocity = vRandom;
			particle.velocity.y = 5.0f + vRandom.r * 2.0f;
			particle.size = float2(3.0f, 5.0f);
			particle.age = 0.0f;
			particle.type = PARTICLE_TYPE_BLOOD;
			pointStream.Append(particle);
			input[0].age = 0.0f;
		}
		if (gfGameTime > 3.0f) return;
		pointStream.Append(input[0]);
	}
	else if (input[0].type == PARTICLE_TYPE_BLOOD)
	{
		if (input[0].age <= vRandom.x + 3.0f) pointStream.Append(input[0]);
	}
}

PARTICLE_OUTPUT VSParticleDraw_Blood(PARTICLE_INPUT input)
{
	PARTICLE_OUTPUT output = (PARTICLE_OUTPUT)0;

	float t = input.age;
	if (input.type == PARTICLE_TYPE_BLOOD)
		output.position = (0.5f * float3(0.0f, -9.8f, 0.0f) * t * t) + (input.velocity * t) + input.position;

	float fOpacity = 1.0f - smoothstep(0.0f, 1.0f, t) * smoothstep(0.0f, 1.0f, t);
	output.color = fOpacity;

	output.size = input.size * t;
	output.type = input.type;
	output.age = input.age;

	return(output);
}

[maxvertexcount(4)]
void GSParticleDraw_Blood(point PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUT> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;
	float3 vLook = normalize(gvCameraPosition_Particles.xyz - input[0].position);
	float3 vRight = normalize(cross(float3(0.0f, 1.0f, 0.0f), vLook));
	float3 vUp = normalize(cross(vLook, vRight));	float fHalfWidth = 0.5f * input[0].size.x;
	float fHalfHeight = 0.5f * input[0].size.y;
	float4 vQuad[4] = {
		float4(input[0].position + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f),
		float4(input[0].position + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f),
		float4(input[0].position - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f),
		float4(input[0].position - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f)
	};

	float2 vQuadTexCoord[4] = {
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	matrix mtxViewProjection = mul(gmtxView, gmtxProjection);
	GS_PARTICLE_OUT output = (GS_PARTICLE_OUT)0;
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(vQuad[i], mtxViewProjection);
		if (input[0].type == PARTICLE_TYPE_BLOOD)
			output.texCoord = float3(vQuadTexCoord[i], 0);
		output.color = input[0].color;
		triStream.Append(output);
	}
}

float4 PSParticleDraw_Blood(GS_PARTICLE_OUT input) : SV_Target
{
	float4 cColor = gtxtArrFP.Sample(gssParticleByPS, input.texCoord);
	return(cColor * input.color * 0.8f);
}

//
// blurring
//

Texture2D gtxtInput : register(t0);
RWTexture2D<float4> gtxtRWOutput : register(u0);

groupshared float4 gTextureCache[256 + 5 * 2]; // 1 x 11 Kernel

static float gWeights[11] = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };

[numthreads(256, 1, 1)]
void HorzBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID)
{
	if (vGroupThreadID.x < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		gTextureCache[vGroupThreadID.x] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, gtxtInput.Length.x - 1);
		gTextureCache[vGroupThreadID.x + 10] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.x + 5 + i; // 0 ~ 265
		cBlurredColor += gWeights[i + 5] * gTextureCache[k];
	}

	float average = (cBlurredColor.x + cBlurredColor.y + cBlurredColor.z) / 3.0f;

	gtxtRWOutput[vDispatchThreadID.xy] = float4(average, average, average, 1.0f);
}

[numthreads(1, 256, 1)]
void VertBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID)
{
	if (vGroupThreadID.y < 5)
	{
		int y = max(vDispatchThreadID.y - 5, 0);
		gTextureCache[vGroupThreadID.y] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	else if (vGroupThreadID.y >= 256 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, gtxtInput.Length.y - 1);
		gTextureCache[vGroupThreadID.y + 10] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	gTextureCache[vGroupThreadID.y + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.y + 5 + i; // 0 ~ 265
		cBlurredColor += gWeights[i + 5] * gTextureCache[k];
	}

	float average = (cBlurredColor.x + cBlurredColor.y + cBlurredColor.z) / 3.0f;

	gtxtRWOutput[vDispatchThreadID.xy] = float4(average, average, average, 1.0f);
}
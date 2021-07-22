//--------------------------------------------------------------------------------------

//
// Lighting
//
#define MAX_DIRECTIONAL_LIGHTS		1
#define MAX_POINT_LIGHTS			30
#define MAX_SPOT_LIGHTS				1
#define MAX_LIGHTS MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS

struct MATERIAL
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular; //a = power
	float4				m_cEmissive;
};

struct DIRECTIONAL_LIGHT
{
	float4		m_cAmbient;
	float4		m_cDiffuse;
	float4		m_cSpecular;
	float3		m_vDirection;
};

struct POINT_LIGHT
{
	float4		m_cAmbient;
	float4		m_cDiffuse;
	float4		m_cSpecular;
	float3		m_vPosition;
	float		m_fRange;
	float3		m_vAttenuation;
};

struct SPOT_LIGHT
{
	float4		m_cAmbient;
	float4		m_cDiffuse;
	float4		m_cSpecular;
	float3		m_vPosition;
	float		m_fRange;
	float3		m_vDirection;
	float		m_fFalloff;
	float3		m_vAttenuation;
};

cbuffer cbLights : register(b0)
{
	DIRECTIONAL_LIGHT		gDirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	POINT_LIGHT				gPointLights[MAX_POINT_LIGHTS];
	SPOT_LIGHT				gSpotLights[MAX_SPOT_LIGHTS];
	float4					gcLightGlobalAmbient;
	float4					gvCameraPosition;
};

cbuffer cbMaterial : register(b1)
{
	MATERIAL			gMaterial;
};

struct LIGHTEDCOLOR
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular;
};

void ComputeDirectionalLight(MATERIAL mat, DIRECTIONAL_LIGHT L, float3 vNormal, float3 vToCamera,
	out float4 cAmbient, out float4 cDiffuse, out float4 cSpecular)
{
	cDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	cSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	cAmbient = mat.m_cAmbient * (L.m_cAmbient + gcLightGlobalAmbient);

	float3 vToLight = -L.m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	if (fDiffuseFactor > 0.0f)
	{
		float3 vReflect = reflect(-vToLight, vNormal);
		float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0), mat.m_cSpecular.a);
		cDiffuse = mat.m_cDiffuse * (L.m_cDiffuse * fDiffuseFactor);
		cSpecular = mat.m_cSpecular * (L.m_cSpecular * fSpecularFactor);
	}
}

void ComputePointLight(MATERIAL mat, POINT_LIGHT L, float3 vPosition, float3 vNormal, float3 vToCamera,
	out float4 cAmbient, out float4 cDiffuse, out float4 cSpecular)
{
	cDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	cSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 vToLight = L.m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance > L.m_fRange) return;

	cAmbient = mat.m_cAmbient * (L.m_cAmbient + gcLightGlobalAmbient);

	vToLight /= fDistance;
	float fDiffuseFactor = dot(vToLight, vNormal);
	if (fDiffuseFactor > 0.0f)
	{
		float3 vReflect = reflect(-vToLight, vNormal);
		float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0), mat.m_cSpecular.a);
		cDiffuse = mat.m_cDiffuse * (L.m_cDiffuse * fDiffuseFactor);
		cSpecular = mat.m_cSpecular * (L.m_cSpecular * fSpecularFactor);
	}

	float fAttenuationFactor = 1.0f / dot(L.m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
	cDiffuse *= fAttenuationFactor;
	cSpecular *= fAttenuationFactor;
}

void ComputeSpotLight(MATERIAL mat, SPOT_LIGHT L, float3 vPosition, float3 vNormal, float3 vToCamera,
	out float4 cAmbient, out float4 cDiffuse, out float4 cSpecular)
{
	cDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	cSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 vToLight = L.m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance > L.m_fRange) return;

	cAmbient = mat.m_cAmbient * (L.m_cAmbient + gcLightGlobalAmbient);

	vToLight /= fDistance;
	float fDiffuseFactor = dot(vToLight, vNormal);
	if (fDiffuseFactor > 0.0f)
	{
		float3 vReflect = reflect(-vToLight, vNormal);
		float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0), mat.m_cSpecular.a);
		cDiffuse = mat.m_cDiffuse * (L.m_cDiffuse * fDiffuseFactor);
		cSpecular = mat.m_cSpecular * (L.m_cSpecular * fSpecularFactor);
	}

	float fSpotFactor = pow(max(dot(-vToLight, L.m_vDirection), 0), L.m_fFalloff);
	float fAttenuationFactor = fSpotFactor / dot(L.m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
	cAmbient *= fSpotFactor;
	cDiffuse *= fAttenuationFactor;
	cSpecular *= fAttenuationFactor;
}
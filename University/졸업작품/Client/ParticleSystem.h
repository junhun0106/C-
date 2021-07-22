#pragma once

#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE		1
#define PARTICLE_TYPE_SMOKE		2
#define PARTICLE_TYPE_BLOOD		3
#define PARTICLES				500

struct Particle 
{
	D3DXVECTOR3		InitialPos;
	D3DXVECTOR3		InitialVel;
	D3DXVECTOR2		Size;
	float			Age;
	unsigned int	Type;
};

struct ParticleInfo
{
	D3DXVECTOR3	EmitterPos;
	float		GameTime;
	D3DXVECTOR3 Acceleration;
	float		DeltaTime;
};

class ParticleSystem
{
public:
	ParticleSystem(); // ok
	~ParticleSystem();

	void CreateShaderVariables(ID3D11Device *pd3dDevice, D3DXVECTOR3 particlePos,
		D3D11_BLEND src_blend, D3D11_BLEND dest_blend, D3D11_BLEND_OP blend_op); // ok
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext); // ok
	void Initialize(ID3D11Device *pd3dDevice, ID3D11ShaderResourceView *pd3dsrvTexArray,
		ID3D11ShaderResourceView *pd3dsrvRandomTexture, UINT nMaxParticles); // ok
	void Update(float DeltaTime, float nCount); // ok
	void Render(ID3D11DeviceContext *pd3dDeviceContext); // ok
	void Reset(ID3D11DeviceContext *pd3dDeviceContext);
	void SetPosition(ID3D11Device *pd3dDevice, D3DXVECTOR3 pos);
	// ---
	void CreateShader(ID3D11Device *pd3dDevice, unsigned int type); // ok
	void CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName,
		LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputElements,
		UINT nElements, ID3D11InputLayout **ppd3dInputLayout); //ok
	void CreateSOGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel,
		ID3D11GeometryShader **ppd3dGeometryShader); // ok
	void CreateGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel,
		ID3D11GeometryShader **ppd3dGeometryShader); // ok
	void CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel,
		ID3D11PixelShader **ppd3dPixelShader); // ok

	// ---
	BYTE * ReadCompiledShaderCode(ID3D11Device *pd3dDevice, WCHAR *pszFileName, int& nSize);

	unsigned int	MaxParticles;
	bool			InitFlag;

	unsigned int particleID;

	float GameTime;
	float DeltaTime;
	D3DXVECTOR3 EmitterPos;
	D3DXVECTOR3 Acceleration;
	
	ID3D11Buffer*	CBforParticle; // set 16-Byte multiples

	ID3D11Buffer*	InitVB;
	ID3D11Buffer*	DrawVB;
	ID3D11Buffer*	StreamOutVB;

	ID3D11InputLayout*		InputLayout;
	ID3D11VertexShader*		VS;
	ID3D11VertexShader*		VSforSO;
	ID3D11GeometryShader*	GS;
	ID3D11GeometryShader*	GSforSO;
	ID3D11PixelShader*		PS;

	ID3D11DepthStencilState* SODepthStencilState;
	ID3D11DepthStencilState* DepthStencilState;
	ID3D11BlendState* BlendState;
	ID3D11RasterizerState* RasterizerState;

	ID3D11SamplerState *ParticleSampler;

	ID3D11ShaderResourceView* RandomTexSRV;
	ID3D11ShaderResourceView* ParticleTexSRV;
};

// 파티클 시스템에 표현할 느낌에 따라 블렌딩을 다르게 적용 해주어야 한다.
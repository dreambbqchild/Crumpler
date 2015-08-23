#pragma once
#include "D3D11.h"

__declspec(align(16)) class D3D11BasicShader
{
	friend class D3DWnd;
private:
	WVP wvp;
	static D3D11BasicShader* instance;

	Microsoft::WRL::ComPtr<ID3D11Device1> pD3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> pD3DContext;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResource;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;

public:
	static D3D11BasicShader* Singleton() { return instance; }
	static void Singleton(D3D11BasicShader* value) { if (!instance) { instance = value; }; }

	D3D11BasicShader(Microsoft::WRL::ComPtr<ID3D11Device1> &pD3DDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> &pD3DContext);
	void SetTexture(Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture);
	void UpdateModelMatrix(DirectX::XMMATRIX& model);
	void SetInContext();

	void SetWVP(WVP& value, WVP_MASK::Mask mask)
	{
		if (mask & WVP_MASK::World)
			wvp.world = value.world;

		if (mask & WVP_MASK::View)
			wvp.view = value.view;

		if (mask & WVP_MASK::Projection)
			wvp.projection = value.projection;
	}

	void* operator new(size_t i)
	{
		return _mm_malloc(i,16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
};

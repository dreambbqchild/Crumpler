#pragma once
#include <D3D11_1.h>
#include <DirectXMath.h>
#include <wrl.h>

#define CLEANUP_PTR(name) if(name) { delete name; name = nullptr; }

struct WVP
{
	DirectX::XMMATRIX  world;
	DirectX::XMMATRIX  view;
	DirectX::XMMATRIX  projection;
};

class WVP_MASK
{
public:
	enum Mask { World = (1 << 0), View = (1 << 1), Projection = (1 << 2) };
};

struct SHADER_INPUT
{
	SHADER_INPUT() : Position(0, 0, 0), Tex(0, 0) { }
	SHADER_INPUT(float x, float y, float z) : Position(x, y, z), Tex(0, 0) { }
	SHADER_INPUT(float x, float y, float z, float u, float v) : Position(x, y, z), Tex(u, v) {}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 Tex;
};

class ICreateTexture 
{
public:
	virtual Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Microsoft::WRL::ComPtr<ID3D11Device1>& pD3DDevice) = 0;
};
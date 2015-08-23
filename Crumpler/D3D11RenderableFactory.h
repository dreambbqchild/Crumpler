#pragma once
#include "D3D11.h"
#include "D3D11Plane.h"

class D3D11RenderableFactory {
private:
	SHADER_INPUT points[4];
	Microsoft::WRL::ComPtr<ID3D11Device1> pD3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext;

public:
	D3D11RenderableFactory(Microsoft::WRL::ComPtr<ID3D11Device1> &pD3DDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext1>& d3dContext) 
		: pD3DDevice(pD3DDevice), d3dContext(d3dContext) {}
	
	D3D11Plane* Plane() { return new D3D11Plane(pD3DDevice, d3dContext); }
	Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(ICreateTexture* source) { return source->CreateTexture(pD3DDevice); }
};
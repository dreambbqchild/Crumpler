#pragma once
#include "D3D11.h"
#include <vector>
#include <functional>

class D3D11Plane
{
private:
	static unsigned short indices[6];

	SHADER_INPUT points[4];
	Microsoft::WRL::ComPtr<ID3D11Device1> pD3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pointBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

public:
	D3D11Plane(Microsoft::WRL::ComPtr<ID3D11Device1> &pD3DDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext1>& d3dContext);
	void SetShaderInput(std::function<void(SHADER_INPUT[4])> setCallbackFn);
	void Draw(DirectX::XMMATRIX& location);
};
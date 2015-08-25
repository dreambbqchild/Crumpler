#include "D3D11BasicShader.h"
#include <fstream>
#include <functional>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

D3D11BasicShader* D3D11BasicShader::instance = nullptr;

void ReadFileAsBytes(const char* cStrFileName, function<void(BYTE*, size_t)> byteConsumerFn)
{
	ifstream infile;
	infile.open(cStrFileName, ios::binary | ios::ate);
	auto length = infile.tellg();
	infile.seekg(0, ios::beg);

	auto data = new char[(size_t)length];
	infile.read(data, length);
	infile.close();

	byteConsumerFn((BYTE*)data, (size_t)length);

	delete[] data;
}

D3D11BasicShader::D3D11BasicShader(ComPtr<ID3D11Device1> &pD3DDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> &pD3DContext)
	: pD3DDevice(pD3DDevice), pD3DContext(pD3DContext)
{
	D3D11_INPUT_ELEMENT_DESC basic[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	wvp.world = wvp.viewProjection = XMMatrixIdentity();

	ReadFileAsBytes("PixelShader.cso", [&](BYTE* data, size_t length)
	{
		pD3DDevice->CreatePixelShader(data, length, nullptr, &pPixelShader);
	});

	ReadFileAsBytes("VertexShader.cso", [&](BYTE* data, size_t length)
	{
		pD3DDevice->CreateVertexShader(data, length, nullptr, &pVertexShader);
		pD3DDevice->CreateInputLayout(basic, ARRAYSIZE(basic), data, length, &pInputLayout);
	});

	D3D11_BUFFER_DESC matrixBufferDesc = { 0 };
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(WVP);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	pD3DDevice->CreateBuffer(&matrixBufferDesc, NULL, &pMatrixBuffer);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	pD3DDevice->CreateSamplerState(&sampDesc, &pSamplerState);
}

void D3D11BasicShader::SetTexture(ComPtr<ID3D11Texture2D>& texture)
{
	pD3DDevice->CreateShaderResourceView(texture.Get(), nullptr, &pShaderResource);
}

void D3D11BasicShader::UpdateModelMatrix(XMMATRIX& model)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WVP* pMatrix;	
	wvp.world = XMMatrixTranspose(model);

	pD3DContext->Map(pMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pMatrix = (WVP*)mappedResource.pData;
	*pMatrix = wvp;
	pD3DContext->Unmap(pMatrixBuffer.Get(), 0);

	pD3DContext->VSSetConstantBuffers(0, 1, pMatrixBuffer.GetAddressOf());
}

void D3D11BasicShader::SetInContext() 
{
	pD3DContext->IASetInputLayout(pInputLayout.Get());
	pD3DContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	pD3DContext->PSSetShader(pPixelShader.Get(), nullptr, 0);

	if (pShaderResource)
	{
		pD3DContext->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());
		pD3DContext->PSSetShaderResources(0, 1, pShaderResource.GetAddressOf());
	}
}
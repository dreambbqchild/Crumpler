#include "D3D11Plane.h"
#include "D3D11BasicShader.h"
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

unsigned short D3D11Plane::indices[6] =
{
	0, 1, 2,
	0, 2, 3
};

UINT stride = sizeof(SHADER_INPUT);
UINT offset = 0;

template <typename T>
ComPtr<ID3D11Buffer> MakeBuffer(ComPtr<ID3D11Device1>& pD3DDevice, UINT byteWidth, T& content, UINT desc)
{
	ComPtr<ID3D11Buffer> buffer;
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = content;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.BindFlags = desc;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	pD3DDevice->CreateBuffer(&bufferDesc, &data, &buffer);

	return buffer;
}

D3D11Plane::D3D11Plane(ComPtr<ID3D11Device1> &pD3DDevice, ComPtr<ID3D11DeviceContext1>& d3dContext)
	: pD3DDevice(pD3DDevice), d3dContext(d3dContext)
{ }

void D3D11Plane::SetShaderInput(function<void(SHADER_INPUT[4])> setCallbackFn) 
{
	setCallbackFn(points);

	XMStoreFloat3(&points[0].Normal, XMVector3Cross(XMLoadFloat3(&points[0].Position), XMLoadFloat3(&points[1].Position)));
	XMStoreFloat3(&points[1].Normal, XMVector3Cross(XMLoadFloat3(&points[1].Position), XMLoadFloat3(&points[2].Position)));
	XMStoreFloat3(&points[2].Normal, XMVector3Cross(XMLoadFloat3(&points[2].Position), XMLoadFloat3(&points[3].Position)));
	XMStoreFloat3(&points[3].Normal, XMVector3Cross(XMLoadFloat3(&points[3].Position), XMLoadFloat3(&points[0].Position)));

	pointBuffer = MakeBuffer(pD3DDevice, sizeof(SHADER_INPUT) * 4, points, D3D11_BIND_VERTEX_BUFFER);
	indexBuffer = MakeBuffer(pD3DDevice, sizeof(indices), indices, D3D11_BIND_INDEX_BUFFER);
}

void D3D11Plane::Draw(XMMATRIX& location)
{
	d3dContext->IASetVertexBuffers(0, 1, pointBuffer.GetAddressOf(), &stride, &offset);
	d3dContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11BasicShader::Singleton()->UpdateModelMatrix(location);

	d3dContext->DrawIndexed(6, 0, 0);
}
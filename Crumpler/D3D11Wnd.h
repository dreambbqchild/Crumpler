#pragma once
#include "D3D11.h"
#include "D3D11RenderableFactory.h"
#include "D3D11BasicShader.h"

typedef void (*D3DDraw)();

class D3D11Wnd
{
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND hWnd;
	SIZE clientSize;
	Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3dRenderTargetView;
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;

	D3D11RenderableFactory* pRenderableFactory;

	void InitD3D();

public:
	D3D11Wnd(SIZE& clientSize, HWND hWndParent, HINSTANCE hInst);
	SIZE RenderAreaSize() { return clientSize; }
	void Resize();
	void Clear();
	void Present();
	void IsVisible(bool value) { ShowWindow(hWnd, value ? SW_SHOW : SW_HIDE); }
	D3DDraw Draw;

	D3D11RenderableFactory* Factory() { return pRenderableFactory; }

	virtual ~D3D11Wnd();
};
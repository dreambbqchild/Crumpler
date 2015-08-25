#include "D3D11Wnd.h"
#include "D3D11BasicShader.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
D3D_FEATURE_LEVEL featureLevels[] =
{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

auto degrees = 45.0f;
auto lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

LRESULT CALLBACK D3D11Wnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D3D11Wnd* pD3DWnd = (D3D11Wnd*)GetWindowLongPtr(hWnd, GWL_USERDATA);
	
	if (message == WM_CREATE || pD3DWnd) 
	{
		switch (message)
		{
		case WM_CREATE:
			pD3DWnd = (D3D11Wnd*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pD3DWnd);
			return 0;
		case WM_SIZE:
			pD3DWnd->hWnd = hWnd;
			pD3DWnd->Resize();
			return 0;
		case WM_PAINT:
			pD3DWnd->Clear();
			if (pD3DWnd->Draw)
				pD3DWnd->Draw();

			pD3DWnd->Present();
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void RegisterWindowClass(WNDPROC wndProc, HINSTANCE hInst)
{
	static bool registered = false;
	if (registered)
		return;

	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"D3DWnd";

	RegisterClassEx(&wcex);
	registered = true;
}

D3D11Wnd::D3D11Wnd(SIZE& clientSize, HWND hWndParent, HINSTANCE hInst)
	: Draw(nullptr), clientSize(clientSize)
{
	RegisterWindowClass(WndProc, hInst);
	InitD3D();

	auto style = WS_VISIBLE | WS_CHILD | WS_BORDER;
	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"D3DWnd", L"D3DWnd", style, 0, 0, clientSize.cx, clientSize.cy, hWndParent, NULL, hInst, this);
}

void D3D11Wnd::InitD3D()
{
	ComPtr<ID3D11Device> deviceKickstart;
	ComPtr<ID3D11DeviceContext> contextKickstart;
	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGIAdapter> dxgiAdapter;
	D3D_FEATURE_LEVEL featureLevel;

	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &deviceKickstart, &featureLevel, &contextKickstart);
	deviceKickstart.As(&d3dDevice);
	contextKickstart.As(&d3dContext);

	d3dDevice.As(&dxgiDevice);
	dxgiDevice->GetAdapter(&dxgiAdapter);
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

	D3D11_RASTERIZER_DESC renderBothSides;
	ComPtr<ID3D11RasterizerState> pRenderBothSides;
	ZeroMemory(&renderBothSides, sizeof(D3D11_RASTERIZER_DESC));
	renderBothSides.FillMode = D3D11_FILL_SOLID;
	renderBothSides.CullMode = D3D11_CULL_NONE;
	renderBothSides.FrontCounterClockwise = FALSE;
	renderBothSides.DepthBiasClamp = 0.0f;
	renderBothSides.DepthClipEnable = TRUE;
	renderBothSides.ScissorEnable = FALSE;
	renderBothSides.MultisampleEnable = FALSE;
	renderBothSides.AntialiasedLineEnable = FALSE;
	d3dDevice->CreateRasterizerState(&renderBothSides, &pRenderBothSides);
	d3dContext->RSSetState(pRenderBothSides.Get());

	D3D11BasicShader::Singleton(new D3D11BasicShader(d3dDevice, d3dContext));
	pRenderableFactory = new D3D11RenderableFactory(d3dDevice, d3dContext);
}

void D3D11Wnd::Resize()
{
	if (!clientSize.cx || !clientSize.cy)
		return;

	ComPtr<ID3D11Texture2D> d3dBackBuffer;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };	
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (pSwapChain)
	{
		d3dContext->OMSetRenderTargets(0, 0, 0);
		d3dRenderTargetView = nullptr;
		pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	}
	else
	{
		dxgiFactory->CreateSwapChainForHwnd(d3dDevice.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &pSwapChain);
	}

	D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
	pSwapChain->GetBuffer(0, IID_PPV_ARGS(&d3dBackBuffer));
	d3dBackBuffer->GetDesc(&backBufferDesc);
	d3dDevice->CreateRenderTargetView(d3dBackBuffer.Get(), nullptr, &d3dRenderTargetView);

	CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(backBufferDesc.Width), static_cast<float>(backBufferDesc.Height));
	d3dContext->RSSetViewports(1, &viewport);

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = clientSize.cx;
	descDepth.Height = clientSize.cy;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	d3dDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	d3dDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDepthStencilView);

	auto aspect = (clientSize.cx / (float)clientSize.cy);
	auto adjacent = (-clientSize.cx * 0.5f) / tan(XMConvertToRadians(degrees * 0.5f));
	auto eye = XMVectorSet(0.0f, 0.0f, (adjacent / aspect), 0.0f);

	auto view = XMMatrixLookAtLH(eye, lookAt, up);
	auto projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(degrees), aspect, 0.01f, 25000.01f);

	D3D11BasicShader::Singleton()->SetViewProjection(view * projection);
}

void D3D11Wnd::Clear()
{
	float color[4] = { 0 };
	color[3] = 1.0f;
	d3dContext->ClearRenderTargetView(d3dRenderTargetView.Get(), color);
	d3dContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	d3dContext->OMSetRenderTargets(1, d3dRenderTargetView.GetAddressOf(), pDepthStencilView.Get());

	D3D11BasicShader::Singleton()->SetInContext();
}

void D3D11Wnd::Present()
{
	if(pSwapChain)
		pSwapChain->Present(1, 0);
}

D3D11Wnd::~D3D11Wnd()
{
	CLEANUP_PTR(pRenderableFactory)
}
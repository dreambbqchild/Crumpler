#include "RichEditWnd.h"
#include <Richedit.h>
#include <Richole.h>

#pragma comment(lib, "comsuppw.lib")

using namespace Microsoft::WRL;

_bstr_t space(" ");

HWND CreateRichEdit(SIZE& clientSize, HWND hwndOwner, HINSTANCE hinst)
{
    LoadLibrary(TEXT("Msftedit.dll"));
	auto style = ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP;
	return CreateWindowEx(0, MSFTEDIT_CLASS, TEXT("Type here"), style, 0, 0, clientSize.cx, clientSize.cy, hwndOwner, NULL, hinst, NULL);
}

RichEditWnd::RichEditWnd(SIZE& clientSize, HWND hwndParent, HINSTANCE hinst)
	: textBuffer(), hBitmap(nullptr)
{
	hWndRich = CreateRichEdit(clientSize, hwndParent, hinst);
	if(hWndRich)
	{
		IRichEditOle* pOle;
		SendMessage(hWndRich, EM_GETOLEINTERFACE, NULL, LPARAM(&pOle));
		pOle->QueryInterface(__uuidof(ITextDocument), &document);
		pOle->Release();
	}
}

ComPtr<ID3D11Texture2D> RichEditWnd::CreateTexture(ComPtr<ID3D11Device1>& pD3DDevice)
{
	ComPtr<ID3D11Texture2D> d3dTexture;

	RECT clientBounds;
	GetClientRect(hWndRich, &clientBounds);
	int width = clientBounds.right - clientBounds.left;
	int height = clientBounds.bottom - clientBounds.top;

	HDC hWndRichDC = GetDC(hWndRich);
	HDC hCaptureDC = CreateCompatibleDC(hWndRichDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hWndRichDC, width, height);
	SelectObject(hCaptureDC, hBitmap);
	FillRect(hCaptureDC, &clientBounds, (HBRUSH)(COLOR_WINDOW + 1));

	FORMATRANGE fr;
	fr.hdc = hCaptureDC;
	fr.hdcTarget = hCaptureDC;

	fr.rcPage.top = 0;
	fr.rcPage.left = 0;
	fr.rcPage.right = MulDiv(width, 1440, GetDeviceCaps(hCaptureDC, LOGPIXELSX));
	fr.rcPage.bottom = MulDiv(height, 1440, GetDeviceCaps(hCaptureDC, LOGPIXELSY));

	fr.rc.left = 0;
	fr.rc.top = 0;
	fr.rc.right = fr.rcPage.right;
	fr.rc.bottom = fr.rcPage.bottom;

	SendMessage(hWndRich, EM_SETSEL, 0, (LPARAM)-1);
	SendMessage(hWndRich, EM_EXGETSEL, 0, (LPARAM)&fr.chrg);
	SendMessage(hWndRich, EM_FORMATRANGE, TRUE, (LPARAM)&fr);
	SendMessage(hWndRich, EM_SETSEL, 0, (LPARAM)0);

	BITMAPINFOHEADER bmih;
	ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biWidth = width;
	bmih.biHeight = -height;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;

	BYTE *pixels = (BYTE*)malloc(4 * width * height);

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader = bmih;

	GetDIBits(hCaptureDC, hBitmap, 0, height, pixels, &bmi, DIB_RGB_COLORS);

	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = pixels; 
	data.SysMemPitch = 4 * width;
	data.SysMemSlicePitch = 4 * width * height;

	pD3DDevice->CreateTexture2D(&texDesc, &data, &d3dTexture);

	ReleaseDC(hWndRich, hWndRichDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hBitmap);
	free(pixels);

	return d3dTexture;
}

_bstr_t RichEditWnd::SelectText()
{
	document->Range(0, LONG_MAX, &range); 
	range->GetText(&textBuffer.GetBSTR());
	return textBuffer;
}

void RichEditWnd::BlankOutCharacterAtForCurrentlySelectedText(long index)
{
	HRESULT hr = range->SetStart(index);
	hr = range->SetEnd(index + 1);
	hr = range->SetText(space);
}

RichEditWnd::~RichEditWnd()
{
	if(hWndRich)
	{
		DestroyWindow(hWndRich);
		hWndRich = nullptr;
	}
}
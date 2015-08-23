#pragma once
#include <Windows.h>
#include <wrl.h>
#include <tom.h>
#include <comutil.h>
#include "D3D11.h"

class RichEditWnd : public ICreateTexture
{
private:
	HWND hWndRich;
	HBITMAP hBitmap;
	Microsoft::WRL::ComPtr<ITextDocument> document;
	Microsoft::WRL::ComPtr<ITextRange> range;
	_bstr_t textBuffer;

public:
	RichEditWnd(SIZE& clientSize, HWND hWndParent, HINSTANCE hinst);

	bool GetHasHandle() {return hWndRich != nullptr;}
	void IsVisible(bool value) { ShowWindow(hWndRich, value ? SW_SHOW : SW_HIDE); }
	Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Microsoft::WRL::ComPtr<ID3D11Device1>& pD3DDevice);
	_bstr_t SelectText();
	void BlankOutCharacterAtForCurrentlySelectedText(long index);

	virtual ~RichEditWnd();
};
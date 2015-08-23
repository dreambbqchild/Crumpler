#include <Windows.h>
#include <time.h> 
#include "WindowSize.h"
#include "RichEditWnd.h"
#include "D3D11Wnd.h"
#include "D3D11AccordionFold.h"
#include "resource.h"
#define MAX_LOADSTRING 100

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
RichEditWnd* pRichEditWnd = nullptr;
D3D11Wnd* pD3DWnd = nullptr;
D3D11AccordionFold* accordion = nullptr;
HWND hwndButton = nullptr;

bool animationRunning = false;
float percentage = 0.0f;
bool needNewImage = false;
DWORD animationStart = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		auto wmId = LOWORD(wParam);

		if (HIWORD(wParam) == BN_CLICKED)
		{			
			D3D11BasicShader::Singleton()->SetTexture(pD3DWnd->Factory()->CreateTexture(pRichEditWnd));
			srand((UINT)time(nullptr));
			auto bStr = pRichEditWnd->SelectText();
			const wchar_t* cStr = bStr;
			for (UINT i = 0; i < bStr.length(); i++)
			{
				wchar_t c = cStr[i];
				if (c < 255 && !isspace(c) && (rand() % 100) < 33)
					pRichEditWnd->BlankOutCharacterAtForCurrentlySelectedText(i);
			}

			pD3DWnd->IsVisible(true);
			pRichEditWnd->IsVisible(false);			

			needNewImage = true;
			animationRunning = true;
			animationStart = GetTickCount();
			break;
		}
	}
	case WM_SIZE:
	{
		accordion->Resize();
		accordion->PercentToClosed(percentage);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1024, 768 + 50, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	auto clientSize = GetSize(GetClientRect, hWnd);
	clientSize.cy -= 50;

	pRichEditWnd = new RichEditWnd(clientSize, hWnd, hInstance);
	pRichEditWnd->IsVisible(true);

	pD3DWnd = new D3D11Wnd(clientSize, hWnd, hInstance);
	pD3DWnd->IsVisible(false);
	pD3DWnd->Draw = []() 
	{
		const float animationLength = 1500.0f;
		if (!animationRunning) 
			return;

		DWORD now = GetTickCount();
		float diff = (float)now - animationStart;

		if (diff >= animationLength * 0.5f && needNewImage)
		{
			D3D11BasicShader::Singleton()->SetTexture(pD3DWnd->Factory()->CreateTexture(pRichEditWnd));
			needNewImage = false;
		}

		percentage = max(0.0f, sin(DirectX::XM_PI * (diff / animationLength)));
		
		accordion->PercentToClosed(percentage);
		accordion->Draw();

		if (diff >= animationLength)
		{
			pRichEditWnd->IsVisible(true);
			pD3DWnd->IsVisible(false);
			animationRunning = false;
		}	
	};
	
	accordion = new D3D11AccordionFold(pD3DWnd, 20);

	hwndButton = CreateWindow(L"BUTTON", L"Crumple!", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, clientSize.cy, clientSize.cx, 50, hWnd, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

ATOM RegisterClass(HINSTANCE hInstance) 
{
	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
//	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CODESMASH));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wcex.lpszClassName = szWindowClass;
//	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CRUMPLER, szWindowClass, MAX_LOADSTRING);

	RegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete pRichEditWnd;
	delete pD3DWnd;
	delete accordion;

	CoUninitialize();
	return 0;
}
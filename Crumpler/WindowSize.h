#pragma once
typedef BOOL(__stdcall *GetRect)(HWND, LPRECT);

SIZE GetSize(GetRect getRect, HWND hWnd)
{
	SIZE result = { 0 };
	RECT rect;
	if (getRect(hWnd, &rect))
	{
		result.cx = rect.right - rect.left;
		result.cy = rect.bottom - rect.top;
	}

	return result;
}

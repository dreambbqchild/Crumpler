#pragma once
#include "D3D11Wnd.h"

class D3D11AccordionFold 
{
private:
	UINT pairs;
	UINT segments;
	float totalWidth;
	float segmentWidth;
	float left;
	D3D11Wnd* d3dWnd;
	D3D11Plane** planes;
	DirectX::XMMATRIX* transformationMatrices;

public:
	D3D11AccordionFold(D3D11Wnd* d3dWnd, UINT pairs);

	void Resize();
	void Draw();
	void PercentToClosed(float percent);

	virtual ~D3D11AccordionFold();
};
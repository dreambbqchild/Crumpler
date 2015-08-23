#pragma once
#include "D3D11AccordionFold.h"

using namespace DirectX;

D3D11AccordionFold::D3D11AccordionFold(D3D11Wnd* d3dWnd, UINT pairs)
	: d3dWnd(d3dWnd), pairs(pairs), segments(pairs * 2), segmentWidth(0), left(0)
{
	planes = new D3D11Plane*[segments];
	for (UINT i = 0; i < segments; i++)
		planes[i] = d3dWnd->Factory()->Plane();

	transformationMatrices = (XMMATRIX*)_mm_malloc(segments * sizeof(XMMATRIX), 16);
}

void D3D11AccordionFold::Resize()
{
	if (segments <= 0)
		return;
	auto size = d3dWnd->RenderAreaSize();
	auto halfWidth = (size.cx * 0.5f) / segments;
	auto halfHeight = size.cy * 0.5f;

	for (UINT i = 0; i < segments; i++)
	{
		auto low = i / (float)segments;
		auto high = (i + 1) / (float)segments;
		planes[i]->SetShaderInput([halfWidth, halfHeight, low, high](SHADER_INPUT points[4]) {
			points[0] = SHADER_INPUT(-halfWidth, halfHeight, 0.0f, low, 0.0f);
			points[1] = SHADER_INPUT(halfWidth, halfHeight, 0.0f, high, 0.0f);
			points[2] = SHADER_INPUT(halfWidth, -halfHeight, 0.0f, high, 1.0f);
			points[3] = SHADER_INPUT(-halfWidth, -halfHeight, 0.0f, low, 1.0f);
		});
	}

	left = -(size.cx * 0.5f) + halfWidth;
	totalWidth = (float)size.cx;
	segmentWidth = halfWidth * 2;
}

void D3D11AccordionFold::PercentToClosed(float percent) //100% = closed
{
	const float HALF_XM_PI = XM_PI * 0.5f;
	if (totalWidth <= 0)
		return;

	float junk = 0;
	UINT movingParing = (UINT)(pairs * (1.0f - percent));
	float percentageToTraverseOnePair = (segmentWidth  * 2) / totalWidth;
	float percentOfMovingPairComplete = 1.0f - modff((1 - percent) / percentageToTraverseOnePair, &junk);
	float lastPosition = 0;

	for (UINT i = 0; i < pairs; i++)
	{
		auto firstSegment = i * 2;
		auto leftSideOfPair = left + segmentWidth * firstSegment;

		if (i > movingParing)
		{
			transformationMatrices[firstSegment] = XMMatrixTranslation(segmentWidth * 0.5f, 0.0f, 0.0f) * XMMatrixRotationY(-HALF_XM_PI) * XMMatrixTranslation(-segmentWidth * 0.5f + lastPosition, 0.0f, 0.0f);
			transformationMatrices[firstSegment + 1] = XMMatrixTranslation(-segmentWidth * 0.5f, 0.0f, 0.0f) * XMMatrixRotationY(HALF_XM_PI) * XMMatrixTranslation(-segmentWidth * 0.5f + lastPosition, 0.0f, 0.0f);
		}
		else if (i < movingParing) 
		{
			transformationMatrices[firstSegment] = XMMatrixTranslation(leftSideOfPair, 0.0f, 0.0f);
			transformationMatrices[firstSegment + 1] = XMMatrixTranslation(left + segmentWidth * (firstSegment + 1), 0.0f, 0.0f);
		}
		else
		{
			auto angle = HALF_XM_PI * percentOfMovingPairComplete;
			auto position = cos(angle) * segmentWidth * 2;
			lastPosition = leftSideOfPair + position;
			transformationMatrices[firstSegment] = XMMatrixTranslation(segmentWidth * 0.5f, 0.0f, 0.0f) * XMMatrixRotationY(-angle) * XMMatrixTranslation(-segmentWidth * 0.5f + leftSideOfPair, 0.0f, 0.0f);
			transformationMatrices[firstSegment + 1] = XMMatrixTranslation(-segmentWidth * 0.5f, 0.0f, 0.0f) * XMMatrixRotationY(angle) * XMMatrixTranslation(-segmentWidth * 0.5f + lastPosition, 0.0f, 0.0f);
		}
	}
}

void D3D11AccordionFold::Draw() 
{
	for (UINT i = 0; i < segments; i++)
	{
		planes[i]->Draw(transformationMatrices[i]);
	}
}

D3D11AccordionFold::~D3D11AccordionFold()
{
	for (UINT i = 0; i < segments; i++)
	{
		delete planes[i];
	}

	delete[] planes;
	_mm_free(transformationMatrices);
}
cbuffer WorldViewProjectionConstantBuffer : register(b0)
{
	matrix WVP;
};

struct VS_INPUT
{
	float3 vPos   : POSITION;
	float2 Tex    : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION; // Vertex shaders must output SV_POSITION
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input) // main is the default function name
{
	VS_OUTPUT Output;

	float4 pos = float4(input.vPos, 1.0f);

	Output.Position = mul(pos, WVP);
	Output.Tex = input.Tex;

	return Output;
}

cbuffer WorldViewProjectionConstantBuffer : register(b0)
{
	matrix World;
	matrix ViewProjection;
};

struct VS_INPUT
{
	float3 Position   : POSITION;
	float2 Tex    : TEXCOORD0;
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT Output;

	float4 pos = float4(input.Position, 1.0f);

	Output.Position = mul(mul(pos, World), ViewProjection);
	Output.Tex = input.Tex;
	Output.Normal = mul(input.Normal, (float3x3)World);

	return Output;
}

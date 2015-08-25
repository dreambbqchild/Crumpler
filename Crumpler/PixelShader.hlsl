Texture2D shaderTexture;
SamplerState SampleType;

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};

struct PS_OUTPUT
{
	float4 RGBColor : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT Output;
	float4 diffuse = shaderTexture.Sample(SampleType, In.Tex);
	Output.RGBColor = diffuse * saturate(dot(float3(0.25, 0.25, -1.0), normalize(In.Normal)));

	return Output;
}
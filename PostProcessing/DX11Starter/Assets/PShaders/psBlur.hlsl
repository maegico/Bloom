cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount;
}

struct VertexToPixel
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD0;
};

Texture2D InitialRender	: register(t0);
SamplerState Sampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 totalColor = float4(0,0,0,0);
	uint numSamples = 0;

	for (int y = -blurAmount; y < blurAmount; y++)
	{
		for (int x = -blurAmount; x < blurAmount; x++)
		{
			//figure out this pixel's uv
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);

			totalColor += InitialRender.Sample(Sampler, uv);
			numSamples++;
		}
	}

	return totalColor / numSamples;
}
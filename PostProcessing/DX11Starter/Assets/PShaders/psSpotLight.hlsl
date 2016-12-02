struct SpotLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Position;
	float DiffuseIntensity;
	float3 Direction;
};

//make arrays in the constant buffer
//make a max size for the arrays
//make a parameterized for loop for lighting
//on CPU side

cbuffer lights : register(b0)
{
	SpotLight spotLight;
};

Texture2D diffuseTexture : register(t0);
SamplerState basicSampler : register(s0);

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float4 worldPos		: POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

//Nice read!
//http://gamedev.stackexchange.com/questions/56897/glsl-light-attenuation-color-and-intensity-formula
float4 calcSpotLight(float4 worldPos, float3 normal, SpotLight spotLight)
{
	//multiply attenuation by intensity
	//attenuation function: 1.0 / (1.0 + a*dist + b*dist*dist))
	/*float a = 5;
	float b = 4;*/

	//normalize(input.worldPos - (-dirLight.Direction)) <-- actually wrong(most likely)
	float3 spotLightDirectionToPixel = worldPos - spotLight.Position;
	//float dist = sqrt(dot(pointLightDirection, pointLightDirection));
	//dist = max(dist, 9);
	//float attenuation = 1.0 / (1.0 + a*dist + b*dist*dist);
	spotLightDirectionToPixel = -normalize(spotLightDirectionToPixel);

	//float temp = attenuation * pointLight.Intensity;

	//N dot L
	float lightAmount = dot(normal, spotLightDirectionToPixel);
	lightAmount = saturate(lightAmount);

	//why 
	float angleFromCenter = max( 0.5f, dot(spotLightDirectionToPixel, spotLight.Direction) );
	//raise to a power for a nice "falloff"
	//multiply diffuse and specular results by this
	float spotAmount = pow(angleFromCenter, 0.7f);
	//float spotAmount = smoothstep(100.0f, 100.0f, angleFromCenter);
	//float spotAmount = 1.0f;

	float4 scaledDiffuse = spotLight.DiffuseColor * lightAmount * spotAmount;

	
	//return float4(spotAmount, 0, 0, 0);
	return scaledDiffuse + spotLight.AmbientColor;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	float4 spotL = calcSpotLight(input.worldPos, input.normal, spotLight);

	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	return (spotL)* surfaceColor;
	//return spotL;
}
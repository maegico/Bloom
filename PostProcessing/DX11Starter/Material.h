#pragma once
#include "SimpleShader.h"
#include "Shaders.h"

//represents a set of vertex and pixel shaders

class Material
{
public:
	// Default Constructor that does nothing
	// @return nothing
	//
	Material();
	Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, ID3D11ShaderResourceView* shaderResView, ID3D11ShaderResourceView* normalMap, ID3D11SamplerState* sampler);
	~Material();

	SimpleVertexShader* getVShader();
	SimplePixelShader* getPShader();

	ID3D11ShaderResourceView* getShaderResView();
	ID3D11ShaderResourceView* getNormalMap();
	ID3D11SamplerState* getSampler();
	void ReleaseShaderResView();
	void ReleaseSampler();

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11ShaderResourceView* shaderResView;
	ID3D11ShaderResourceView* normalMap;
	ID3D11SamplerState* sampler;
};
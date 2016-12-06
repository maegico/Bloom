#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include "Entity.h"
#include "Camera.h"
#include "Lights.h"
#include "ContentManager.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:
	// The matrices to go from model space to screen space

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	DirectionalLight dirLight1;
	DirectionalLight dirLight2;
	DirectionalLight dirLight3;
	PointLight pointLight;
	SpotLight spotLight;
	Lights lights;

	//Skyboxing
	ID3D11RasterizerState* skyRastState;
	ID3D11DepthStencilState* skyDepthState;

	// Post process requirements
	//initial render
	ID3D11RenderTargetView* irRTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* irSRV;	// Allows us to sample from the same texture

	//bright pixels
	ID3D11RenderTargetView* bpRTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* bpSRV;	// Allows us to sample from the same texture

	//blur
	ID3D11RenderTargetView* blRTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* blSRV;	// Allows us to sample from the same texture

	ContentManager* cmanager;
	std::vector<Entity*>  entities;
	Camera* camera;
	bool moving = false;
};


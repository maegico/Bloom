#include "Game.h"
#include "Vertex.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore( 
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	delete camera;

	for (int i = 0; i < entities.size(); i++)
		delete entities[i];

	skyDepthState->Release();
	skyRastState->Release();

	delete cmanager;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	/*dirLight2 = { XMFLOAT4(0.5f, 1.0f, 0.32f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, 0.0f) };

	dirLight3 = { XMFLOAT4(0.5f, 1.0f, 0.32f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f) };*/

	dirLight1 = { XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
				XMFLOAT3(1.0f, -1.0f, 0.0f) };

	pointLight = { XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
				XMFLOAT3(1.0f, -1.0f, 0.0f),
				2.0f };

	spotLight = { XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
				XMFLOAT3(1.0f, 0.0f, 0.0f),
				2.0f,
				XMFLOAT3(-1.0f, 0.0f, 0.0f) };

	lights = { { spotLight },
			{ pointLight },
			dirLight1,
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
			1,
			1 };

	camera = new Camera();

	cmanager = new ContentManager();
	cmanager->Init(device, context);
	Material* brick = cmanager->LoadMaterialWithNormalMap("brickLightingNormalMap", "sampler", "vsLighting.cso", "psLighting.cso", "bricks.png", "bricksNM.png");
	Material* skybox = cmanager->LoadCubeMapMaterial("skyMap", "sampler", "SkyVS.cso", "SkyPS.cso", "SunnyCubeMap.dds");
	Material* bloom = cmanager->LoadPostProcessingMaterial("Blur", "sampler", "vsBlur.cso", "psBlur.cso");

	entities = {
		new Entity(context, cmanager->GetMesh("cube.obj"), brick, { 0.5, 0.5, 0 }, 0),
		new Entity(context, cmanager->GetMesh("cone.obj"), brick, { 0, 0, 0 }, 0),
		new Entity(context, cmanager->GetMesh("helix.obj"), brick, { -1.0f, 0, 0 }, 0),
		new Entity(context, cmanager->GetMesh("cube.obj"), brick, { 0.0f, -0.5f, 0 }, 0),
		new Entity(context, cmanager->GetMesh("torus.obj"), brick, { 1.0f, -1.0f, 0 }, 0) };

	camera->updateProjection(width, height);

	// Create a rasterizer state so we can render backfaces
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &skyRastState);

	// Create a depth state so that we can accept pixels
	// at a depth less than or EQUAL TO an existing depth
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // Make sure we can see the sky (at max depth)
	device->CreateDepthStencilState(&dsDesc, &skyDepthState);

	//Post process resources
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;


	ID3D11Texture2D* initialRender;
	ID3D11Texture2D* brightPixels;
	device->CreateTexture2D(&textureDesc, 0, &initialRender);
	device->CreateTexture2D(&textureDesc, 0, &brightPixels);

	device->CreateRenderTargetView(initialRender, &rtvDesc, &irRTV);
	device->CreateRenderTargetView(brightPixels, &rtvDesc, &bpRTV);

	device->CreateShaderResourceView(initialRender, &srvDesc, &irSRV);
	device->CreateShaderResourceView(brightPixels, &srvDesc, &bpSRV);

	initialRender->Release();
	brightPixels->Release();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	camera->updateProjection(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	camera->update(deltaTime);

	for (int i = 0; i < entities.size(); i++)
	{
		//entities[i]->move({ 0.25f * deltaTime, 0.0f * deltaTime, 0.0f * deltaTime });
		entities[i]->update(deltaTime);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTiame, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	//const float color[4] = {0.4f, 0.6f, 0.75f, 0.0f};
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(irRTV, color);
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->OMSetRenderTargets(1, &irRTV, depthStencilView);

	//// Send data to shader variables
	////  - Do this ONCE PER OBJECT you're drawing
	////  - This is actually a complex process of copying data to a local buffer
	////    and then copying that entire buffer to the GPU.  
	////  - The "SimpleShader" class handles all of that for you.
	
	Entity* entity = entities[0];
	Material* mat = entity->getMat();
	ID3D11SamplerState* sampler = mat->getSampler();
	ID3D11ShaderResourceView* shaderResView = mat->getShaderResView();
	ID3D11ShaderResourceView* normalMap = mat->getNormalMap();
	SimplePixelShader* ps = mat->getPShader();

	mat->getVShader()->SetMatrix4x4("view", camera->getViewMatrix());
	mat->getVShader()->SetMatrix4x4("projection", camera->getProjectionMatrix());
	ps->SetData("lights", &lights, sizeof(Lights));
	ps->SetSamplerState("Sampler", sampler);
	ps->SetShaderResourceView("Texture", shaderResView);
	ps->SetShaderResourceView("NormalMap", normalMap);

	mat->ReleaseSampler();
	mat->ReleaseShaderResView();
	normalMap->Release();

	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->draw();
	}

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* skyVB = entities[0]->getMesh()->GetVertexBuffer();
	ID3D11Buffer* skyIB = entities[0]->getMesh()->GetIndexBuffer();
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	Material* skyMap = cmanager->GetMaterial("skyMap");
	SimplePixelShader* skyPS = skyMap->getPShader();
	SimpleVertexShader* skyVS = skyMap->getVShader();
	ID3D11ShaderResourceView* skySRV = skyMap->getShaderResView();

	// Set up shaders
	skyVS->SetMatrix4x4("view", camera->getViewMatrix());
	skyVS->SetMatrix4x4("projection", camera->getProjectionMatrix());
	skyVS->CopyAllBufferData();
	skyVS->SetShader();

	skyPS->SetShaderResourceView("Sky", skySRV);
	skyPS->CopyAllBufferData();
	skyPS->SetShader();

	// Set the proper render states
	context->RSSetState(skyRastState);
	context->OMSetDepthStencilState(skyDepthState, 0);

	// Actually draw
	context->DrawIndexed(entities[0]->getMesh()->GetIndexCount(), 0, 0);

	// Reset the states!
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

	//Post Processing
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	//Set the Shaders
	//Send data to the Shaders
	//copy all the buffer data
	Material* blurMat = cmanager->GetMaterial("Blur");
	SimplePixelShader* blurPS = blurMat->getPShader();
	
	blurMat->getVShader()->SetShader();

	blurPS->SetShader();
	blurPS->SetShaderResourceView("InitialRender", irSRV);
	blurPS->SetSamplerState("Sampler", sampler);
	blurPS->SetInt("blurAmount", 9);
	blurPS->SetFloat("pixelWidth", 1.0f / width);
	blurPS->SetFloat("pixelHeight", 1.0f / height);
	blurPS->CopyAllBufferData();
	
	//then draw
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	////draw the triangle that encompasses the whole screen
	context->Draw(3, 0);

	blurPS->SetShaderResourceView("InitialRender", 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	moving = true;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	moving = false;

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (moving)
	{
		// Add any custom code here...
		camera->addRotX(prevMousePos.y - y);
		camera->addRotY(prevMousePos.x - x);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion
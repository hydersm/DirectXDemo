#include "pch.h"
#include "Game.h"
#include <fstream>

using namespace std;
//this function initializes direct3d for use
void CGame::Initialize()
{
	// Define temporary pointers to a device and a device context
	ComPtr<ID3D11Device> dev11;
	ComPtr<ID3D11DeviceContext> devcon11;
		
	// Create the device and device context objects
	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&dev11,
		nullptr,
		&devcon11);

	// Convert the pointers from the DirectX 11 versions to the DirectX 11.1 versions
	dev11.As(&dev);
	devcon11.As(&devcon);
	
	//this part is for getting the dxgi factory
	//convert the device to dxgidevice
	ComPtr<IDXGIDevice1> dxgiDevice;
	dev.As(&dxgiDevice);

	//get the dxgiAdapter (its a representation of the gpu)
	ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	//get the dxgiFactory (the thing that create the device and device context objects)
	ComPtr<IDXGIFactory2> dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);

	//swap chan desc struct
	DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // how the swap chain should be used
	scd.BufferCount = 2;                                  // a front buffer and a back buffer
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;              // the most common swap chain format
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;    // the recommended flip mode
	scd.SampleDesc.Count = 1;                             // disable anti-aliasing

	CoreWindow^ Window = CoreWindow::GetForCurrentThread();    // get a pointer to the window

	//use dxgifactory to create the swapchain
	dxgiFactory->CreateSwapChainForCoreWindow(
		dev.Get(),                                  // address of the device
		reinterpret_cast<IUnknown*>(Window),        // address of the window
		&scd,                                       // address of the swap chain description
		nullptr,                                    // advanced
		&swapchain);                                // address of the new swap chain pointer

	//get a pointer to the back buffer
	ComPtr<ID3D11Texture2D> backbuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);

	//create the render target
	dev->CreateRenderTargetView(backbuffer.Get(), nullptr, &renderTarget);

	// set the viewport
	D3D11_VIEWPORT viewport = { 0 };

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	devcon->RSSetViewports(1, &viewport);

	InitGraphics();
	InitPipeline();
	InitStates();

	time = 0;
}

//this function updates the state of the game
void CGame::Update()
{
	time += 0.05f;
}

//this function renders a single frame of 3d graphics
void CGame::Render()
{
	//clear backbuffer to grey
	float colour[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(renderTarget.Get(), colour);

	//clear the depth (or z) buffer
	devcon->ClearDepthStencilView(zbuffer.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	//set the render target
	devcon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), zbuffer.Get());

	// set the vertex and index buffer
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	devcon->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	//set to true if you want to see wireframe
	bool wireFrame = false;

	if (wireFrame){
		devcon->RSSetState(wireFrameRastertizerState.Get());
	}
	else {
		devcon->RSSetState(defaultRasterizerState.Get());
	}

	//set the blend state
	devcon->OMSetBlendState(blendState.Get(), 0, 0xffffffff);

	//set the depth state - set to true if you want to use the depth buffer
	bool useDepth = true;

	if (useDepth){
		devcon->OMSetDepthStencilState(depthOnState.Get(), 0);
	}
	else {
		devcon->OMSetDepthStencilState(depthOffState.Get(), 0);
	}
	
	//world transformations
	XMMATRIX matRotate = XMMatrixRotationY(time / 4);
	XMMATRIX matTranslate = XMMatrixTranslation(1.5f, 1.0f, 1.5f);
	XMMATRIX matResize = XMMatrixScaling(0.5, 0.5, 0.5);

	//calculate view transformation
	XMVECTOR vecCamPosition = XMVectorSet(0, 3, 8, 0); 
	XMVECTOR vecCamLook = XMVectorSet(0, 0, 0, 0);	
	XMVECTOR vecCamUp = XMVectorSet(0, 1, 0, 0);
	XMMATRIX matView = XMMatrixLookAtLH(vecCamPosition, vecCamLook, vecCamUp); //view matrix
	
	//calculate the projection transformation
	//CoreWindow ^window = CoreWindow::GetForCurrentThread(); //get pointer to window to calculate the aspect ratio
	XMMATRIX matProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45), SCREEN_WIDTH/SCREEN_HEIGHT, 1, 100);

	//update the constant buffer
	CBUFFER cbuffer;
	cbuffer.Final = matRotate * matView * matProjection;
	cbuffer.Rotation = matRotate;
	cbuffer.DiffuseVector = XMVectorSet(-1.0f, -1.0f, -1.0f, 0.0f);
	cbuffer.DiffuseColor = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
	cbuffer.AmbientColor = XMVectorSet(0.2f, 0.2f, 0.2f, 1.0f);

	devcon->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbuffer, 0, 0);

	//set what primitive to use
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//draw the first cube
	devcon->DrawIndexed(36, 0, 0);
	
	cbuffer.Final = matResize * matTranslate * matRotate * matView * matProjection;
	devcon->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbuffer, 0, 0);

	//draw the second cube
	devcon->DrawIndexed(36, 0, 0);

	//swap the buffers
	swapchain->Present(1, 0);
}

//creating the buffer object
void CGame::InitGraphics()
{	
	//list of the vertices we will be using
	VERTEX vertices[] =
	{
		{ -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },    // side 1
		{ 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },

		{ -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f },    // side 2
		{ -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
		{ 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f },

		{ -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f },    // side 3
		{ -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },

		{ -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f },    // side 4
		{ 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f },
		{ -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f },
		{ 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f },

		{ 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f },    // side 5
		{ 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },

		{ -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f },    // side 6
		{ -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f },
		{ -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f },
	};

	//bd is a struc describing all the properties of the buffer
	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(VERTEX)* ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //what kind of buffer to make

	D3D11_SUBRESOURCE_DATA srd = { vertices, 0, 0 };

	dev->CreateBuffer(&bd, &srd, &vertexBuffer);

	//create the index buffer
	short indices[] =
	{
		0, 1, 2,    // side 1
		2, 1, 3,
		4, 5, 6,    // side 2
		6, 5, 7,
		8, 9, 10,    // side 3
		10, 9, 11,
		12, 13, 14,    // side 4
		14, 13, 15,
		16, 17, 18,    // side 5
		18, 17, 19,
		20, 21, 22,    // side 6
		22, 21, 23,
	};

	D3D11_BUFFER_DESC indexBufferDesc = { 0 };
	indexBufferDesc.ByteWidth = sizeof(short)*ARRAYSIZE(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices, 0, 0 };
	dev->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);

	//create constant buffer
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(CBUFFER); //constant buffer size has to be a multiple of 16
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	dev->CreateBuffer(&cbd, nullptr, &constantBuffer);
	devcon->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	//create the z-buffer
	D3D11_TEXTURE2D_DESC zbd = { 0 };
	zbd.Width = SCREEN_WIDTH;
	zbd.Height = SCREEN_HEIGHT;
	zbd.ArraySize = 1;
	zbd.MipLevels = 1;
	zbd.SampleDesc.Count = 1;
	zbd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	zbd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	dev->CreateTexture2D(&zbd, nullptr, &zbuffertexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	dev->CreateDepthStencilView(zbuffertexture.Get(), &bufferDesc, &zbuffer);

}


// this function loads a file into an Array^
Array<byte>^ LoadShaderFile(std::string File)
{
	Array<byte>^ FileData = nullptr;

	// open the file
	std::ifstream VertexFile(File, std::ios::in | std::ios::binary | std::ios::ate);

	// if open was successful
	if (VertexFile.is_open())
	{
		// find the length of the file
		int Length = (int)VertexFile.tellg();

		// collect the file data
		FileData = ref new Array<byte>(Length);
		VertexFile.seekg(0, std::ios::beg);
		VertexFile.read(reinterpret_cast<char*>(FileData->Data), Length);
		VertexFile.close();
	}

	return FileData;
}

//create and set the vertex shader, pixel shader and input layout
void CGame::InitPipeline()
{
	Array<byte> ^VSFile = LoadShaderFile("VertexShader.cso");
	Array<byte> ^PSFile = LoadShaderFile("PixelShader.cso");

	//encapsulate both shader arrays into shader objects
	dev->CreateVertexShader(VSFile->Data, VSFile->Length, nullptr, &vertexshader);
	dev->CreatePixelShader(PSFile->Data, PSFile->Length, nullptr, &pixelshader);

	//set the shader objects as the active objects
	devcon->VSSetShader(vertexshader.Get(), nullptr, 0);
	devcon->PSSetShader(pixelshader.Get(), nullptr, 0);

	// initialize input layout
	//input layout description
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA , 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//create the input layout
	dev->CreateInputLayout(ied, ARRAYSIZE(ied), VSFile->Data, VSFile->Length, &inputlayout);
	devcon->IASetInputLayout(inputlayout.Get());
	
}

void CGame::InitStates()
{
	//initializing the rasterizer state
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = FALSE;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = FALSE;
	rd.AntialiasedLineEnable = TRUE;
	rd.MultisampleEnable = FALSE;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.SlopeScaledDepthBias = 0.0f;

	dev->CreateRasterizerState(&rd, &defaultRasterizerState);

	rd.FillMode = D3D11_FILL_WIREFRAME;

	dev->CreateRasterizerState(&rd, &wireFrameRastertizerState);

	//initializing the blend state
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable =	true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.AlphaToCoverageEnable = FALSE;

	dev->CreateBlendState(&blendDesc, &blendState);

	//initializing the depth state
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dev->CreateDepthStencilState(&depthStencilDesc, depthOnState.GetAddressOf());

	depthStencilDesc.DepthEnable = false;

	dev->CreateDepthStencilState(&depthStencilDesc, depthOffState.GetAddressOf());
}
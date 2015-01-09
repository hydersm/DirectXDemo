#include "pch.h"
#include "Game.h"
#include <fstream>

struct VERTEX
{
	float X, Y, Z;
};

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

	CoreWindow^ Window = CoreWindow::GetForCurrentThread();    // get the window pointer

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

	// set the viewport ****ask jeff
	D3D11_VIEWPORT viewport = { 0 };

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Window->Bounds.Width;
	viewport.Height = Window->Bounds.Height;

	devcon->RSSetViewports(1, &viewport);

	InitGraphics();
	InitPipeline();
}

//this function updates the state of the game
void CGame::Update()
{

}

//this function renders a single frame of 3d graphics
void CGame::Render()
{
	//set the render target
	devcon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);

	//clear backbuffer to red
	float colour[4] = { 1, 0, 0, 1 };
	devcon->ClearRenderTargetView(renderTarget.Get(), colour);

	// set the vertex buffer
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	//set what primitive to use and DRAW!!!
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//draw 3 vertices starting from vertex 0
	devcon->Draw(3, 0);

	//swap the buffers
	swapchain->Present(1, 0);
}

void CGame::InitGraphics()
{
	//triangle vertices
	VERTEX vertices[] = 
	{
		{0.0f, 0.5f, 0.0f},
		{0.45f, -0.5f, 0.0f},
		{-0.45f, -0.5f, 0.0f},
	};

	//bd is a struc describing all the properties of the buffer
	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(VERTEX)* ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //what kind of buffer to make

	D3D11_SUBRESOURCE_DATA srd = { vertices, 0, 0 };

	dev->CreateBuffer(&bd, &srd, &vertexBuffer);
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
	};

	//create the input layout
	dev->CreateInputLayout(ied, ARRAYSIZE(ied), VSFile->Data, VSFile->Length, &inputlayout);
	devcon->IASetInputLayout(inputlayout.Get());

}
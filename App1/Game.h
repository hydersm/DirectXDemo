#pragma once

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace DirectX;

class CGame
{
private:
	ComPtr<ID3D11Device1> dev;
	ComPtr<ID3D11DeviceContext1> devcon;
	ComPtr<IDXGISwapChain1> swapchain;
	ComPtr<ID3D11RenderTargetView> renderTarget;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11VertexShader> vertexshader;
	ComPtr<ID3D11PixelShader> pixelshader;
	ComPtr<ID3D11InputLayout> inputlayout;
	ComPtr<ID3D11Buffer> constantBuffer;
	ComPtr<ID3D11Texture2D> zbuffertexture;
	ComPtr<ID3D11DepthStencilView> zbuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11RasterizerState> defaultRasterizerState;
	ComPtr<ID3D11RasterizerState> wireFrameRastertizerState;
	ComPtr<ID3D11BlendState> blendState;
	ComPtr<ID3D11DepthStencilState> depthOnState;
	ComPtr<ID3D11DepthStencilState> depthOffState;


	struct VERTEX
	{
		float X, Y, Z;
		//float R, G, B;
		float NX, NY, NZ;
	};

	struct CBUFFER
	{
		XMMATRIX Final;
		XMMATRIX Rotation;
		XMVECTOR DiffuseVector;
		XMVECTOR DiffuseColor;
		XMVECTOR AmbientColor;
	};

	float time;
	const float SCREEN_WIDTH = 2048;
	const float SCREEN_HEIGHT = 1152;

	void InitStates();

public:
	void Initialize();
	void Update();
	void Render();
	void InitGraphics();
	void InitPipeline();
};

